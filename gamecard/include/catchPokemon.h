#ifndef CATCHPOKEMON_H_
#define CATCHPOKEMON_H_

#include "commons_filesystem.h"
#include "fileSystemTallGrass.h"

void catchPokemonTallGrass(threadPokemonMessage* threadCatchPokemonMessage);
void sacarDatosYOrdenarBloques(char* metadata, catch_pokemon* catchPokemon);
void removeLastBlock(char* metadata, catch_pokemon* catchPokemon, positionQuantity* posicionPokemonSacar);

#endif