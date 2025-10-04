#include <arpa/inet.h>
#include <errno.h>
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
#include <time.h>
#include <unistd.h>
#include "protocol.h"

// ====== Estado del vehículo ======
typedef struct {
  int speed;     // km/h
  int battery;   // %
  int temp;      // °C
  char dir[8];   // "LEFT","RIGHT","STRAIGHT"
} vehicle_t;

static vehicle_t g_vehicle = {0, 100, 25, "STRAIGHT"};
static pthread_mutex_t g_vehicle_mtx = PTHREAD_MUTEX_INITIALIZER;

// ====== Registro de clientes ======
typedef struct {
    int fd;
    char who[64];      // "ip:port"
    bool alive;
    bool is_admin;
    time_t last;
    } client_t;

    static client_t g_clients[MAX_CLIENTS];
    static pthread_mutex_t g_clients_mtx = PTHREAD_MUTEX_INITIALIZER;

    // ====== Logging JSON ======
    static FILE *g_logf = NULL;
    static void jlog(const char *type, const char *who, const char *fmt, ...) {
    char msg[1024];
    va_list ap; va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    time_t now=time(NULL);
    fprintf(g_logf?g_logf:stderr,
            "{\"ts\":%ld,\"type\":\"%s\",\"who\":\"%s\",\"msg\":\"%s\"}\n",
            now, type, who?who:"-", msg);
    fflush(g_logf?g_logf:stderr);
    }

    // ====== Util ======
    static void add_client(int fd, const char *who) {
    pthread_mutex_lock(&g_clients_mtx);
    for (int i=0;i<MAX_CLIENTS;i++){
    if (!g_clients[i].alive){
        g_clients[i]=(client_t){.fd=fd,.alive=true,.is_admin=false};
        snprintf(g_clients[i].who, sizeof g_clients[i].who, "%s", who);
        g_clients[i].last=time(NULL);
        break;
    }
    }
    pthread_mutex_unlock(&g_clients_mtx);
    }

    static void remove_client_fd(int fd){
    pthread_mutex_lock(&g_clients_mtx);
    for(int i=0;i<MAX_CLIENTS;i++){
    if(g_clients[i].alive && g_clients[i].fd==fd){
        g_clients[i].alive=false;
        close(fd);
        break;
    }
    }
    pthread_mutex_unlock(&g_clients_mtx);
    }

    static void broadcast(const char *line){
    pthread_mutex_lock(&g_clients_mtx);
    for (int i=0;i<MAX_CLIENTS;i++){
    if (g_clients[i].alive) {
        ssize_t _ = send(g_clients[i].fd, line, strlen(line), 0);
        (void)_;
    }
    }
    pthread_mutex_unlock(&g_clients_mtx);
    }

    // ====== Telemetría periódica ======
    static void *telemetry_thr(void *arg){
    (void)arg;
    char line[256];
    for(;;){
    sleep(TELEMETRY_INTERVAL);
    pthread_mutex_lock(&g_vehicle_mtx);
        // Simulación simple de dinámica:
        // - La batería baja 5% cada intervalo hasta mínimo 0.
        if (g_vehicle.battery > 0) g_vehicle.battery -= 5;
        // - Temperatura fluctúa ligeramente.
        if (g_vehicle.temp < 40 && (rand()%3==0)) g_vehicle.temp += 1;
        else if (g_vehicle.temp > 20 && (rand()%5==0)) g_vehicle.temp -= 1;
        // - Si dirección es LEFT/RIGHT vuelve a STRAIGHT después de 2 intervalos aprox.
        static int dir_count=0; dir_count++;
        if ((g_vehicle.dir[0]=='L' || g_vehicle.dir[0]=='R') && dir_count>=2){
            strncpy(g_vehicle.dir, "STRAIGHT", sizeof g_vehicle.dir); dir_count=0;
        }
        // - Pequeña inercia: si velocidad >0 y no se ha pedido cambio, reduce 1 cada 3 intervalos.
        static int speed_tick=0; speed_tick++;
        if (g_vehicle.speed>0 && speed_tick>=3){ g_vehicle.speed -= 1; speed_tick=0; }
    snprintf(line,sizeof line,
        "{\"t\":\"TELEMETRY\",\"speed\":%d,\"battery\":%d,\"temp\":%d,\"dir\":\"%s\",\"ts\":%ld}\n",
        g_vehicle.speed, g_vehicle.battery, g_vehicle.temp, g_vehicle.dir, time(NULL));
    pthread_mutex_unlock(&g_vehicle_mtx);
    broadcast(line);
    jlog("TX_DATA","*", "%s", line);
    }
    return NULL;
    }

    // ====== Thread-pool sencillo ======
    #define NWORKERS 6
    typedef struct job { int fd; struct job *next; } job_t;
    static job_t *q_head=NULL, *q_tail=NULL;
    static pthread_mutex_t q_mtx=PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t  q_cv =PTHREAD_COND_INITIALIZER;

    static void enqueue(int fd){
    job_t *j=malloc(sizeof *j); j->fd=fd; j->next=NULL;
    pthread_mutex_lock(&q_mtx);
    if(q_tail) q_tail->next=j; else q_head=j;
    q_tail=j;
    pthread_cond_signal(&q_cv);
    pthread_mutex_unlock(&q_mtx);
    }
    static int dequeue(void){
    pthread_mutex_lock(&q_mtx);
    while(!q_head) pthread_cond_wait(&q_cv,&q_mtx);
    job_t *j=q_head; q_head=j->next; if(!q_head) q_tail=NULL;
    pthread_mutex_unlock(&q_mtx);
    int fd=j->fd; free(j); return fd;
    }

    // ====== Parser/handler de mensajes ======
    static void sendln(int fd, const char *json){
    send(fd, json, strlen(json), 0);
    }

    static void handle_line(int fd, client_t *cli, char *line){
    // recorta \r\n
    size_t n=strlen(line); while(n && (line[n-1]=='\n'||line[n-1]=='\r')) line[--n]=0;
    jlog("RX", cli->who, "%s", line);
    cli->last=time(NULL);

    // HELLO
    if (strstr(line, "\"t\":\"HELLO\"")){
    bool admin = strstr(line, "\"role\":\"ADMIN\"");
    if (admin){
        const char *tok = strstr(line,"\"token\":\"");
        if (!tok || strncmp(tok+9, ADMIN_TOKEN, strlen(ADMIN_TOKEN))!=0){
        sendln(fd, "{\"t\":\"ERR\",\"code\":\"bad-auth\"}\n");
        return;
        }
        cli->is_admin=true;
    }
    char resp[128];
    snprintf(resp,sizeof resp,"{\"t\":\"HELLO_OK\",\"role\":\"%s\"}\n", cli->is_admin?"ADMIN":"OBSERVER");
    sendln(fd, resp);
    return;
    }

    // GET_DATA
    if (strstr(line, "\"t\":\"GET_DATA\"")){
    char resp[256];
    pthread_mutex_lock(&g_vehicle_mtx);
    snprintf(resp,sizeof resp,
        "{\"t\":\"DATA\",\"speed\":%d,\"battery\":%d,\"temp\":%d,\"dir\":\"%s\",\"ts\":%ld}\n",
        g_vehicle.speed,g_vehicle.battery,g_vehicle.temp,g_vehicle.dir,time(NULL));
    pthread_mutex_unlock(&g_vehicle_mtx);
    sendln(fd,resp);
    return;
    }

    // LIST_USERS (solo admin)
    if (strstr(line, "\"t\":\"LIST_USERS\"")){
    if (!cli->is_admin){ sendln(fd,"{\"t\":\"ERR\",\"code\":\"not-authorized\"}\n"); return; }
    char resp[1024]; strcpy(resp,"{\"t\":\"USERS\",\"items\":[");
    bool first=true;
    pthread_mutex_lock(&g_clients_mtx);
    for(int i=0;i<MAX_CLIENTS;i++) if(g_clients[i].alive){
        char item[128];
        snprintf(item,sizeof item,"%s{\"ip\":\"%.*s\",\"port\":0,\"role\":\"%s\"}",
                first?"":",", (int)strcspn(g_clients[i].who,":"), g_clients[i].who,
                g_clients[i].is_admin?"ADMIN":"OBSERVER");
        strcat(resp,item); first=false;
    }
    pthread_mutex_unlock(&g_clients_mtx);
    strcat(resp,"]}\n");
    sendln(fd,resp);
    return;
    }

    // CMD (solo admin)
    if (strstr(line,"\"t\":\"CMD\"")){
    if (!cli->is_admin){ sendln(fd,"{\"t\":\"ERR\",\"code\":\"not-authorized\"}\n"); return; }
    // extrae name
    const char *nm=strstr(line,"\"name\":\"");
    if(!nm){ sendln(fd,"{\"t\":\"ERR\",\"code\":\"bad-cmd\"}\n"); return; }
    char name[32]={0}; sscanf(nm+8,"%31[^\"]", name);

    // lógica simple con posibilidad de NACK
    char out[128];
    pthread_mutex_lock(&g_vehicle_mtx);
    if(!strcmp(name,"SPEED_UP")){
        if (g_vehicle.battery<15 || g_vehicle.speed>=100){
        snprintf(out,sizeof out,"{\"t\":\"NACK\",\"name\":\"%s\",\"reason\":\"%s\"}\n",name,
                    g_vehicle.battery<15?"battery-low":"speed-limit");
        } else { g_vehicle.speed+=10; snprintf(out,sizeof out,"{\"t\":\"ACK\",\"name\":\"%s\"}\n",name); }
    } else if(!strcmp(name,"SLOW_DOWN")) {
        g_vehicle.speed = (g_vehicle.speed>=10? g_vehicle.speed-10:0);
        snprintf(out,sizeof out,"{\"t\":\"ACK\",\"name\":\"%s\"}\n",name);
    } else if(!strcmp(name,"TURN_LEFT") || !strcmp(name,"TURN_RIGHT")){
        strncpy(g_vehicle.dir, !strcmp(name,"TURN_LEFT")?"LEFT":"RIGHT", sizeof g_vehicle.dir);
        snprintf(out,sizeof out,"{\"t\":\"ACK\",\"name\":\"%s\"}\n",name);
    } else {
        snprintf(out,sizeof out,"{\"t\":\"ERR\",\"code\":\"bad-cmd\"}\n");
    }
    pthread_mutex_unlock(&g_vehicle_mtx);
    sendln(fd,out);
    return;
    }

    if (strstr(line,"\"t\":\"PING\"")){ sendln(fd,"{\"t\":\"PONG\"}\n"); return; }
    if (strstr(line,"\"t\":\"BYE\"")) { remove_client_fd(fd); return; }

    sendln(fd,"{\"t\":\"ERR\",\"code\":\"bad-request\"}\n");
    }

    static void *worker(void *arg){
    (void)arg;
    char buf[BUF_SIZE];
    for(;;){
    int fd = dequeue();
    // quién es:
    client_t me = {0};
    pthread_mutex_lock(&g_clients_mtx);
    for (int i=0;i<MAX_CLIENTS;i++) if(g_clients[i].alive && g_clients[i].fd==fd) { me=g_clients[i]; break; }
    pthread_mutex_unlock(&g_clients_mtx);

    // loop de lectura por líneas
    FILE *fp = fdopen(dup(fd),"r");
    if(!fp){ remove_client_fd(fd); continue; }
    while (fgets(buf,sizeof buf, fp)){
        handle_line(fd, &me, buf);
    }
    fclose(fp);
    remove_client_fd(fd);
    jlog("DISCONNECT", me.who, "closed");
    }
    return NULL;
    }

    // ====== main ======
    int main(int argc, char **argv){
    if(argc!=3){ fprintf(stderr,"Uso: %s <port> <LogsFile>\n", argv[0]); return 1; }
    int port=atoi(argv[1]); g_logf=fopen(argv[2],"a"); if(!g_logf) g_logf=stderr;

    signal(SIGPIPE, SIG_IGN);

    // hilo de telemetría
    pthread_t ttele; pthread_create(&ttele,NULL,telemetry_thr,NULL); pthread_detach(ttele);

    // inicia pool
    pthread_t th[NWORKERS]; for (int i=0;i<NWORKERS;i++){ pthread_create(&th[i],NULL,worker,NULL); pthread_detach(th[i]); }

    int s=socket(AF_INET,SOCK_STREAM,0);
    int yes=1; setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
    struct sockaddr_in a={0}; a.sin_family=AF_INET; a.sin_addr.s_addr=INADDR_ANY; a.sin_port=htons(port);
    if(bind(s,(struct sockaddr*)&a,sizeof a)==-1){ perror("bind"); return 1; }
    if(listen(s,64)==-1){ perror("listen"); return 1; }
    fprintf(stderr,"VTAP/1.0 server on %d\n", port);

    for(;;){
    struct sockaddr_in ca; socklen_t cl=sizeof ca;
    int c=accept(s,(struct sockaddr*)&ca,&cl);
    if(c==-1){ if(errno==EINTR) continue; perror("accept"); continue; }
    char who[64]; snprintf(who,sizeof who,"%s:%d", inet_ntoa(ca.sin_addr), ntohs(ca.sin_port));
    add_client(c, who); jlog("CONNECT", who, "ok");
    enqueue(c);
    }
}