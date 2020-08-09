#ifndef COMMONS_FILESYSTEM_H_
#define COMMONS_FILESYSTEM_H_

//#include "iniciarGameCard.h"
#include "common_utils.h"
#include "common_connections.h"
#include <math.h>
#include <commons/string.h>
#include <ctype.h>
#include <sys/file.h>
#include "fileSystemTallGrass.h"
typedef struct{
    uint32_t posicionX; 
    uint32_t posicionY;
    uint32_t cantidad;
}positionQuantity; 

typedef struct{

    char* nombreDirectorio; 
    pthread_mutex_t mutex;
}mutexDirectory;

typedef struct{
    void* pokemon; 
    uint32_t client_fd; 
    uint32_t* id_mensaje;
} threadPokemonMessage;

pthread_mutex_t mutexthreadSubscribeList;

t_list* mutexListDirectory; 
pthread_mutex_t mutexthreadSubscribeList;
pthread_mutex_t mutexListOfMutex;

pthread_mutex_t mutexBitmap;

pthread_mutex_t metadata_create; 

char* crearBloque(new_pokemon* newPokemon);
int metadataBlocks(char* metadata, char* pokemon);
t_list* levantarBloquesAMemoria(char** bloques, int cantidadBloques);
bool coincidePosicion(void* elem, void* args);
void* structALinea(void* posicion);
char* concatenarStrings(t_list* lista);
char* bajarBloquesADisco(t_list* lista, char** bloques, int cantidadBloques, char* nombrePokemon, uint32_t posx, uint32_t posy, uint32_t quantity, char* metadata);
bool esPokemon(void* elem, void* args);
void bloquearMetadata(char* pokemon);
void desbloquearMetadata(char* pokemon);
void abrirMetadata(char* metadata, char* pokemon);
void cerrarMetadata(char* metadata, char* pokemon);
void addBlockMetadata(char* metadata, char* block, new_pokemon* newPokemon);
void intentarAbrirMetadata(char* metadata, char* pokemon);

#endif