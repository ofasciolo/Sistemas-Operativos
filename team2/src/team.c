#include "team.h"

int main(int argc, char ** argv){

    initialize_team();

    return 0;
}

//--------States transition--------
//Called when appear a pokemon
void calculateTrainerFromNewToReady(){
    calculateTrainerToReady(NEW);
}

void calculateTrainerToReady(enum_process_state threadTrainerState){
    //From the list of pokemons on the map i seek wich trainer can look up for it
    t_pokemon_on_map* pokemonOnMapAux;
    t_threadTrainer* threadTrainerAux;
    bool continueFor = true;

	pthread_mutex_lock(&pokemonsOnMap_mutex);
    for(int i = 0; i < list_size(pokemonsOnMap) && continueFor; i++){
        pokemonOnMapAux = (t_pokemon_on_map*)list_get(pokemonsOnMap, i);
        if(pokemonOnMapAux->state == P_FREE && noOneChasingPokemon(pokemonOnMapAux->pokemon) && pokemonIsGlobalObjetive(pokemonOnMapAux->pokemon)){
            threadTrainerAux = getClosestTrainer(pokemonOnMapAux->position, threadTrainerState);
			pthread_mutex_lock(&threadsTrainers_mutex);
            if(threadTrainerAux != NULL && !isCandidateDeadlock(threadTrainerAux->trainer)){
                threadTrainerAux->state = READY;
				threadTrainerAux->positionTo.posx = pokemonOnMapAux->position.posx;
				threadTrainerAux->positionTo.posy = pokemonOnMapAux->position.posy;
				
				pokemonOnMapAux->state = P_CHASING;
				continueFor = false;
				threadTrainerAux->incomingTime = time(NULL);

				if(threadTrainerState == NEW){
					log_info(obligatory_logger, "Entrenador %d, cambia de NEW a READY, porque es el más cercano para realizar la captura", threadTrainerAux->trainer->id_trainer);
				}else{
					log_info(obligatory_logger, "Entrenador %d, cambia de BLOCKED a READY, porque es el más cercano para realizar la captura", threadTrainerAux->trainer->id_trainer);
				}
				sem_post(&plannerSemaphore);
            }
			pthread_mutex_unlock(&threadsTrainers_mutex);
        }
    }
	pthread_mutex_unlock(&pokemonsOnMap_mutex);
}

bool pokemonIsGlobalObjetive(char* pokemonAsked){
	//Remove the global objetive
	pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
	pokemonCompareGlobalObjetive = malloc(strlen(pokemonAsked)+1);
	strcpy(pokemonCompareGlobalObjetive, pokemonAsked);
	bool result = list_any_satisfy(globalObjetive, analyzePokemonInGlobal);
	free(pokemonCompareGlobalObjetive);
	pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
	return result;
}

bool noOneChasingPokemon(char* pokemonChased){
	t_pokemon_on_map* pokemonOnMapAux;
	for(int i = 0; i < list_size(pokemonsOnMap); i++){
        pokemonOnMapAux = (t_pokemon_on_map*)list_get(pokemonsOnMap, i);
        if(pokemonOnMapAux->state == P_CHASING && strcmp(pokemonOnMapAux->pokemon, pokemonChased) == 0){
            return false;
        }
    }
	return true;
}

t_threadTrainer* getClosestTrainer(t_position position, enum_process_state threadTrainerState){
    t_threadTrainer* threadTrainerAux;
    int32_t minimunDistance = -1;
	t_threadTrainer* threadTrainerSelected = NULL;

	pthread_mutex_lock(&threadsTrainers_mutex);
    for(int i=0; i < list_size(threadsTrainers); i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        if(threadTrainerAux->state == threadTrainerState && !isCandidateDeadlock(threadTrainerAux->trainer)){
            if(minimunDistance == -1 || minimunDistance > calculateDistance(threadTrainerAux->trainer->position, position)){
                minimunDistance = calculateDistance(threadTrainerAux->trainer->position, position);
				threadTrainerSelected = threadTrainerAux;
            }
        }
    }
	pthread_mutex_unlock(&threadsTrainers_mutex);

	return threadTrainerSelected;
}

uint32_t calculateDistance(t_position positionFrom, t_position positionTo){
    int32_t posX = positionTo.posx - positionFrom.posx;
    int32_t posY = positionTo.posy - positionFrom.posy;
    if(posX < 0) posX = posX * -1;
    if(posY < 0) posY = posY * -1;
    return posX + posY;
}

//Called always that a trainer its ready
void calculateTrainerFromReadyToExec(){
	pthread_mutex_lock(&threadsTrainers_mutex);
	bool result = list_any_satisfy(threadsTrainers, existsThreadTrainerInExec);
	pthread_mutex_unlock(&threadsTrainers_mutex);
    if(result) return;
	
    setTrainerToExec();
}

bool existsThreadTrainerInExec(void* threadTrainer){
    return ((t_threadTrainer*)threadTrainer)->state == EXEC;
}

void setTrainerToExec(){
    if (strcmp(config_values.algoritmo_planificacion, "FIFO") == 0) setTrainerToExec_FirstCome();
    else if (strcmp(config_values.algoritmo_planificacion, "RR") == 0) setTrainerToExec_FirstCome();
    else if (strcmp(config_values.algoritmo_planificacion, "SJF-SD") == 0) setTrainerToExec_SJF();
    else if (strcmp(config_values.algoritmo_planificacion, "SJF-CD") == 0) setTrainerToExec_SJF();
}

void setTrainerToExec_FirstCome(){
    time_t lowestTime = time(NULL);
    t_threadTrainer* threadTrainerAux;
	t_threadTrainer* threadTrainerSelected = NULL;
	
	pthread_mutex_lock(&threadsTrainers_mutex);
    for(int i=0; i < list_size(threadsTrainers); i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        if(threadTrainerAux->incomingTime <= lowestTime && threadTrainerAux->state == READY){
            lowestTime = threadTrainerAux->incomingTime;
            threadTrainerSelected = threadTrainerAux;
        }
    }

	if(threadTrainerSelected != NULL){
		threadTrainerSelected->state = EXEC;
		sem_post(&(threadTrainerSelected->semaphoreAction));
		log_info(obligatory_logger, "Entrenador %d, cambia de READY a EXEC, porque es el siguiente a ejecutar", threadTrainerSelected->trainer->id_trainer);
	}
	pthread_mutex_unlock(&threadsTrainers_mutex);
}

void setTrainerToExec_SJF(){
    //SJF: Estimador = Ti-1*alpha + Ri-1*(1-alpha)
    double estimator = -1;
	uint32_t cyclesNeeded;
	double previusEstimator;
    t_threadTrainer* threadTrainerAux;
	t_threadTrainer* threadTrainerSelected = NULL;

	pthread_mutex_lock(&threadsTrainers_mutex);
    for(int i=0; i < list_size(threadsTrainers); i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
		if(threadTrainerAux->state == READY){	
			previusEstimator = threadTrainerAux->valueEstimator;
			cyclesNeeded = calculateDistance(threadTrainerAux->trainer->position, threadTrainerAux->positionTo);

			threadTrainerAux->valueEstimator = previusEstimator * config_values.alpha + cyclesNeeded * (1 - config_values.alpha);
log_info(optional_logger, "Trainer %d: from: (%d,%d) - to: (%d,%d)", threadTrainerAux->trainer->id_trainer, threadTrainerAux->trainer->position.posx,threadTrainerAux->trainer->position.posy,threadTrainerAux->positionTo.posx,threadTrainerAux->positionTo.posy);
log_info(optional_logger, "Trainer %d: previus estimator: %f * alpha: %f + cycles: %d * (1 - alpha: %f)", threadTrainerAux->trainer->id_trainer, previusEstimator, config_values.alpha, cyclesNeeded, config_values.alpha);
log_info(optional_logger, "Trainer %d: estimator: %f", threadTrainerAux->trainer->id_trainer, threadTrainerAux->valueEstimator);
			if((estimator == -1 || estimator > threadTrainerAux->valueEstimator) && threadTrainerAux->state == READY){
				estimator = threadTrainerAux->valueEstimator;
				threadTrainerSelected = threadTrainerAux;
			}
		}
    }

	if(threadTrainerSelected != NULL){
log_info(optional_logger, "Selected Trainer %d: estimator: %f", threadTrainerSelected->trainer->id_trainer, threadTrainerSelected->valueEstimator);
		threadTrainerSelected->state = EXEC;
		sem_post(&(threadTrainerSelected->semaphoreAction));
		log_info(obligatory_logger, "Entrenador %d, cambia de READY a EXEC, porque es el siguiente a ejecutar", threadTrainerSelected->trainer->id_trainer);
	}
	pthread_mutex_unlock(&threadsTrainers_mutex);
}

//Called when a pokemon appear, on deadlock thread, and on message "caught pokemon"
void calculateLeaveBlockedFromAppear(){
    calculateTrainerToReady(BLOCKED);
}

void calculateLeaveBlockedFromDeadlock(t_threadTrainer* threadTrainerAux){
	threadTrainerAux->state = READY;
	threadTrainerAux->incomingTime = time(NULL);
	log_info(obligatory_logger, "Entrenador %d, cambia de BLOCKED a READY, porque intercambiará por deadlock", threadTrainerAux->trainer->id_trainer);
	sem_post(&plannerSemaphore);
}

void calculateLeaveBlockedFromCaught(uint32_t idTrainer){
    //If the trainer accomplish his goal goes to exit, else, stays in blocked
    t_threadTrainer* threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, idTrainer - 1);

	if(trainerCompleteOwnObjetives(threadTrainerAux->trainer)){
		threadTrainerAux->state = E_P_EXIT;
		log_info(obligatory_logger, "Entrenador %d, cambia de BLOCKED a EXIT, porque cumplió sus objetivos individuales", threadTrainerAux->trainer->id_trainer);
		sem_post(&threadTrainerAux->semaphoreAction);
	}
}

bool trainerCompleteOwnObjetives(t_trainer* trainerAux){
	bool result = false;
	if(list_size(trainerAux->pokemonNeeded) == list_size(trainerAux->pokemonOwned)){
		t_list* copyOfNeeded = list_duplicate(trainerAux->pokemonNeeded);
		t_list* copyOfOwned = list_duplicate(trainerAux->pokemonOwned);
		result = true;

		list_sort(copyOfNeeded, compareStrings);
		list_sort(copyOfOwned, compareStrings);

		for(int j=0; j<list_size(copyOfNeeded) && result; j++){
			if(strcmp((char*)list_get(copyOfNeeded, j), (char*)list_get(copyOfOwned, j)) != 0){
				result = false;
			}
		}
		list_destroy(copyOfNeeded);
		list_destroy(copyOfOwned);
	}

	return result;
}

bool compareStrings(void* string1, void* string2){
    return(strcmp((char*)string1, (char*)string2) < 0);
}

void calculateTrainerInExit(t_threadTrainer* threadTrainerAux){
	if(threadTrainerAux->state == E_P_EXIT){
		writeTrainerMetrics(threadTrainerAux);
		sem_post(&plannerSemaphore);
		pthread_cancel(threadTrainerAux->threadTrainer);
	}
}

void writeTrainerMetrics(t_threadTrainer* threadTrainerAux){
	log_info(obligatory_logger, "Trainer: %d", threadTrainerAux->trainer->id_trainer);
	log_info(obligatory_logger, "Total Cycle CPU by Trainer %d: %d", threadTrainerAux->trainer->id_trainer, threadTrainerAux->cpuCycleCount);

	t_threadTrainer* threadTrainerAux2;
	for(int i = 0; i<list_size(threadsTrainers); i++){
        threadTrainerAux2 = (t_threadTrainer*)list_get(threadsTrainers, i);
           
		writePokemonsOfTrainer(threadTrainerAux2->trainer);
    }
}

bool trainerStateIsExit(void* threadTrainer){
    return ((t_threadTrainer*)threadTrainer)->state == E_P_EXIT;
}

void writeTeamMetrics(){
    t_threadTrainer* threadTrainerAux;
    uint32_t totalCycleCpuTeam = 0;
    uint32_t totalContextSwitch = 0;

    for(int i = 0; i<list_size(threadsTrainers); i++){
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        totalCycleCpuTeam += threadTrainerAux->cpuCycleCount;
        totalContextSwitch += threadTrainerAux->contextSwitchCount;
        
		writePokemonsOfTrainer(threadTrainerAux->trainer);
    }
	log_info(obligatory_logger, "Algorithm: %s ", config_values.algoritmo_planificacion); 
	log_info(obligatory_logger, "Total Cycle CPU by TEAM: %d", totalCycleCpuTeam);
	log_info(obligatory_logger, "Total Context Switch by Team: %d", totalContextSwitch);
	log_info(obligatory_logger, "Total Deadlocks by Team: %d", deadlockCount);
}

void writePokemonsOfTrainer(t_trainer* trainerAux){
	log_info(optional_logger, "Trainer %d result: ", trainerAux->id_trainer);
	for(int i=0; i<list_size(trainerAux->pokemonOwned); i++){
		log_info(optional_logger, "Pokemon owned: %s", (char*)list_get(trainerAux->pokemonOwned, i));
	}
	for(int i=0; i<list_size(trainerAux->pokemonNeeded); i++){
		log_info(optional_logger, "Pokemon needed: %s", (char*)list_get(trainerAux->pokemonNeeded, i));
	}
}

void finishTeam(){
    release_resources();
}

void executeAlgorithm() {
	pthread_mutex_lock(&threadsTrainers_mutex);
	t_threadTrainer* threadTrainerAux = (t_threadTrainer*)list_find(threadsTrainers, threadTrainerInExec);
	pthread_mutex_unlock(&threadsTrainers_mutex);

	if(threadTrainerAux != NULL && threadTrainerAux->state == EXEC){
		if (strcmp(config_values.algoritmo_planificacion, "FIFO") == 0) execThreadTrainerSetedFCFS(threadTrainerAux);
		else if (strcmp(config_values.algoritmo_planificacion, "RR") == 0) execThreadTrainerSetedRR(threadTrainerAux);
		else if (strcmp(config_values.algoritmo_planificacion, "SJF-SD") == 0) execThreadTrainerSetedSJF_SD(threadTrainerAux);
		else if (strcmp(config_values.algoritmo_planificacion, "SJF-CD") == 0) execThreadTrainerSetedSJF_CD(threadTrainerAux);
	}
}

bool threadTrainerInExec(void* threadTrainerAux){
	return ((t_threadTrainer*)threadTrainerAux)->state == EXEC;
}

t_pokemon_on_map* getPokemonByPosition(t_position pokemonPosition){
	positionCompare.posx = pokemonPosition.posx;
	positionCompare.posy = pokemonPosition.posy;

	pthread_mutex_lock(&pokemonsOnMap_mutex);
	t_pokemon_on_map* result = (t_pokemon_on_map*)list_find(pokemonsOnMap, pokemonsOnMapComparePosition);
	pthread_mutex_unlock(&pokemonsOnMap_mutex);

	return result;
}
bool pokemonsOnMapComparePosition(void* pokemonOnMapAux){
	return ((t_pokemon_on_map*)pokemonOnMapAux)->position.posx == positionCompare.posx && ((t_pokemon_on_map*)pokemonOnMapAux)->position.posy == positionCompare.posy;
}
bool move_to_objetive(t_threadTrainer* threadTrainerAux){
	bool reachDestiny = false;
	pthread_mutex_lock(&threadsTrainers_mutex);
	t_position currentPosition = threadTrainerAux->trainer->position;
	t_trainer* trainerAux = threadTrainerAux->trainer;
	if(trainerAux->position.posx != threadTrainerAux->positionTo.posx){
		if(trainerAux->position.posx < threadTrainerAux->positionTo.posx){
			trainerAux->position.posx++;
		}else{
			trainerAux->position.posx--;
		}
	}else{
		if(trainerAux->position.posy != threadTrainerAux->positionTo.posy){
			if(trainerAux->position.posy < threadTrainerAux->positionTo.posy){
				trainerAux->position.posy++;
			}else{
				trainerAux->position.posy--;
			}
		}	
	}
	threadTrainerAux->cpuCycleCount++;
	pthread_mutex_unlock(&threadsTrainers_mutex);
	log_info(obligatory_logger, "Entrenador %d - Posicion actual: (%d,%d) - Posicion destino: (%d,%d)", trainerAux->id_trainer, currentPosition.posx, currentPosition.posy, trainerAux->position.posx, trainerAux->position.posy);
	sleep(config_values.retardo_ciclo_cpu);

	if(trainerAux->position.posx == threadTrainerAux->positionTo.posx && trainerAux->position.posy == threadTrainerAux->positionTo.posy){
		if(threadTrainerAux->destinyIsTrainer){
			//My goal was a trainer, then, interchange
			if(threadTrainerAux->interchangeCycleCount == 5){
				reachDestiny = true;
				pthread_mutex_lock(&threadsTrainers_mutex);
				threadTrainerAux->destinyIsTrainer = false;
				threadTrainerAux->interchangeCycleCount = 0;
				pthread_mutex_unlock(&threadsTrainers_mutex);
				interchangePokemon(threadTrainerAux);
			}
			pthread_mutex_lock(&threadsTrainers_mutex);
			threadTrainerAux->interchangeCycleCount++;
			pthread_mutex_unlock(&threadsTrainers_mutex);
		}else{
			reachDestiny = true;
			//My goal was a pokemon, then, message catch
			t_pokemon_on_map* pokemonDestiny = getPokemonByPosition(threadTrainerAux->trainer->position);
			if(pokemonDestiny != NULL){
				if(!sendCatch(pokemonDestiny, threadTrainerAux)){
					catch_succesfull(threadTrainerAux);
				}
			}
		}
	}
	return reachDestiny;
}

void interchangePokemon(t_threadTrainer* threadTrainerFrom){
	t_cycleDeadlock* pokemonInDeadlockFrom = (t_cycleDeadlock*)list_get(cycleDeadLock, 0);
	t_cycleDeadlock* pokemonInDeadlockTo = (t_cycleDeadlock*)list_get(cycleDeadLock, 1);
	t_trainer* trainerTo = ((t_threadTrainer*)list_get(threadsTrainers, pokemonInDeadlockTo->idTrainer-1))->trainer;
	t_trainer* trainerFrom = threadTrainerFrom->trainer;

	char* pokemonOwnedTrainerFrom = getPokemonNotNeeded(trainerFrom);
	char* pokemonOwnedTrainerTo = getPokemonSpecify(trainerTo, pokemonInDeadlockFrom->pokemon);

	list_add(trainerFrom->pokemonOwned, pokemonOwnedTrainerTo);
	list_add(trainerTo->pokemonOwned, pokemonOwnedTrainerFrom);

	list_destroy_and_destroy_elements(cycleDeadLock, (void*)destroy_cycleNode);
	
	pthread_mutex_lock(&flagExistsDeadlock_mutex);
	flagExistsDeadlock = false;
	pthread_mutex_unlock(&flagExistsDeadlock_mutex);
	pthread_mutex_lock(&threadsTrainers_mutex);
	if(trainerCompleteOwnObjetives(trainerFrom)){
		threadTrainerFrom->state = E_P_EXIT;
		log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a EXIT, porque cumplió sus objetivos individuales luego de intercambio", trainerFrom->id_trainer);
	}else{
		threadTrainerFrom->state = BLOCKED;
		//threadTrainerFrom->contextSwitchCount++;
		log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a BLOCKED, porque le falta cumplir su objetivo", trainerFrom->id_trainer);
	}
	pthread_mutex_unlock(&threadsTrainers_mutex);
	calculateLeaveBlockedFromCaught(trainerTo->id_trainer);
	sem_post(&plannerSemaphore);
	if(threadTrainerFrom->state == E_P_EXIT) calculateTrainerInExit(threadTrainerFrom);
}

char* getPokemonNotNeeded(t_trainer* trainerAux){
	//return the first pokemon that does not need (that own but not need)
	t_list* copyOfOwned;
	char* result;
	bool continueFor = true;
	copyOfOwned = list_duplicate(trainerAux->pokemonOwned);
	for(int j = 0; j < list_size(trainerAux->pokemonNeeded) && continueFor; j++){
		result = (char*)list_get(trainerAux->pokemonNeeded, j);
		pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
		pokemonCompareGlobalObjetive = malloc(strlen(result)+1);
		strcpy(pokemonCompareGlobalObjetive, result);
		list_remove_by_condition(copyOfOwned, analyzePokemonInGlobal);
		free(pokemonCompareGlobalObjetive);
		pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
	}
	result = (char*)list_get(copyOfOwned, 0);
	list_destroy(copyOfOwned);
	pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
	pokemonCompareGlobalObjetive = malloc(strlen(result)+1);
	strcpy(pokemonCompareGlobalObjetive, result);
	list_remove_by_condition(trainerAux->pokemonOwned, analyzePokemonInGlobal);
	free(pokemonCompareGlobalObjetive);
	pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
	return result;
}
char* getPokemonSpecify(t_trainer* trainerAux, char* pokemon){
	char* result;
	pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
	pokemonCompareGlobalObjetive = malloc(strlen(pokemon)+1);
	strcpy(pokemonCompareGlobalObjetive, pokemon);
	result = (char*)list_remove_by_condition(trainerAux->pokemonOwned, analyzePokemonInGlobal);
	free(pokemonCompareGlobalObjetive);
	pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);

	return result;
}

bool sendCatch(t_pokemon_on_map* pokemon, t_threadTrainer* threadTrainerAux){
	log_info(obligatory_logger, "Atrapar pokemon: %s, en posicion: (%d,%d)", pokemon->pokemon, pokemon->position.posx, pokemon->position.posy);

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(config_values.ip_broker, config_values.puerto_broker, &hints, &server_info);

	uint32_t client_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	bool result = true;
	if(connect(client_fd, server_info->ai_addr, server_info->ai_addrlen) != -1){
		pthread_mutex_lock(&threadsTrainers_mutex);
		threadTrainerAux->state = BLOCKED_BY_BROKER;
		//threadTrainerAux->contextSwitchCount++;
		log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a BLOCKED, porque espera resultado de catch", threadTrainerAux->trainer->id_trainer);
		pthread_mutex_unlock(&threadsTrainers_mutex);

		uint32_t* id_message = malloc(sizeof(uint32_t));

		catch_pokemon* catchPokemonMessage = init_catch_pokemon(pokemon->pokemon, pokemon->position.posx, pokemon->position.posy);
		*id_message = -1;
		t_paquete* packageToSend = stream_to_package(CATCH_POKEMON, catch_pokemon_to_stream(catchPokemonMessage, id_message), size_of_catch_pokemon(catchPokemonMessage));
		int bytes = packageToSend->buffer->size + 2*sizeof(uint32_t);
		void* buffer = (void *) serializar_paquete(packageToSend, bytes);

		send(client_fd, buffer, bytes, 0);
		free_package(packageToSend);
		free(buffer);
		uint32_t sizeOfBuffer = sizeof(uint32_t) * 3;
		buffer = malloc(sizeOfBuffer);
		recv(client_fd, buffer, sizeOfBuffer, MSG_WAITALL);
		ack* acknowledgementMessage = stream_to_ack(buffer+8);
		pthread_mutex_lock(&threadsTrainers_mutex);
		threadTrainerAux->idMessageCatch = acknowledgementMessage->id_message;
		pthread_mutex_unlock(&threadsTrainers_mutex);
log_info(optional_logger, "Send catch pokemon: Pokemon: %s - Id-message: %d", pokemon->pokemon, acknowledgementMessage->id_message);

		close(client_fd);
		free(id_message);
		free(buffer);
		free(catchPokemonMessage);
		free_ack(acknowledgementMessage);
		sem_post(&plannerSemaphore);
	}else{
		log_info(obligatory_logger, "Falló conexión con broker, se ejecutará función por default de appeared");
		result = false;
	}

	free(server_info);
	return result;
}
void catch_succesfull(t_threadTrainer* threadTrainerAux){
	t_trainer* trainerAux = threadTrainerAux->trainer;
	t_pokemon_on_map* pokemonOnMapAux = getPokemonByPosition(threadTrainerAux->positionTo);
	
	//Remove the global objetive
	pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
	pokemonCompareGlobalObjetive = malloc(strlen(pokemonOnMapAux->pokemon)+1);
	strcpy(pokemonCompareGlobalObjetive, pokemonOnMapAux->pokemon);
	list_remove_by_condition(globalObjetive, analyzePokemonInGlobal);
	free(pokemonCompareGlobalObjetive);
	pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
	
	//Add pokemon owned
	pthread_mutex_lock(&threadsTrainers_mutex);
	char* newPokemonOwned = malloc(strlen(pokemonOnMapAux->pokemon)+1);
	strcpy(newPokemonOwned, pokemonOnMapAux->pokemon);
	list_add(trainerAux->pokemonOwned, newPokemonOwned);
	pthread_mutex_unlock(&threadsTrainers_mutex);

	removePokemonOnMap(threadTrainerAux->positionTo);

	//Validate trainer objetive complete
	if(trainerCompleteOwnObjetives(trainerAux)){
		threadTrainerAux->state = E_P_EXIT;
		log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a EXIT, porque cumplió sus objetivos individuales", trainerAux->id_trainer);
		calculateTrainerInExit(threadTrainerAux);
	}else{
		pthread_mutex_lock(&threadsTrainers_mutex);
		if(threadTrainerAux->state != BLOCKED_BY_BROKER){
			//threadTrainerAux->contextSwitchCount++;
			log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a BLOCKED, porque le falta capturar más pokemon", trainerAux->id_trainer);
		}
		threadTrainerAux->state = BLOCKED;
		pthread_mutex_unlock(&threadsTrainers_mutex);
		sem_post(&plannerSemaphore);
	}
}

void removePokemonOnMap(t_position position){
	t_pokemon_on_map* pokemonOnMapAux = getPokemonByPosition(position);
	t_pokemon_on_map* pokemonOnMapToRemove;
	bool continueFor = true;
	setStateToPokemonOnMap(position, P_CATCHED);
	pthread_mutex_lock(&pokemonsOnMap_mutex);
	for(int i=0; i<list_size(pokemonsOnMap) && continueFor; i++){
		pokemonOnMapToRemove = (t_pokemon_on_map*)list_get(pokemonsOnMap, i);
		if(pokemonOnMapToRemove->id == pokemonOnMapAux->id){
			pokemonOnMapToRemove = (t_pokemon_on_map*)list_remove(pokemonsOnMap, i);
			free(pokemonOnMapToRemove->pokemon);
			free(pokemonOnMapToRemove);
			continueFor = false;
		}
	}
	pthread_mutex_unlock(&pokemonsOnMap_mutex);
}

//// algoritmos de planifiacion
//despues de  setTrainerToExec_FirstCome() obtengo el threadTrainerChosen y el pokemonOnMap
void execThreadTrainerSetedFCFS(t_threadTrainer* threadTrainerChosen){
	bool reachDestiny = false;

	setStateToPokemonOnMap(threadTrainerChosen->positionTo, P_CHASING);
	pthread_mutex_lock(&threadsTrainers_mutex);
	threadTrainerChosen->contextSwitchCount++;
	pthread_mutex_unlock(&threadsTrainers_mutex);
	for(int i=0; !reachDestiny; i++){
		reachDestiny = move_to_objetive(threadTrainerChosen);
	}
}
void execThreadTrainerSetedRR(t_threadTrainer* threadTrainerChosen){
	bool continueMoving = true;
	bool reachDestiny = false;
	uint32_t usedCycle = 0;
	setStateToPokemonOnMap(threadTrainerChosen->positionTo, P_CHASING);
	pthread_mutex_lock(&threadsTrainers_mutex);
	threadTrainerChosen->contextSwitchCount++;
	pthread_mutex_unlock(&threadsTrainers_mutex);
	for(int i=0; continueMoving && !reachDestiny; i++){
		usedCycle++;
		reachDestiny = move_to_objetive(threadTrainerChosen);

		if(!reachDestiny && usedCycle == config_values.quantum){
			pthread_mutex_lock(&threadsTrainers_mutex);
			threadTrainerChosen->state = READY;
			//threadTrainerChosen->contextSwitchCount++;
			threadTrainerChosen->incomingTime = time(NULL);
			log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a READY, porque se le acabó el QUANTUM", threadTrainerChosen->trainer->id_trainer);
			pthread_mutex_unlock(&threadsTrainers_mutex);
			continueMoving = false;
			sem_post(&plannerSemaphore);
		}
	}
}
void execThreadTrainerSetedSJF_SD(t_threadTrainer* threadTrainerChosen){
	setStateToPokemonOnMap(threadTrainerChosen->positionTo, P_CHASING);
	
	bool reachDestiny = false;
	pthread_mutex_lock(&threadsTrainers_mutex);
	threadTrainerChosen->contextSwitchCount++;
	pthread_mutex_unlock(&threadsTrainers_mutex);
	for(int i=0; !reachDestiny; i++){
		reachDestiny = move_to_objetive(threadTrainerChosen);
	}
}

void execThreadTrainerSetedSJF_CD(t_threadTrainer* threadTrainerChosen){
	setStateToPokemonOnMap(threadTrainerChosen->positionTo, P_CHASING);
	
	int countPokemonOnReady = calculatePokemonsOnReady();
	bool reachDestiny = false;
	bool continueMoving = true;
	
	threadTrainerChosen->contextSwitchCount++;
	for(int i=0; !reachDestiny && continueMoving; i++){
		reachDestiny = move_to_objetive(threadTrainerChosen);

		if(!reachDestiny){
			if(countPokemonOnReady != calculatePokemonsOnReady()){
				pthread_mutex_lock(&threadsTrainers_mutex);
				threadTrainerChosen->state = READY;
				//threadTrainerChosen->contextSwitchCount++;
				threadTrainerChosen->incomingTime = time(NULL);
				log_info(obligatory_logger, "Entrenador %d, cambia de EXEC a READY, porque apareció un entrenador con estimador menor que está en READY", threadTrainerChosen->trainer->id_trainer);
				pthread_mutex_unlock(&threadsTrainers_mutex);
				sem_post(&plannerSemaphore);
				continueMoving = false;
			}
		}
	}
}

void setStateToPokemonOnMap(t_position positionTo, e_pokemon_catch_state state){
	t_pokemon_on_map* pokemonOnMap = getPokemonByPosition(positionTo);
	if(pokemonOnMap != NULL){
		pthread_mutex_lock(&pokemonsOnMap_mutex);
		pokemonOnMap->state = state;
		pthread_mutex_unlock(&pokemonsOnMap_mutex);
	}
}

int calculatePokemonsOnReady(){
	int count = 0;
	t_threadTrainer* threadTrainerAux;
	pthread_mutex_lock(&threadsTrainers_mutex);
	for(int i=0; i<list_size(threadsTrainers);i++){
		threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
		if(threadTrainerAux->state == READY) count++;
	}
	pthread_mutex_unlock(&threadsTrainers_mutex);

	return count;
}
