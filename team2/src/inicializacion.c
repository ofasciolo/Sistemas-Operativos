
#include "inicializacion.h"
#include "suscripcion.h"
#include "mapa.h"
#include <string.h>
#include "deadlock.h"

void initialize_team() { 
    create_mutex();
    pokemonsToLocalize = list_create();
    pokemonsOnMap = list_create();
    flagExistsDeadlock = false;
    threadSubscribeList = list_create();
    read_config();
    create_obligatory_logger();
    create_optional_logger();
    load_values_config();
    
    assign_data_trainer();
    calculate_global_objetives();
    
    request = &reception_message_queue_subscription;
    pthread_create(&brokerSuscriptionThread, NULL, connection_broker_global_suscribe, NULL);
    listen_to_gameboy();
    pthread_create(&sendGetPokemonThread, NULL, send_get_pokemon_global_team, NULL);
    sem_init(&plannerSemaphore, 0, 0);
    pthread_create(&plannerThread, NULL, planTrainers, NULL);
    validateEndTeam();
}

void create_mutex(){
    pthread_mutex_init(&mutexPokemonCompareDeadlock, NULL);
    pthread_mutex_init(&flagExistsDeadlock_mutex, NULL);
    pthread_mutex_init(&pokemonCompareGlobalObjetive_mutex, NULL);
    pthread_mutex_init(&pokemonsOnMap_mutex, NULL);
    pthread_mutex_init(&threadSubscribeList_mutex, NULL);
    pthread_mutex_init(&threadsTrainers_mutex, NULL);
    pthread_mutex_init(&localized_mutex, NULL);
    pthread_mutex_init(&appeared_mutex, NULL);
    pthread_mutex_init(&caught_mutex, NULL);

}
void destroy_mutex(){
    pthread_mutex_destroy(&mutexPokemonCompareDeadlock);
    pthread_mutex_destroy(&flagExistsDeadlock_mutex);
    pthread_mutex_destroy(&pokemonCompareGlobalObjetive_mutex);
    pthread_mutex_destroy(&pokemonsOnMap_mutex);
    pthread_mutex_destroy(&threadSubscribeList_mutex);
    pthread_mutex_destroy(&threadsTrainers_mutex);
    pthread_mutex_destroy(&localized_mutex);
    pthread_mutex_destroy(&appeared_mutex);
    pthread_mutex_destroy(&caught_mutex);
}
void validateEndTeam(){
    for(int i=0; i<list_size(threadsTrainers); i++){
        t_threadTrainer* threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        pthread_join(threadTrainerAux->threadTrainer,NULL);
    }

    pthread_cancel(plannerThread);
    sem_destroy(&plannerSemaphore);
    pthread_join(plannerThread,NULL);

    writeTeamMetrics();
    finishTeam();
}

void* planTrainers(){
    while(true){
        sem_wait(&plannerSemaphore);
        calculateTrainerFromNewToReady();
        calculateLeaveBlockedFromAppear();
        calculateTrainerFromReadyToExec();
        detectDeadlock_do();
    }
}

void* trainerDo(void* ptrThreadTrainer){
    t_threadTrainer* threadTrainerAux = (t_threadTrainer*)ptrThreadTrainer;
    while(true){
        sem_wait(&(threadTrainerAux->semaphoreAction));
        //Actions according state
        if(threadTrainerAux->state == READY){
            calculateTrainerFromReadyToExec();
        }else if(threadTrainerAux->state == EXEC){
            executeAlgorithm();
        }else if(threadTrainerAux->state == E_P_EXIT){
            calculateTrainerInExit(threadTrainerAux);
        }
    }

    return NULL;
}

void read_config() {   
    char* config_path = "./cfg/team.config";
    config = config_create(config_path); 
    if(config == NULL) {
        error_show("Error creating TEAM process config on %s\t", config_path);
        exit(CONFIG_FAIL);
    }    
}

void create_obligatory_logger() {      
    char* log_obligatory_config = config_get_string_value(config, "LOG_FILE");
    obligatory_logger = log_create(log_obligatory_config, "TEAM", 1, LOG_LEVEL_INFO);
    if(obligatory_logger == NULL) {
        error_show("Error creating TEAM process obligatory logger %s\n", log_obligatory_config);
        exit(LOG_FAIL);
    }
    log_info(obligatory_logger, "Obligatory Log was created successfully\n");
}
 
 void create_optional_logger() {        
    char* log_optional_config = config_get_string_value(config, "LOG_FILE_OPTIONAL");
    optional_logger = log_create(log_optional_config, "TEAM", 0, LOG_LEVEL_INFO);
    if(optional_logger == NULL) {
        error_show("Error creating TEAM process optional logger %s\n", log_optional_config);
        exit(LOG_FAIL);
    }
    log_info(optional_logger, "Optional Log was created successfully\n");
}

void load_values_config() {
    config_values.tiempo_reconexion = (uint32_t)config_get_int_value(config, "TIEMPO_RECONEXION");
    config_values.retardo_ciclo_cpu = (uint32_t)config_get_int_value(config, "RETARDO_CICLO_CPU");
    config_values.algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_values.quantum = (uint32_t)config_get_int_value(config, "QUANTUM");
    config_values.alpha = (double)config_get_double_value(config, "ALPHA");
    config_values.estimacion_inicial = (uint32_t)config_get_int_value(config, "ESTIMACION_INICIAL");
    config_values.ip_broker = config_get_string_value(config, "IP_BROKER");
    config_values.puerto_broker= config_get_string_value(config, "PUERTO_BROKER");
    config_values.ip_team = config_get_string_value(config, "IP_TEAM");
    config_values.puerto_team = config_get_string_value(config, "PUERTO_TEAM");

log_info(optional_logger, "Algorithm: %s - Quantum: %d - ALPHA: %f - ESTIMACION INICIAL: %d", config_values.algoritmo_planificacion, config_values.quantum, config_values.alpha, config_values.estimacion_inicial);
}

void assign_data_trainer() {
    threadsTrainers = list_create();
    t_trainer *data_trainer;
    t_threadTrainer* threadTrainerAux;
    char** init_position = config_get_array_value(config, "POSICIONES_ENTRENADORES");
    char** pokemonOwns = config_get_array_value(config, "POKEMON_ENTRENADORES");
    char** pokemonNeeds = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
    char** valorAux;
    char* dataAux;

    for(uint32_t i = 0; init_position[i] != NULL; i++) {
        data_trainer = malloc(sizeof(t_trainer));
        if(data_trainer != NULL) {
            data_trainer->id_trainer = i+1;
            data_trainer->pokemonOwned = list_create();
            data_trainer->pokemonNeeded = list_create();

            valorAux = string_split(init_position[i], "|");
            data_trainer->position.posx = (uint32_t)atoi(valorAux[0]);
            data_trainer->position.posy = (uint32_t)atoi(valorAux[1]);
            freeArrayConfigValue(valorAux);
            free(valorAux);

            if(pokemonNeeds != NULL && pokemonNeeds[i] != NULL && !string_is_empty(pokemonNeeds[i])){
                if(strchr(pokemonNeeds[i], '|') == NULL){
                    dataAux = malloc(strlen(pokemonNeeds[i])*sizeof(char)+1);
                    strcpy(dataAux, pokemonNeeds[i]);
                    list_add(data_trainer->pokemonNeeded, dataAux);
                }else{
                    valorAux = string_split(pokemonNeeds[i], "|");
                    addValuesToListFromArray(valorAux, data_trainer->pokemonNeeded);
                    freeArrayConfigValue(valorAux);
                    free(valorAux);
                }
            }

            //list_add(trainers, (void*)data_trainer);

            //Creation of trainer thread
            threadTrainerAux = malloc(sizeof(t_threadTrainer));
            threadTrainerAux->state = NEW;
            threadTrainerAux->incomingTime = time(NULL);
            threadTrainerAux->valueEstimator = config_values.estimacion_inicial; //Needed for SJF
            threadTrainerAux->contextSwitchCount = 0;
            threadTrainerAux->interchangeCycleCount = 0;
            threadTrainerAux->cpuCycleCount = 0;
            threadTrainerAux->destinyIsTrainer = false;
            threadTrainerAux->trainer = data_trainer;
            
            sem_init(&(threadTrainerAux->semaphoreAction), 0, 0);

            list_add(threadsTrainers, (void*)threadTrainerAux);
            pthread_create(&threadTrainerAux->threadTrainer, NULL, trainerDo, (void*)threadTrainerAux);

            log_info(obligatory_logger, "Entrenador %d entra a cola NEW.", data_trainer->id_trainer);
        }else{
            log_info(optional_logger, "Error on request malloc to TRAINER \n");
        }
    }

    for(uint32_t i = 0; pokemonOwns[i] != NULL; i++) {
        data_trainer = ((t_threadTrainer*)list_get(threadsTrainers, i))->trainer;
        if(!string_is_empty(pokemonOwns[i])){
            if(strchr(pokemonOwns[i], '|') == NULL){
                dataAux = malloc(strlen(pokemonOwns[i])+1);
                strcpy(dataAux, pokemonOwns[i]);
                list_add(data_trainer->pokemonOwned, dataAux);
            }else{
                valorAux = string_split(pokemonOwns[i], "|");
                addValuesToListFromArray(valorAux, data_trainer->pokemonOwned);
                freeArrayConfigValue(valorAux);
                free(valorAux);
            }
        }
    }

    freeArrayConfigValue(init_position);
    free(init_position);
    freeArrayConfigValue(pokemonOwns);
    free(pokemonOwns);
    freeArrayConfigValue(pokemonNeeds);
    free(pokemonNeeds);
    return;
}

void addValuesToListFromArray(char** valorAux, t_list* listDestiny){
    char* dataAux;
    while(*valorAux != NULL){
        dataAux = malloc(strlen(*valorAux)*sizeof(char)+1);
        strcpy(dataAux, *valorAux);
        list_add(listDestiny, dataAux);
        valorAux++;
    };
}

void freeArrayConfigValue(char** valorAux){
    while(*valorAux != NULL){
        free(*valorAux);
        valorAux++;
    };
}

void calculate_global_objetives(){
    //Its the join of all needs of trainers, minus the pokemon that already have
    globalObjetive = list_create();
    deadlockCount = 0;
    t_trainer* trainerAux;
    int i, j;
    uint32_t pokemonsOwnedCount;
    char* pokemonOwnedAux;

    //Join all needs
    for(i = 0; i < list_size(threadsTrainers); i++){
        trainerAux = ((t_threadTrainer*)list_get(threadsTrainers, i))->trainer;
        list_add_all(globalObjetive, trainerAux->pokemonNeeded);
    }

    //rest already in stock
    for(i = 0; i < list_size(threadsTrainers); i++){
        trainerAux = ((t_threadTrainer*)list_get(threadsTrainers, i))->trainer;

        pokemonsOwnedCount = list_size(trainerAux->pokemonOwned);
        for(j = 0; j < pokemonsOwnedCount; j++){
            pokemonOwnedAux = (char*)list_get(trainerAux->pokemonOwned, j);
            pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
            pokemonCompareGlobalObjetive = malloc(strlen(pokemonOwnedAux)+1);
            strcpy(pokemonCompareGlobalObjetive, pokemonOwnedAux);
            list_remove_by_condition(globalObjetive, analyzePokemonInGlobal);
            free(pokemonCompareGlobalObjetive);
            pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
        }
    }
}

bool analyzePokemonInGlobal(void* objetiveGlobal){
    if(strcmp(objetiveGlobal, pokemonCompareGlobalObjetive) == 0){
        return 1;
    }else{
        return 0;
    }
}

void release_resources() { 
    pthread_cancel(suscripcionAppearedPokemon);
    pthread_cancel(suscripcionCaughtPokemon);
    pthread_cancel(suscripcionLocalizedPokemon);

    pthread_cancel(sendGetPokemonThread);
    pthread_cancel(brokerSuscriptionThread);
    pthread_cancel(server);

    pthread_join(sendGetPokemonThread,NULL);
    pthread_join(brokerSuscriptionThread,NULL);
    pthread_join(server,NULL);
/*
    pthread_join(suscripcionAppearedPokemon,NULL);
    pthread_join(suscripcionCaughtPokemon,NULL);
    pthread_join(suscripcionLocalizedPokemon,NULL);*/

    if(config)
        config_destroy(config);

    if(obligatory_logger)
        log_destroy(obligatory_logger);

    if(optional_logger)
        log_destroy(optional_logger);

    destroy_lists_and_loaded_elements();
    destroy_mutex();
}

void destroy_trainer(void* pointer){   
    t_trainer* trainerAux = (t_trainer*)pointer;
    list_destroy_and_destroy_elements(trainerAux->pokemonOwned, free);
    list_destroy_and_destroy_elements(trainerAux->pokemonNeeded, free);
    free(pointer);
}
void destroy_threadTrainer(void* pointer){
    t_threadTrainer* threadTrainerAux = (t_threadTrainer*)pointer;
    destroy_trainer((void*)threadTrainerAux->trainer);
    sem_destroy(&threadTrainerAux->semaphoreAction);
    free(pointer);
}
void destroy_pokemonsOnMap(void* pointer){
    t_pokemon_on_map* pokemonOnMapAux = (t_pokemon_on_map*)pointer;
    free(pokemonOnMapAux->pokemon);
    free(pointer);
}
void destroy_pokemonsToLocalize(void* pointer){
    t_pokemonToLocalized* pokemonToLocalizedAux = (t_pokemonToLocalized*)pointer;
    if(pokemonToLocalizedAux->pokemon != NULL) free(pokemonToLocalizedAux->pokemon);
    free(pointer);
}

void destroy_lists_and_loaded_elements(){
    list_destroy_and_destroy_elements(threadsTrainers, (void*)destroy_threadTrainer);
    list_destroy_and_destroy_elements(threadSubscribeList, free);
    list_destroy_and_destroy_elements(pokemonsOnMap, (void*)destroy_pokemonsOnMap);
    list_destroy_and_destroy_elements(globalObjetive, free);
    list_destroy_and_destroy_elements(pokemonsToLocalize, (void*)destroy_pokemonsToLocalize);
}
