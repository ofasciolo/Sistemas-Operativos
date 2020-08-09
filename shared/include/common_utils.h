#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/error.h>
#include <commons/process.h>
#include <commons/collections/list.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>
//#include "common_connections.h"
#include "exit_status.h"

t_log* obligatory_logger,* optional_logger;
t_config* config;
pthread_t server;

typedef struct{
	uint32_t posx; 
	uint32_t posy; 
}t_position;

typedef struct{
	uint32_t sizePokemon;
	char* pokemon;
	t_position position; 
	uint32_t quantity;
}new_pokemon;

typedef struct{
	uint32_t sizePokemon;
	char* pokemon;
	t_position position; 
}appeared_pokemon;

typedef struct{
	uint32_t sizePokemon;
	char* pokemon;
	t_position position; 
}catch_pokemon;

typedef struct{
	uint32_t success;
}caught_pokemon;

typedef struct{
	uint32_t sizePokemon;
	char* pokemon;
}get_pokemon;

typedef struct{
	uint32_t sizePokemon; 
	char* pokemon; 
	t_list* positions; //al serializar se tiene que agregar la cantidad de posiciones
}localized_pokemon;

typedef struct{
}new_connection;

typedef struct{
	uint32_t id_connection;
}reconnect;

typedef struct{
	uint32_t id_message;
}ack;

typedef struct{
	uint32_t id_connection; 
}connection;

typedef struct{
	uint32_t colaMensajes;
}subscribe;

typedef struct t_connection {
    uint32_t socket;
    uint32_t id_connection;
    bool is_connected;
} t_connection;

t_list* connections; // solo para broker, pero no me quedaba otra que ponerlo aca
pthread_mutex_t m_connections;

new_pokemon* stream_to_new_pokemon(void* stream, uint32_t* id_message, bool is_broker);
void* new_pokemon_to_stream(new_pokemon* newPokemonMessage, uint32_t* id_message);
appeared_pokemon* stream_to_appeared_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker);
void* appeared_pokemon_to_stream(appeared_pokemon* appearedPokemonMessage, uint32_t* id_message, uint32_t* id_correlational);
catch_pokemon* stream_to_catch_pokemon(void* stream, uint32_t* id_message, bool is_broker);
void* catch_pokemon_to_stream(catch_pokemon* catchPokemonMessage, uint32_t* id_message);
caught_pokemon* stream_to_caught_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker);
void* caught_pokemon_to_stream(caught_pokemon* caughtPokemonMessage, uint32_t* id_message, uint32_t* id_correlational);
get_pokemon* stream_to_get_pokemon(void* stream, uint32_t* id_message, bool is_broker);
void* get_pokemon_to_stream(get_pokemon* getPokemonMessage, uint32_t* id_message);
localized_pokemon* stream_to_localized_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker);
void* localized_pokemon_to_stream(localized_pokemon* localizedPokemonMessage, uint32_t* id_message, uint32_t* id_correlational);
subscribe* stream_to_subscribe(void* stream);
void* subscribe_to_stream(subscribe* subscribeMessage);
new_connection* stream_to_new_connection(void* stream);
void* new_connection_to_stream(new_connection* newConnectionMessage);
reconnect* stream_to_reconnect(void* stream);
void* reconnect_to_stream(reconnect* reconnectMessage);
connection* stream_to_connection(void* stream);
void* connection_to_stream(connection* connectionMessage);
ack* stream_to_ack(void* stream);
void* ack_to_stream(ack* acknowledgementMessage);
new_pokemon* init_new_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t quantity);
appeared_pokemon* init_appeared_pokemon(char* nombre, uint32_t posx, uint32_t posy);
catch_pokemon* init_catch_pokemon(char* nombre, uint32_t posx, uint32_t posy);
caught_pokemon* init_caught_pokemon(bool success);
get_pokemon* init_get_pokemon(char* nombre);
localized_pokemon* init_localized_pokemon(char* nombre, t_list* positions);
new_connection* init_new_connection();
reconnect* init_reconnect(uint32_t id_connection);
connection* init_connection(uint32_t id_connection);
ack* init_ack(uint32_t id_message);
subscribe* init_subscribe(uint32_t id_queue);
void free_ack(ack* acknowledgement);
void free_connection(connection* conn);
void free_reconnect(reconnect* reconn);
void free_new_connection(new_connection* conn);
void free_subscribe(subscribe* subscriber);
void free_localized_pokemon(localized_pokemon* pokemon);
void free_get_pokemon(get_pokemon* pokemon);
void free_caught_pokemon(caught_pokemon* pokemon);
void free_catch_pokemon(catch_pokemon* pokemon);
void free_appeared_pokemon(appeared_pokemon* pokemon);
void free_new_pokemon(new_pokemon* pokemon);
uint32_t size_of_localized_pokemon(localized_pokemon* localizedPokemonMessage);
uint32_t size_of_get_pokemon(get_pokemon* getPokemonMessage);
uint32_t size_of_caught_pokemon(caught_pokemon* caughtPokemonMessage);
uint32_t size_of_catch_pokemon(catch_pokemon* catchPokemonMessage);
uint32_t size_of_appeared_pokemon(appeared_pokemon* appearedPokemonMessage);
uint32_t size_of_new_pokemon(new_pokemon* newPokemonMessage);
void* list_find_with_args(t_list *self, bool(*condition)(void* elem, void* args), void* args);
bool has_socket_fd(void* data, void* socket);
int strlenNewLine(const char* str);
uint64_t timestamp(void);



#endif