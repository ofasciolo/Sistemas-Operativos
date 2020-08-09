#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "common_utils.h"
#include "memory.h"

void process_request(uint32_t cod_op, uint32_t size, uint32_t cliente_fd);
void handle_new_connection(uint32_t socket_fd);
void handle_reconnect(uint32_t client_fd,reconnect* reconnectMessage);




#endif