#include "deadlock.h"

void detectDeadlock_do(){
    //If exists a cycle of blocked trainers without disponible space for more pokemon, exists deadlock
    t_threadTrainer* threadTrainerAux;
    t_cycleDeadlock* deadlockNode;
    char* pokemonNeededAux;
    pthread_mutex_lock(&flagExistsDeadlock_mutex);
    bool continueAnalize = !flagExistsDeadlock;
    pthread_mutex_unlock(&flagExistsDeadlock_mutex);

    if(!continueAnalize) return;

    pthread_mutex_lock(&threadsTrainers_mutex);
    log_info(obligatory_logger, "Inicia algoritmo de detección de deadlock.");
    for(int i = 0; i < list_size(threadsTrainers) && continueAnalize; i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        
        if(threadTrainerAux->state == BLOCKED && isCandidateDeadlock(threadTrainerAux->trainer)){
            t_list* pokemonsNeeded = getPokemonsNeeded(threadTrainerAux->trainer);
            for(int j = 0; j < list_size(pokemonsNeeded) && continueAnalize; j++){
                pokemonNeededAux = (char*)list_get(pokemonsNeeded, j);
                cycleDeadLock = list_create();
                deadlockNode = malloc(sizeof(t_cycleDeadlock));
                deadlockNode->idTrainer = threadTrainerAux->trainer->id_trainer;
                deadlockNode->pokemon = malloc(strlen(pokemonNeededAux)+1);
                strcpy(deadlockNode->pokemon, pokemonNeededAux);
                list_add(cycleDeadLock, (void*)deadlockNode);
                if(!completeCycleDeadlock()){
                    list_destroy_and_destroy_elements(cycleDeadLock, (void*)destroy_cycleNode);
                }else{
                    if(existsDeadlock()){
                        pthread_mutex_lock(&flagExistsDeadlock_mutex);
                        flagExistsDeadlock = true;
                        pthread_mutex_unlock(&flagExistsDeadlock_mutex);
                        deadlockCount++;
                        setInterchangePokemon();
                        continueAnalize = false;
                    }else{
                        list_destroy_and_destroy_elements(cycleDeadLock, (void*)destroy_cycleNode);
                    }
                }
            }
            list_destroy(pokemonsNeeded);
        }
    }
    pthread_mutex_unlock(&threadsTrainers_mutex);

    if(continueAnalize){
        log_info(obligatory_logger, "No se detectó deadlock.");
    }else{
        log_info(obligatory_logger, "Se detectó deadlock.");
    }
}

t_list* getPokemonsNeeded(t_trainer* trainerAux){
    char* pokemonOwnedAux;
    t_list* pokemonsNeeded = list_duplicate(trainerAux->pokemonNeeded);

    for (int i=0; i<list_size(trainerAux->pokemonOwned); i++){
        pokemonOwnedAux = (char*)list_get(trainerAux->pokemonOwned, i);
        pthread_mutex_lock(&mutexPokemonCompareDeadlock);
        pokemonCompareDeadlock = malloc(strlen(pokemonOwnedAux)+1);
        strcpy(pokemonCompareDeadlock, pokemonOwnedAux);
        list_remove_by_condition(pokemonsNeeded, comparePokemonDeadlock);
        free(pokemonCompareDeadlock);
        pthread_mutex_unlock(&mutexPokemonCompareDeadlock);
    }

    return pokemonsNeeded;
}

void* destroy_cycleNode(void* pointer){
    t_cycleDeadlock* cycleDeadLockNode = (t_cycleDeadlock*)pointer;
    free(cycleDeadLockNode->pokemon);
    free(cycleDeadLockNode);

    return NULL;
}

bool comparePokemonDeadlock(void* pokemonOwn){
    return (strcmp((char*)pokemonOwn, pokemonCompareDeadlock) == 0);
}

bool isCandidateDeadlock(t_trainer* blockedTrainer){
    return list_size(blockedTrainer->pokemonOwned) == list_size(blockedTrainer->pokemonNeeded);
}

bool completeCycleDeadlock(){
    //the cycle already has the elements, so i look for the next to add
    t_cycleDeadlock* deadlockLastNode = (t_cycleDeadlock*)list_get(cycleDeadLock, list_size(cycleDeadLock) - 1);
    t_threadTrainer* threadTrainerAux;

    for(int i = 0; i<list_size(threadsTrainers); i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);

        if(threadTrainerAux->state == BLOCKED && isCandidateDeadlock(threadTrainerAux->trainer) && trainerHasPokemonNoNeeded(threadTrainerAux->trainer, deadlockLastNode->pokemon)){
            t_list* pokemonsNeeded = getPokemonsNeeded(threadTrainerAux->trainer);
            for(int j = 0; j < list_size(pokemonsNeeded); j++){
                if(trainerAlreadyInCycleCount(threadTrainerAux->trainer->id_trainer) > list_size(pokemonsNeeded)+1){
                    list_destroy(pokemonsNeeded);
                    return false;
                } 
                char* pokemonNeededAux = (char*)list_get(pokemonsNeeded, j);
                t_cycleDeadlock* deadlockNode = malloc(sizeof(t_cycleDeadlock));
                deadlockNode->idTrainer = threadTrainerAux->trainer->id_trainer;
                deadlockNode->pokemon = malloc(strlen(pokemonNeededAux)+1);
                strcpy(deadlockNode->pokemon, pokemonNeededAux);
                list_add(cycleDeadLock, (void*)deadlockNode);

                if(existsDeadlock()){
                    list_destroy(pokemonsNeeded);
                    log_cycle();
                    return true;
                }else{
                    if(!completeCycleDeadlock()){
                        list_remove_and_destroy_element(cycleDeadLock, list_size(cycleDeadLock)-1, (void*)destroy_cycleNode);
                    }else{
                        list_destroy(pokemonsNeeded);
                        return true;
                    }
                }
            }
            list_destroy(pokemonsNeeded);
        }
    }
    return false;
}

void log_cycle(){
    log_info(optional_logger, "Ciclo deadlock:");
    for(int i=0; i<list_size(cycleDeadLock); i++){
        t_cycleDeadlock* cycleNode = (t_cycleDeadlock*)list_get(cycleDeadLock, i);
        log_info(optional_logger, "Entrenador: %d, Pokemon: %s", cycleNode->idTrainer, cycleNode->pokemon);
    }
}

int trainerAlreadyInCycleCount(uint32_t idTrainer){
    int count=0;
    t_cycleDeadlock* cycleDeadLockAux;

    for(int i=0; i<list_size(cycleDeadLock); i++){
        cycleDeadLockAux = (t_cycleDeadlock*)list_get(cycleDeadLock, i);
        if(cycleDeadLockAux->idTrainer == idTrainer) count++;
    }

    return count;
}

bool trainerHasPokemonNoNeeded(t_trainer* trainerAux, char* pokemonNeeded){
    pthread_mutex_lock(&mutexPokemonCompareDeadlock);
    pokemonCompareDeadlock = malloc(strlen(pokemonNeeded)+1);
    strcpy(pokemonCompareDeadlock, pokemonNeeded);
    bool result = list_count_satisfying(trainerAux->pokemonOwned, comparePokemonDeadlock) > list_count_satisfying(trainerAux->pokemonNeeded, comparePokemonDeadlock);
    free(pokemonCompareDeadlock);
    pthread_mutex_unlock(&mutexPokemonCompareDeadlock);    
    return result;
}

bool existsDeadlock(){
    t_cycleDeadlock* firstNode = (t_cycleDeadlock*)list_get(cycleDeadLock, 0);
    t_cycleDeadlock* lastNode = (t_cycleDeadlock*)list_get(cycleDeadLock, list_size(cycleDeadLock)-1);

    return firstNode->idTrainer == lastNode->idTrainer && strcmp(firstNode->pokemon, lastNode->pokemon) == 0;
}

void setInterchangePokemon(){
    t_cycleDeadlock* cycleDeadlockFrom = (t_cycleDeadlock*)list_get(cycleDeadLock, 0);
    t_threadTrainer* threadTrainerToMove = (t_threadTrainer*)list_get(threadsTrainers, cycleDeadlockFrom->idTrainer - 1);
    t_cycleDeadlock* cycleDeadlockTo = (t_cycleDeadlock*)list_get(cycleDeadLock, 1);
    t_threadTrainer* threadTrainerWithDestiny = (t_threadTrainer*)list_get(threadsTrainers, cycleDeadlockTo->idTrainer - 1);

    log_info(obligatory_logger, "El entrenador %d intercambiará con el entrenador %d", threadTrainerToMove->trainer->id_trainer, threadTrainerWithDestiny->trainer->id_trainer);
log_info(optional_logger, "Interchange pokemon: From: Trainer: %d, pokemon: %s - To: Trainer: %d, pokemon: %s", cycleDeadlockFrom->idTrainer, cycleDeadlockFrom->pokemon, cycleDeadlockTo->idTrainer, cycleDeadlockTo->pokemon);

    threadTrainerToMove->positionTo.posx = threadTrainerWithDestiny->trainer->position.posx;
    threadTrainerToMove->positionTo.posy = threadTrainerWithDestiny->trainer->position.posy;
    threadTrainerToMove->destinyIsTrainer = true;

    calculateLeaveBlockedFromDeadlock(threadTrainerToMove);
}