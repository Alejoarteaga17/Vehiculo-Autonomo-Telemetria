#ifndef PROTOCOL_H
#define PROTOCOL_H

// Parámetros del servidor
#define MAX_CLIENTS 128
#define BUF_SIZE 2048
#define TELEMETRY_INTERVAL 10

// Autenticación
#define ADMIN_TOKEN "SECRETO_2025" 

// Tipos de mensajes
#define MSG_HELLO "HELLO"
#define MSG_WELCOME "WELCOME"
#define MSG_CMD "CMD"
#define MSG_CMD_RESP "CMD_RESP"
#define MSG_TELEMETRY "TELEMETRY"
#define MSG_ERROR "ERROR"
#define MSG_GET_DATA "GET_DATA"
#define MSG_LIST_USERS "LIST_USERS"

// Comandos disponibles
#define CMD_SPEED_UP "SPEED_UP"
#define CMD_SLOW_DOWN "SLOW_DOWN"
#define CMD_TURN_LEFT "TURN_LEFT"
#define CMD_TURN_RIGHT "TURN_RIGHT"

// Roles
#define ROLE_ADMIN "ADMIN"
#define ROLE_OBSERVER "OBSERVER"

// Límites del vehículo
#define MAX_SPEED 300  // km/h
#define MIN_BATTERY 15 // % mínimo para ejecutar comandos

#endif // PROTOCOL_H