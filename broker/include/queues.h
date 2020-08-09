#ifndef QUEUES_H
#define QUEUES_H

#include "common_utils.h"
#include "common_connections.h"
#include "memory.h"
#include <semaphore.h>

void handle_new_connection(uint32_t client_fd);
void handle_reconnect(uint32_t client_fd, reconnect* reconn);
void handle_subscribe(uint32_t client_fd, subscribe* subs);
void queue_message_sender(void* queue);
void handle_ack(uint32_t client_fd, ack* acknowledgement);
void add_message_to_queue(void* message, uint32_t queue_id);




#endif