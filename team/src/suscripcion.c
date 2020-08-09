#include "suscripcion.h"
#include <string.h>
#include "team.h"

void* connection_broker_global_suscribe() {
    connection_broker_suscribe_to_appeared_pokemon(APPEARED_POKEMON);
    connection_broker_suscribe_to_caught_pokemon(CAUGHT_POKEMON);
    connection_broker_suscribe_to_localized_pokemon(LOCALIZED_POKEMON);
    
    pthread_join(suscripcionAppearedPokemon,NULL);
    pthread_join(suscripcionCaughtPokemon,NULL);
    pthread_join(suscripcionLocalizedPokemon,NULL);

    return NULL;
 }

void connection_broker_suscribe_to_appeared_pokemon(op_code code) {   
    //se envia un connect por cada cola de mensajes a suscribirse
    uint32_t server_connection_appeared_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    send_new_connection(server_connection_appeared_pokemon); 
    
    log_info(obligatory_logger, "Connection to broker succesfully\n"); 
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_appeared_pokemon);    
    pthread_create(&suscripcionAppearedPokemon, NULL, (void*)suscribeOnThreadList, arguments);
}

void connection_broker_suscribe_to_caught_pokemon(op_code code) {	
    uint32_t server_connection_caught_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    send_new_connection(server_connection_caught_pokemon); 

    log_info(obligatory_logger, "Connection to broker succesfully\n");
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_caught_pokemon);   
    pthread_create(&suscripcionCaughtPokemon, NULL, (void*)suscribeOnThreadList, arguments);
}

void connection_broker_suscribe_to_localized_pokemon(op_code code) {
    uint32_t server_connection_localized_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);   
    send_new_connection(server_connection_localized_pokemon); 

    log_info(obligatory_logger, "Connection to broker succesfully\n");	
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_localized_pokemon);   
    pthread_create(&suscripcionLocalizedPokemon, NULL, (void*)suscribeOnThreadList, arguments); 	
}

args_pthread* thread_suscribe_arguments(op_code code, uint32_t socket) {
    args_pthread* arguments = malloc(sizeof(args_pthread)); 
    arguments->codigoCola = code;
    arguments->socket = malloc(sizeof(uint32_t));
    *arguments->socket = socket;
    return arguments;
}

void suscribeOnThreadList(args_pthread* arguments){
    suscribirseA(arguments->codigoCola,*arguments->socket);
    uint32_t socket = 0;
    
    switch(arguments->codigoCola){
        case APPEARED_POKEMON:
            structAppearedPokemon = malloc(sizeof(threadSubscribe));
            structAppearedPokemon->idQueue = APPEARED_POKEMON;
            structAppearedPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structAppearedPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            socket = *arguments->socket;
            free(arguments->socket);
            free(arguments);
            connect_client(socket,APPEARED_POKEMON);
            //connect_client(*arguments->socket,APPEARED_POKEMON);
            break;
        case CAUGHT_POKEMON:
            structCaughtPokemon = malloc(sizeof(threadSubscribe));
            structCaughtPokemon->idQueue = CAUGHT_POKEMON;
            structCaughtPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structCaughtPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            socket = *arguments->socket;
            free(arguments->socket);
            free(arguments);
            connect_client(socket,CAUGHT_POKEMON);
            //connect_client(*arguments->socket,CAUGHT_POKEMON);
            break;
        case LOCALIZED_POKEMON:
            structLocalizedPokemon = malloc(sizeof(threadSubscribe));
            structLocalizedPokemon->idQueue = LOCALIZED_POKEMON;
            structLocalizedPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structLocalizedPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            socket = *arguments->socket;
            free(arguments->socket);
            free(arguments);
            connect_client(socket,LOCALIZED_POKEMON);
            //connect_client(*arguments->socket,LOCALIZED_POKEMON);
            break;
        default:
            break;
    }
}

void connect_client(uint32_t socket,op_code codeOperation){
    t_process_request* process_request = malloc(sizeof(t_process_request)); 
    (*process_request).socket = malloc(sizeof(uint32_t));
    *(*process_request).socket = socket; 
    (*process_request).request_receiver = request;

    uint32_t id_connection = receive_connection_id(socket);

    switch(codeOperation){
        case APPEARED_POKEMON:
            structAppearedPokemon->idConnection = id_connection;
            break;
        case CAUGHT_POKEMON:
            structCaughtPokemon->idConnection = id_connection;
            break;
        case LOCALIZED_POKEMON:
            structLocalizedPokemon->idConnection = id_connection;
            break;
        default:
            break;
    }
    
    while(1){
        serve_client(process_request);
        close(socket);
        socket = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
        process_request = malloc(sizeof(t_process_request)); 
        (*process_request).socket = malloc(sizeof(uint32_t));
        *(*process_request).socket = socket; 
        (*process_request).request_receiver = request;
        send_reconnect(socket, id_connection);
    } 

    pthread_join(server, NULL);
}

void listen_to_gameboy() { 
   start_server(config_values.ip_team, config_values.puerto_team, request);
}

void reception_message_queue_subscription(uint32_t code, uint32_t sizeofstruct, uint32_t client_fd) {
	void* stream = malloc(sizeofstruct);
    uint32_t* id_message = malloc(sizeof(uint32_t));
    uint32_t* id_message_correlational = malloc(sizeof(uint32_t));
    recv(client_fd, stream, sizeofstruct, MSG_WAITALL);

    switch(code){
        case APPEARED_POKEMON:;
            pthread_mutex_lock(&appeared_mutex);
            appeared_pokemon* appeared_pokemon_Message = stream_to_appeared_pokemon(stream, id_message, id_message_correlational, false); 
            appeared_pokemon_Message->pokemon = realloc(appeared_pokemon_Message->pokemon, appeared_pokemon_Message->sizePokemon+1);
            appeared_pokemon_Message->pokemon[appeared_pokemon_Message->sizePokemon] = '\0';
            log_info(obligatory_logger, "Receiving Message Appeared pokemon, Pokemon Appeared: %s, position: (%d,%d)", appeared_pokemon_Message->pokemon, appeared_pokemon_Message->position.posx, appeared_pokemon_Message->position.posy);

            pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
            pokemonCompareGlobalObjetive = malloc(strlen(appeared_pokemon_Message->pokemon)+1);
            strcpy(pokemonCompareGlobalObjetive, appeared_pokemon_Message->pokemon);
            bool anyPokemonInGlobalObjetive = list_any_satisfy(globalObjetive, analyzePokemonInGlobal);
            free(pokemonCompareGlobalObjetive);
            pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
            
            if(anyPokemonInGlobalObjetive){
                pthread_mutex_lock(&pokemonsOnMap_mutex);
                t_pokemon_on_map* newPokemonAppeared = malloc(sizeof(t_pokemon_on_map));
                newPokemonAppeared->state = P_FREE;
                newPokemonAppeared->position.posx = appeared_pokemon_Message->position.posx;
                newPokemonAppeared->position.posy = appeared_pokemon_Message->position.posy;
                newPokemonAppeared->pokemon = malloc(strlen(appeared_pokemon_Message->pokemon)+1);
                newPokemonAppeared->id = list_size(pokemonsOnMap) + 1;
                strcpy(newPokemonAppeared->pokemon, appeared_pokemon_Message->pokemon);
                list_add(pokemonsOnMap, newPokemonAppeared);
                pthread_mutex_unlock(&pokemonsOnMap_mutex);

                sem_post(&plannerSemaphore);
            }
            free(appeared_pokemon_Message->pokemon);
            free_appeared_pokemon(appeared_pokemon_Message);
            send_ack(client_fd, *id_message);
            pthread_mutex_unlock(&appeared_mutex);
            break;
        case CAUGHT_POKEMON:;
            pthread_mutex_lock(&caught_mutex);
			caught_pokemon* caught_Pokemon_Message = stream_to_caught_pokemon(stream, id_message, id_message_correlational, false);
            log_info(obligatory_logger, "Receiving Message Caught pokemon, Result %d", caught_Pokemon_Message->success);
log_info(optional_logger, "Receiving Message Caught pokemon, success: %d, id-message-correlational: %d", caught_Pokemon_Message->success, *id_message_correlational);
            processCaughtPokemon(*id_message_correlational, caught_Pokemon_Message->success);
            
            send_ack(client_fd, *id_message);
            free_caught_pokemon(caught_Pokemon_Message);
            pthread_mutex_unlock(&caught_mutex);
            break;
        case LOCALIZED_POKEMON:;
            pthread_mutex_lock(&localized_mutex);
            localized_pokemon* localized_Pokemon_Message = stream_to_localized_pokemon(stream, id_message, id_message_correlational, false);
            localized_Pokemon_Message->pokemon = realloc(localized_Pokemon_Message->pokemon, localized_Pokemon_Message->sizePokemon+1);
            localized_Pokemon_Message->pokemon[localized_Pokemon_Message->sizePokemon] = '\0';
log_info(optional_logger, "Receiving Message Localized pokemon, pokemon: %s, id-message-correlational: %d", localized_Pokemon_Message->pokemon, *id_message_correlational);
            int indexOfPokemonToLocalyze = getIndexPokemonToLocalizedByMessage(*id_message_correlational);
            if(indexOfPokemonToLocalyze != -1){
                log_info(obligatory_logger, "Receiving Message Localized pokemon, pokemon: %s", localized_Pokemon_Message->pokemon);
                bool mustPlan = false;
                
                for(int i = 0; i<list_size(localized_Pokemon_Message->positions); i++) {
                    t_position* positionAux = list_get(localized_Pokemon_Message->positions, i);
                    log_info(obligatory_logger,"Find in position: (%d,%d)", positionAux->posx, positionAux->posy);

                    pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
                    pokemonCompareGlobalObjetive = malloc(strlen(localized_Pokemon_Message->pokemon)+1);
                    strcpy(pokemonCompareGlobalObjetive, localized_Pokemon_Message->pokemon);
                    bool anyPokemonInGlobalObjetive = list_any_satisfy(globalObjetive, analyzePokemonInGlobal);
                    free(pokemonCompareGlobalObjetive);
                    pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
                    
                    if(anyPokemonInGlobalObjetive){
                        mustPlan = true;
                        t_pokemon_on_map* newPokemonAppeared = malloc(sizeof(t_pokemon_on_map));
                        newPokemonAppeared->state = P_FREE;
                        newPokemonAppeared->position.posx = positionAux->posx;
                        newPokemonAppeared->position.posy = positionAux->posy;
                        newPokemonAppeared->pokemon = malloc(strlen(localized_Pokemon_Message->pokemon)+1);
                        pthread_mutex_lock(&pokemonsOnMap_mutex);
                        newPokemonAppeared->id = list_size(pokemonsOnMap) + 1;
                        strcpy(newPokemonAppeared->pokemon, localized_Pokemon_Message->pokemon);
                        list_add(pokemonsOnMap, newPokemonAppeared);
                        pthread_mutex_unlock(&pokemonsOnMap_mutex);
                    }
                }
                if(mustPlan) sem_post(&plannerSemaphore);
                send_ack(client_fd, *id_message);
                //list_remove(pokemonsToLocalize, indexOfPokemonToLocalyze);
                //list_remove_and_destroy_element(pokemonsToLocalize, indexOfPokemonToLocalyze, (void*)destroy_pokemonsToLocalize);
            }
            free(localized_Pokemon_Message->pokemon);
            free_localized_pokemon(localized_Pokemon_Message);
            pthread_mutex_unlock(&localized_mutex);
            break;
         case CONNECTION:;
            connection* connectionMessage = stream_to_connection(stream);
            pthread_mutex_lock(&threadSubscribeList_mutex);
            threadSubscribe* thread = list_find_with_args(threadSubscribeList, compareSockets, (void*)client_fd);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            thread->idConnection = connectionMessage->id_connection;

            suscribirseA(thread->idQueue, client_fd);

            break;
    }
    free(stream);
    free(id_message);
    free(id_message_correlational);
}
bool compareSockets(void* element, void* args){
    threadSubscribe* thread = (threadSubscribe*) element;
    
    return thread->socket == (uint32_t) args;
}

void* send_get_pokemon_global_team(){
    uint32_t client_fd = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    uint32_t* id_message = malloc(sizeof(uint32_t));
    get_pokemon* getPokemonMessage;
    void* stream;
    char* pokemonToSend;
    pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
    int globalObjetiveCount = list_size(globalObjetive);
    pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);

    for(int i=0; i< globalObjetiveCount; i++) {
        pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
        pokemonToSend = (char*)list_get(globalObjetive, i);
        getPokemonMessage = malloc(sizeof(get_pokemon));
        getPokemonMessage->pokemon = malloc(strlen(pokemonToSend)+1);
        strcpy(getPokemonMessage->pokemon, pokemonToSend);
        getPokemonMessage->sizePokemon = strlen(getPokemonMessage->pokemon);
        pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
        *id_message = -1;
        stream =get_pokemon_to_stream(getPokemonMessage, id_message);

        t_paquete* packageToSend = stream_to_package(GET_POKEMON, stream, size_of_get_pokemon(getPokemonMessage));
        int bytes = packageToSend->buffer->size + 2*sizeof(uint32_t);
        void* buffer = (void *) serializar_paquete(packageToSend, bytes);

        send(client_fd, buffer, bytes, 0);
        free(buffer);

        uint32_t sizeOfBuffer = sizeof(uint32_t) * 3;
        buffer = malloc(sizeOfBuffer);
        recv(client_fd, buffer, sizeOfBuffer, MSG_WAITALL);
        ack* acknowledgementMessage = stream_to_ack(buffer+8);
        log_info(optional_logger, "Send get pokemon: Pokemon: %s - Id-message: %d", getPokemonMessage->pokemon, acknowledgementMessage->id_message);
        free(getPokemonMessage->pokemon);
        free(getPokemonMessage);
        free_package(packageToSend);

        addPokemonToLocalize(pokemonToSend, acknowledgementMessage->id_message);
        free(buffer);
        free(acknowledgementMessage);
    }
    free(id_message);
    return NULL;
}

void addPokemonToLocalize(char* pokemon, uint32_t idMessage){
    t_pokemonToLocalized* pokemonToLocalizeAux;
    bool pokemonExists = false;

    for(int i=0; i<list_size(pokemonsToLocalize) && !pokemonExists;i++){
        pokemonToLocalizeAux = (t_pokemonToLocalized*)list_get(pokemonsToLocalize, i);
        if(strcmp(pokemon, pokemonToLocalizeAux->pokemon) == 0){
            pokemonExists = true;
        }
    }

    if(!pokemonExists){
        pokemonToLocalizeAux = malloc(sizeof(t_pokemonToLocalized));
        pokemonToLocalizeAux->idMessage = idMessage;
        pokemonToLocalizeAux->pokemon = malloc(strlen(pokemon)+1);
        strcpy(pokemonToLocalizeAux->pokemon, pokemon);
        list_add(pokemonsToLocalize, pokemonToLocalizeAux);
    }
}

int getIndexPokemonToLocalizedByMessage(uint32_t id_message){
    int result = -1;
    t_pokemonToLocalized* pokemonToLocalizeAux;

    for(int i=0; i<list_size(pokemonsToLocalize) && result == -1; i++){
        pokemonToLocalizeAux = (t_pokemonToLocalized*)list_get(pokemonsToLocalize, i);

        if(pokemonToLocalizeAux->idMessage == id_message){
            result = i;
        }
    }

    return result;
}

void processCaughtPokemon(uint32_t id_message, uint32_t success){
    t_threadTrainer* threadTrainerAux;
    pthread_mutex_lock(&threadsTrainers_mutex);
    int threadsTrainersCount = list_size(threadsTrainers);
    pthread_mutex_unlock(&threadsTrainers_mutex);
    for(int i=0; i<threadsTrainersCount; i++){
        pthread_mutex_lock(&threadsTrainers_mutex);
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        pthread_mutex_unlock(&threadsTrainers_mutex);
        if(threadTrainerAux->idMessageCatch == id_message){
            if(success == 1){
                catch_succesfull(threadTrainerAux);
                return;
            }else{
                pthread_mutex_lock(&threadsTrainers_mutex);
                threadTrainerAux->state = BLOCKED;
                removePokemonOnMap(threadTrainerAux->positionTo);
                sem_post(&plannerSemaphore);
                pthread_mutex_unlock(&threadsTrainers_mutex);
            }
        }
    }
}