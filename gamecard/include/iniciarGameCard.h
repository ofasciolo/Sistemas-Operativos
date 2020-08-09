#ifndef INICIARGAMECARD_H_
#define INICIARGAMECARD_H_

#include "common_utils.h"
#include "newPokemon.h"
#include "catchPokemon.h"
#include "getPokemon.h"
#include <dirent.h>
#include <semaphore.h>
 //#include <sys/types.h>

typedef struct
{
    uint32_t idCola;
    uint32_t socket;
    uint32_t idConnection;
    pthread_mutex_t mutex;
} threadSubscribe;

uint32_t socket_broker;
on_request request;
char* PUERTO_BROKER;
char* IP_BROKER;
char* IP_GAMECARD;
char* PUERTO_GAMECARD;


pthread_t suscripcionNewPokemon;
pthread_t suscripcionCatchPokemon;
pthread_t suscripcionGetPokemon;
pthread_t threadForThreads;

threadSubscribe* structNewPokemon;
threadSubscribe* structCatchPokemon;
threadSubscribe* structGetPokemon;

t_list* threadSubscribeList;

sem_t semServeClient;

pthread_mutex_t createAndDetachMutex;

void iniciarGameCard();
void suscribirseATodo();
void finalizarGameCard();
void connect_client();
void crearSuscripcion(uint32_t socket,op_code codeOperation, pthread_t* threadName);
void subscribeAndConnect(args_pthread* arguments);
bool compareSockets(void* element, void* args);
void iniciarMutex();
#endif