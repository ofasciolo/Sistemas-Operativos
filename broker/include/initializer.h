#ifndef INITIALIZER_H
#define INITIALIZER_H


#include <commons/collections/queue.h>
#include "common_utils.h"
#include <queues.h>
#include <semaphore.h>
#include "memory.h"
#include "handler.h"

void dumpMemory();

typedef struct config_values{
    uint32_t tamano_memoria;
    uint32_t tamano_minimo_particion;
    char* algoritmo_memoria;
    char* algoritmo_reemplazo;
    char* algoritmo_particion_libre;
    char* ip_broker;
    char* puerto_broker;
    int frecuencia_compactacion;
    char* dump_file;
} config_values;


typedef struct t_receiver {
    t_connection* conn;
    bool sent;
    bool received;
} t_receiver;

typedef struct t_message_queue {
    uint32_t id_queue;
    t_queue* messages;
    t_list* subscribers;
    sem_t* sem_message;
    pthread_mutex_t* m_queue_modify;
    pthread_mutex_t* m_subscribers_modify;
} t_message_queue;


config_values cfg_values;
uint32_t listening_socket, id_connection, id_message;
on_request p_on_request;
pthread_mutex_t m_id_message, m_id_connection, m_new_partition;
t_list* list_queues;

void initialize();
void fill_config_values();
t_log* create_log_from_config(char* path_key, char* name, char* show_key);
void set_sig_handler(void);
void init_queues();

#endif