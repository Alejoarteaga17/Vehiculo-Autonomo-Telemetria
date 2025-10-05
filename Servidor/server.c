#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <netinet/tcp.h>

#define BACKLOG 128
#define MAX_LINE 1024
#define MAX_CLIENTS 20
#define ADMIN_TOKEN "SECRETO_2025"
#define TELEMETRY_PERIOD_SEC 10
#define OVERHEAT_BLOCK_C 92

typedef enum { DIR_LEFT, DIR_RIGHT, DIR_STRAIGHT } direction_t;

typedef struct {
    int speed;       // km/h
    int battery;     // %
    int temp;        // °C
    direction_t dir;
} vehicle_state_t;

typedef enum { ROLE_OBSERVER=0, ROLE_ADMIN=1 } role_t;

typedef struct client_s {
    int fd;
    struct sockaddr_storage addr;
    socklen_t addrlen;
    char ip[INET_ADDRSTRLEN];
    int  port;
    role_t role;
    time_t connected_at;
    bool subscribed;
    bool alive;
    struct client_s* next;
} client_t;

// --- Globales ---
static int g_listen_fd = -1;
static FILE* g_logf = NULL;
static pthread_mutex_t g_log_mtx = PTHREAD_MUTEX_INITIALIZER;

static client_t* g_clients = NULL;
static pthread_mutex_t g_clients_mtx = PTHREAD_MUTEX_INITIALIZER;

static vehicle_state_t g_state = { .speed=10, .battery=100, .temp=15, .dir=DIR_STRAIGHT };
static pthread_mutex_t g_state_mtx = PTHREAD_MUTEX_INITIALIZER;

static int g_turn_ticks = 0;

static volatile sig_atomic_t g_running = 1;

// --- Utilidades ---
static void log_ts(FILE* f) {
    time_t now=time(NULL); struct tm tm; localtime_r(&now,&tm);
    char buf[32]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&tm);
    fprintf(f,"[%s] ",buf);
}

static void log_msg(const char* fmt, ...) {
    va_list ap;

    pthread_mutex_lock(&g_log_mtx);
    // consola
    log_ts(stdout);
    va_start(ap, fmt); vfprintf(stdout, fmt, ap); va_end(ap);
    fputc('\n', stdout); fflush(stdout);
    // archivo
    if (g_logf) {
        log_ts(g_logf);
        va_start(ap, fmt); vfprintf(g_logf, fmt, ap); va_end(ap);
        fputc('\n', g_logf); fflush(g_logf);
    }
    pthread_mutex_unlock(&g_log_mtx);
}


// Enviar todo manejando envíos parciales e interrupciones
static ssize_t send_all(int fd, const char* buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, buf + off, len - off, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)n;
    }
    return (ssize_t)off;
}

static ssize_t send_line(int fd, const char* line) {
    size_t n = strlen(line);
    ssize_t s1 = send(fd, line, n, 0);
    if (s1<0) return s1;
    ssize_t s2 = send(fd, "\n", 1, 0);
    if (s2<0) return s2;
    return s1 + s2;
}

static int recv_line(int fd, char* buf, size_t cap) {
    size_t i=0;
    while (i < cap-1) {
        char c;
        ssize_t r = recv(fd, &c, 1, 0);
        if (r==0) return 0; // FIN
        if (r<0) {
            if (errno==EINTR) continue;
            return -1; // error
        }
        if (c=='\r') continue;
        if (c=='\n') { buf[i]='\0'; return (int)i; }
        buf[i++] = c;
    }
    buf[i]='\0';
    return (int)i;
}

// --- Gestión de clientes ---
static void add_client(client_t* c) {
    pthread_mutex_lock(&g_clients_mtx);
    c->next = g_clients;
    g_clients = c;
    pthread_mutex_unlock(&g_clients_mtx);
}

static void remove_client(int fd) {
    pthread_mutex_lock(&g_clients_mtx);
    client_t **pp=&g_clients, *cur=g_clients;
    while (cur) {
        if (cur->fd == fd) {
            *pp = cur->next;
            close(cur->fd);
            free(cur);
            break;
        }
        pp = &cur->next;
        cur = cur->next;
    }
    pthread_mutex_unlock(&g_clients_mtx);
}

static int client_count(void) {
    int n=0;
    pthread_mutex_lock(&g_clients_mtx);
    for (client_t* c=g_clients; c; c=c->next) n++;
    pthread_mutex_unlock(&g_clients_mtx);
    return n;
}

// Enviar telemetría a todos 
static void broadcast_telemetry(void) {
    vehicle_state_t s;
    pthread_mutex_lock(&g_state_mtx); s = g_state; pthread_mutex_unlock(&g_state_mtx);

    char line[256];
    time_t now = time(NULL);
    const char* d = (s.dir==DIR_LEFT? "LEFT": s.dir==DIR_RIGHT? "RIGHT":"STRAIGHT");
    snprintf(line, sizeof(line),
        "TELEMETRY speed=%d battery=%d temp=%d dir=%s ts=%ld",
        s.speed, s.battery, s.temp, d, (long)now);

    int n=0;
    pthread_mutex_lock(&g_clients_mtx);
    for (client_t* c=g_clients; c; c=c->next) n++;
    int *fds = (int*)malloc(sizeof(int)*n);
    bool *wants = (bool*)malloc(sizeof(bool)*n);
    int i=0;
    for (client_t* c=g_clients; c && i<n; c=c->next, ++i) {
        fds[i] = c->fd;
        wants[i] = (c->alive && c->subscribed);
    }
    pthread_mutex_unlock(&g_clients_mtx);

    int *to_close = (int*)malloc(sizeof(int)*n);
    int nc=0;
    for (int k=0; k<n; ++k) {
        if (!wants[k]) continue;
        if (send_line(fds[k], line) < 0) {
            to_close[nc++] = fds[k];
        }
    }
    // Cerrar fallidos
    for (int k=0; k<nc; ++k) remove_client(to_close[k]);

    free(fds); free(wants); free(to_close);
}

// --- Lógica de comandos ---
static bool maybe_reject_for_policy(const char* cmd, char* reason, size_t rcap) {
    vehicle_state_t s;
    pthread_mutex_lock(&g_state_mtx); s=g_state; pthread_mutex_unlock(&g_state_mtx);

    if (!strcmp(cmd,"SPEED UP") && s.temp >= OVERHEAT_BLOCK_C) {
        snprintf(reason, rcap, "overheat"); 
        return true;
    }
    if (!strcmp(cmd,"SPEED UP") && s.battery < 10) {
        snprintf(reason, rcap, "low_battery");
        return true;
    }
    if (!strcmp(cmd,"SLOW DOWN") && s.speed==0) {
        snprintf(reason, rcap, "already_stopped");
        return true;
    }
    return false;
}

static void apply_command(const char* cmd) {
    pthread_mutex_lock(&g_state_mtx);
    if (!strcmp(cmd,"SPEED UP")) {
        g_state.speed += 5;
        if (g_state.speed > 120) g_state.speed = 120;
        if (g_state.battery>0) g_state.battery -= 1;
        g_state.temp += 3;               
        if (g_state.temp > 95) g_state.temp = 95;

    } else if (!strcmp(cmd,"SLOW DOWN")) {
        g_state.speed -= 5;
        if (g_state.speed < 0) g_state.speed = 0;
        if (g_state.temp > 20) g_state.temp -= 1;
    } else if (!strcmp(cmd,"TURN LEFT")) {
        g_state.dir = DIR_LEFT;
        g_turn_ticks = 2;
    } else if (!strcmp(cmd,"TURN RIGHT")) {
        g_state.dir = DIR_RIGHT;
        g_turn_ticks = 2;
    }
    pthread_mutex_unlock(&g_state_mtx);
}

// --- Hilos ---
static void* telemetry_thread_fn(void* _arg) {
    (void)_arg;
    while (g_running) {
        for (int i=0;i<TELEMETRY_PERIOD_SEC && g_running;i++) sleep(1);
        if (!g_running) break;

        pthread_mutex_lock(&g_state_mtx);
        if (g_state.dir != DIR_STRAIGHT && g_turn_ticks > 0) {
            g_turn_ticks--;
            if (g_turn_ticks == 0) {
                g_state.dir = DIR_STRAIGHT;   // Volver s straight
            }
        }
        pthread_mutex_unlock(&g_state_mtx);

        broadcast_telemetry();
    }
    return NULL;
}

static void list_users_to_client(int fd) {
    pthread_mutex_lock(&g_clients_mtx);
    int n=0; for (client_t* c=g_clients;c;c=c->next) n++;
    char hdr[64]; snprintf(hdr,sizeof(hdr),"USERS %d",n);
    send_line(fd,hdr);
    char line[256];
    for (client_t* c=g_clients;c;c=c->next) {
        snprintf(line, sizeof(line), "USER %s:%d %s %ld",
                 c->ip, c->port,
                 (c->role == ROLE_ADMIN ? "ADMIN" : "OBSERVER"),
                 (long)c->connected_at);
        send_line(fd,line);
    }
    pthread_mutex_unlock(&g_clients_mtx);
}

static void* client_thread_fn(void* arg) {
    client_t* self = (client_t*)arg;
    log_msg("CLIENT CONNECT %s:%d", self->ip, self->port);

    send_line(self->fd, "OK Welcome to TLP/1.0");
    self->subscribed = true; 

    char buf[MAX_LINE];
    while (g_running) {
        int n = recv_line(self->fd, buf, sizeof(buf));
        if (n==0) { log_msg("CLIENT FIN %s:%d", self->ip, self->port); break; }
        if (n<0)  { log_msg("CLIENT ERROR RECV %s:%d: %s", self->ip, self->port, strerror(errno)); break; }
        // Normaliza espacios
        while (n>0 && (buf[n-1]==' ')) { buf[--n]='\0'; }
        log_msg("REQ %s:%d -> %s", self->ip, self->port, buf);

        if (!strncmp(buf,"HELLO",5)) {
            send_line(self->fd, "OK HELLO");
        } else if (!strncmp(buf,"SUBSCRIBE",9)) {
            self->subscribed = true;
            send_line(self->fd, "OK SUBSCRIBED");
        } else if (!strncmp(buf,"AUTH ADMIN ",11)) {
            const char* tok = buf+11;
            if (!strcmp(tok, ADMIN_TOKEN)) {
                self->role = ROLE_ADMIN;
                send_line(self->fd, "OK AUTH ADMIN");
            } else {
                send_line(self->fd, "ERROR AUTH bad_token");
            }
        } else if (!strncmp(buf,"LIST USERS",10)) {
            list_users_to_client(self->fd);
        } else if (!strncmp(buf,"COMMAND ",8)) {
            if (self->role != ROLE_ADMIN) {
                send_line(self->fd, "ERROR PERM admin_required");
                continue;
            }
            const char* cmd = buf+8;
            if (strcmp(cmd,"SPEED UP") && strcmp(cmd,"SLOW DOWN") && strcmp(cmd,"TURN LEFT") && strcmp(cmd,"TURN RIGHT")) {
                send_line(self->fd, "ERROR CMD unknown_command");
                continue;
            }
            char reason[64];
            if (maybe_reject_for_policy(cmd, reason, sizeof(reason))) {
                char line[128]; snprintf(line,sizeof(line),"ERROR REJECTED %s", reason);
                send_line(self->fd, line);
                continue;
            }
            apply_command(cmd);
            send_line(self->fd, "OK EXECUTED");
        } else {
            send_line(self->fd, "ERROR BAD_REQUEST syntax");
        }
    }

    // Cierre
    self->alive=false;
    remove_client(self->fd);
    log_msg("CLIENT CLOSED %s:%d", self->ip, self->port);
    return NULL;
}

// --- Señales ---
static void on_sigint(int sig) {
    (void)sig;
    g_running = 0;
    if (g_listen_fd>=0) close(g_listen_fd);
}

// --- Main ---
int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <port> <LogsFile>\n", argv[0]);
        return 1;
    }
    const char* port = argv[1];
    const char* logpath = argv[2];

    g_logf = fopen(logpath, "a");
    if (!g_logf) { perror("fopen log"); return 1; }

    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    // --- Socket de escucha: SOCK_STREAM (TCP) ---
    struct addrinfo hints, *res, *rp;
    memset(&hints,0,sizeof(hints));
    hints.ai_family   = AF_INET;    
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags    = AI_PASSIVE;

    int rc = getaddrinfo(NULL, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(rc));
        return 1;
    }

    int listen_fd = -1;
    for (rp=res; rp; rp=rp->ai_next) {
        listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listen_fd<0) continue;

        int yes=1; setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen)==0) {
            if (listen(listen_fd, BACKLOG)==0) break;
        }
        close(listen_fd); listen_fd=-1;
    }
    freeaddrinfo(res);

    if (listen_fd<0) { perror("listen/bind"); return 1; }
    g_listen_fd = listen_fd;

    log_msg("SERVER START port=%s (socket type: SOCK_STREAM/TCP)", port);

    // Lanzar hilo de telemetría
    pthread_t th_tel; pthread_create(&th_tel, NULL, telemetry_thread_fn, NULL);

    // Loop de aceptación
    while (g_running) {
        struct sockaddr_storage cliaddr; socklen_t clilen=sizeof(cliaddr);
        int cfd = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
        if (cfd<0) {
            if (errno==EINTR && !g_running) break;
            if (errno==EINTR) continue;
            perror("accept"); continue;
        }

        // Límite de clientes concurrentes
        if (client_count() >= MAX_CLIENTS) {
            const char* busy = "ERROR server_busy max_clients_reached\n";
            send_all(cfd, busy, strlen(busy));
            shutdown(cfd, SHUT_RDWR);
            close(cfd);
            continue;
        }

        // TCP_NODELAY 
        int nd=1; setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &nd, sizeof(nd));

        client_t* c = calloc(1,sizeof(*c));
        c->fd = cfd; c->addr = cliaddr; c->addrlen=clilen; c->role=ROLE_OBSERVER; c->connected_at=time(NULL); c->subscribed=true; c->alive=true;

        struct sockaddr_in* sin = (struct sockaddr_in*)&cliaddr;  
        inet_ntop(AF_INET, &sin->sin_addr, c->ip, sizeof(c->ip));
        c->port = (int)ntohs(sin->sin_port);

        add_client(c);

        pthread_t th;
        pthread_create(&th, NULL, client_thread_fn, c);
        pthread_detach(th);
    }

    // Cierre ordenado
    g_running = 0;
    close(listen_fd);

    // Cerrar todos los clientes
    pthread_mutex_lock(&g_clients_mtx);
    for (client_t* c=g_clients;c;c=c->next) { shutdown(c->fd, SHUT_RDWR); close(c->fd); }
    pthread_mutex_unlock(&g_clients_mtx);

    pthread_join(th_tel, NULL);

    if (g_logf) fclose(g_logf);
    log_msg("SERVER STOP");
    return 0;
}
