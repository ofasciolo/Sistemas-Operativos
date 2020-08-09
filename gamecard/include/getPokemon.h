#ifndef GETPOKEMON_H_
#define GETPOKEMON_H_

#include "commons_filesystem.h"
#include "fileSystemTallGrass.h"

void getPokemonTallGrass(threadPokemonMessage* threadGetPokemonMessage);
t_list* getPositionsPokemon(char* metadata, char* pokemon);
void* structALineaSinCantidad(void* posicion);


#endif