#ifndef DEADLOCK_H
#define DEADLOCK_H

#include "inicializacion.h"
#include "team.h"

char* pokemonCompareDeadlock; //variable used ONLY to detect pokemon needed in deadlock
pthread_mutex_t mutexPokemonCompareDeadlock;
bool flagExistsDeadlock;
pthread_mutex_t flagExistsDeadlock_mutex;

void detectDeadlock_do();
t_list* getPokemonsNeeded(t_trainer* trainerAux);
bool comparePokemonDeadlock(void* pokemonOwn);
bool isCandidateDeadlock(t_trainer* blockedTrainer);
bool completeCycleDeadlock();
bool trainerHasPokemonNoNeeded(t_trainer* trainerAux, char* pokemonNeeded);
bool existsDeadlock();
void setInterchangePokemon();
int trainerAlreadyInCycleCount(uint32_t idTrainer);
void* destroy_cycleNode(void* pointer);
void log_cycle();

#endif