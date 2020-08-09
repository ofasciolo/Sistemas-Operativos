#ifndef NEWPOKEMON_H_
#define NEWPOKEMON_H_

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "commons_filesystem.h"

pthread_mutex_t createBlock; 

void newPokemonTallGrass(threadPokemonMessage* threadPokemonMessage);
void createMetadataPokemon(char* directory, new_pokemon* newPokemon);
void configMetadataCreate(char* metadata);
void agregarDatosYOrdenarBloques(char* metadata, new_pokemon* newPokemon);


#endif