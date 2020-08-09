#ifndef SUSCRIPCION_H
#define SUSCRIPCION_H

#include<commons/collections/list.h>
#include<commons/string.h>
#include "common_utils.h"
#include "common_connections.h"
#include "inicializacion.h"

typedef struct
{
    uint32_t idQueue;
    uint32_t socket;
    uint32_t idConnection;

} threadSubscribe;

typedef struct{
    uint32_t idMessage;
    char* pokemon;
} t_pokemonToLocalized;

//hilos colas de suscripcion
pthread_t suscripcionAppearedPokemon;
pthread_t suscripcionCaughtPokemon;
pthread_t suscripcionLocalizedPokemon;

threadSubscribe* structAppearedPokemon;
threadSubscribe* structCaughtPokemon;
threadSubscribe* structLocalizedPokemon;

t_list* threadSubscribeList;
pthread_mutex_t threadSubscribeList_mutex;

on_request request;

//hilo puerto escucha gameboy
t_list* pokemonsToLocalize;

void* connection_broker_global_suscribe();
void connection_broker_suscribe_to_appeared_pokemon(op_code);
void connection_broker_suscribe_to_caught_pokemon(op_code);
void connection_broker_suscribe_to_localized_pokemon(op_code);
args_pthread* thread_suscribe_arguments(op_code, uint32_t);
void suscribeOnThreadList(args_pthread*);
void listen_to_gameboy();
void reception_message_queue_subscription(uint32_t, uint32_t, uint32_t);
bool compareSockets(void*, void*);
uint32_t caught_default();
uint32_t localized_default(char*);
void* send_get_pokemon_global_team();
void connect_client(uint32_t socket,op_code codeOperation);
void addPokemonToLocalize(char* pokemon, uint32_t idMessage);
int getIndexPokemonToLocalizedByMessage(uint32_t id_message);
void removePokemonOnMap(t_position position);
void processCaughtPokemon(uint32_t id_message, uint32_t success);

#endif