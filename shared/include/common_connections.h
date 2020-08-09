#ifndef COMMON_CONNECTIONS_H
#define COMMON_CONNECTIONS_H

#include "common_utils.h"

typedef void (*on_request)(uint32_t codop, uint32_t sizeofstruct, uint32_t socketfd);

typedef struct
{
	uint32_t* socket;
	on_request request_receiver;
} t_process_request;

typedef enum
{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
	NEW_CONNECTION = 8,
	CONNECTION = 9,
	RECONNECT = 10,
	ACK = 11,
	ERROR = 12,
	MENSAJE = 13
}op_code;


typedef struct
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	op_code codigoCola;
	uint32_t* socket;
} args_pthread;

void start_server(char* ip, char* port, on_request request_receiver);
void run_server(void* processor);
void receive_new_connections(uint32_t socket_escucha, on_request request_receiver);
void serve_client(void* processor);
void* serializar_paquete(t_paquete* paquete, uint32_t bytes);
void devolver_mensaje(void* payload, uint32_t socket_cliente);
uint32_t crear_conexion(char *ip, char* puerto);
void enviar_mensaje(char* mensaje, uint32_t socketfd);
char* recibir_mensaje(uint32_t socket_cliente, uint32_t buffer_size);
void process_message(uint32_t client_fd, void* stream);
void pthread_create_and_detach(void* function, void* args);
void mask_sig(void);
void* suscribirseA(op_code codigoOp,uint32_t socket_broker);
t_paquete* stream_to_package(op_code code,void* payload, uint32_t size_payload);
void free_package(t_paquete* package);
void send_ack(uint32_t socket_broker, uint32_t id_message);
void send_new_connection(uint32_t socket_broker);
uint32_t receive_connection_id(uint32_t socket_broker);
void send_reconnect(uint32_t socket_broker, uint32_t id_connection);
void send_appeared(appeared_pokemon* appearedPokemon, uint32_t socket, uint32_t* id_message);
void send_caught(caught_pokemon* caughtPokemon, uint32_t socket, uint32_t* id_message);
void send_localized(localized_pokemon* localizedPokemon, uint32_t socket, uint32_t* id_message);

#endif