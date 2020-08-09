#ifndef SERVER_GAMEBOY_H
#define SERVER_GAMEBOY_H

#include "common_utils.h"
#include "common_connections.h"

const static struct{
    op_code operation;
    char* name; 
}message_string [] = {
    {NEW_POKEMON, "NEW_POKEMON"},
    {APPEARED_POKEMON, "APPEARED_POKEMON"},
    {CATCH_POKEMON, "CATCH_POKEMON"},
    {CAUGHT_POKEMON, "CAUGHT_POKEMON"},
    {GET_POKEMON, "GET_POKEMON"},
    {LOCALIZED_POKEMON, "LOCALIZED_POKEMON"},
    {SUSCRIPTOR, "SUSCRIPTOR"},
    {ERROR, "ERROR"}
};

uint32_t id_connection;
uint32_t id_queue_to_subscribe;

void send_message(char** message, int socket_cliente,t_log*  optional_logger);
op_code stringToEnum(char* message);
void send_new_connection(uint32_t socket_broker);
void receiveMessageSubscriptor(uint32_t cod_op, uint32_t sizeofstruct, uint32_t socketfd);
uint32_t receive_connection_id(uint32_t socket_broker);

#endif