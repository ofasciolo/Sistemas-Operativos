#include "iniciarGameCard.h"

void receiveMessage(uint32_t cod_op, uint32_t sizeofstruct, uint32_t client_fd) {
	void* stream = malloc(sizeofstruct);
    uint32_t* id_message = malloc(sizeof(uint32_t));
    *id_message = 0;
    if (recv(client_fd, stream, sizeofstruct, MSG_WAITALL)<=0){free(stream); return;}

    switch(cod_op){
        case NEW_POKEMON:;
            new_pokemon* newPokemonMessage = stream_to_new_pokemon(stream,id_message,false); 
            log_info(obligatory_logger, "Recibi NEW POKEMON");
            
            send_ack(client_fd, *id_message);

            threadPokemonMessage* threadNewPokemonMessage = malloc(sizeof(threadPokemonMessage));
            threadNewPokemonMessage->pokemon = newPokemonMessage;

            pthread_mutex_lock(&structNewPokemon->mutex);
            if(structNewPokemon->socket != 0){
                threadNewPokemonMessage->client_fd = structNewPokemon->socket;
            }else{
                threadNewPokemonMessage->client_fd = 0;
            }
            pthread_mutex_unlock(&structNewPokemon->mutex);
            threadNewPokemonMessage->id_mensaje = id_message;

            pthread_mutex_lock(&createAndDetachMutex);
            pthread_create_and_detach(newPokemonTallGrass, threadNewPokemonMessage);
            pthread_mutex_unlock(&createAndDetachMutex);
            
            break;
        case CATCH_POKEMON:;
            log_info(obligatory_logger, "Recibi CATCH POKEMON");
            catch_pokemon* catchPokemonMessage = stream_to_catch_pokemon(stream,id_message,false);

            send_ack(client_fd, *id_message);
            
            threadPokemonMessage* threadCatchPokemonMessage = malloc(sizeof(threadPokemonMessage));
            threadCatchPokemonMessage->pokemon = catchPokemonMessage;
            
            pthread_mutex_lock(&structCatchPokemon->mutex);
            if(structCatchPokemon->socket != 0){
                threadCatchPokemonMessage->client_fd = structCatchPokemon->socket;
            }else{
                threadCatchPokemonMessage->client_fd = 0;
            }
            pthread_mutex_unlock(&structCatchPokemon->mutex);
            threadCatchPokemonMessage->id_mensaje = id_message;

            pthread_mutex_lock(&createAndDetachMutex);
            pthread_create_and_detach(catchPokemonTallGrass, threadCatchPokemonMessage);
            pthread_mutex_unlock(&createAndDetachMutex);
            
            break;
        case GET_POKEMON:;
            log_info(obligatory_logger, "Recibi GET POKEMON");
            get_pokemon* getPokemonMessage = stream_to_get_pokemon(stream,id_message,false);

            send_ack(client_fd, *id_message);
            //TODO SEND LOCALIZED

            threadPokemonMessage* threadGetPokemonMessage = malloc(sizeof(threadPokemonMessage));
            threadGetPokemonMessage->pokemon = getPokemonMessage;
            pthread_mutex_lock(&structGetPokemon->mutex);
            if(structGetPokemon->socket != 0){
                threadGetPokemonMessage->client_fd = structGetPokemon->socket;
            }else{
                threadGetPokemonMessage->client_fd = 0;
            }
            pthread_mutex_unlock(&structGetPokemon->mutex);
            threadGetPokemonMessage->id_mensaje = id_message;

            pthread_mutex_lock(&createAndDetachMutex);
            pthread_create_and_detach(getPokemonTallGrass, threadGetPokemonMessage);
            pthread_mutex_unlock(&createAndDetachMutex);
        
            break;
        case CONNECTION:;
            connection* connectionMessage = stream_to_connection(stream);

            threadSubscribe* thread = list_find_with_args(threadSubscribeList, compareSockets, (void*)client_fd);
            thread->idConnection = connectionMessage->id_connection;

            suscribirseA(thread->idCola, client_fd);

            free(id_message);
            break;
        default:;
            free(id_message);
    }

    free(stream);
    //free(id_message);
}

void iniciarMutex(){
    pthread_mutex_init(&mutexListOfMutex, NULL);
    mutexListDirectory = list_create();

    DIR* dirp; 
    struct dirent *direntp; 

    dirp = opendir(filesPath);

    while((direntp = readdir(dirp)) != NULL){
        if(direntp->d_type == DT_DIR){
            mutexDirectory* mutexDirectoryRead = malloc(sizeof(mutexDirectory));

            char* directory = malloc(strlen(direntp->d_name) + 2);
            strcpy(directory, "");
            strcat(directory, direntp->d_name);
            strcat(directory, "\0");

            mutexDirectoryRead->nombreDirectorio = directory;
            pthread_mutex_init(&mutexDirectoryRead->mutex, NULL);

            pthread_mutex_lock(&mutexListOfMutex); 
            list_add(mutexListDirectory, mutexDirectoryRead);
            pthread_mutex_unlock(&mutexListOfMutex);
        }
    }

    closedir(dirp);
}

bool compareSockets(void* element, void* args){
    
    threadSubscribe* thread = (threadSubscribe*) element;
    
    return thread->socket == (uint32_t) args;
}

void iniciarGameCard(){

    config = config_create("./cfg/gamecard.config");
    uint32_t showConsole = config_get_int_value(config,"LOG_SHOW");
    obligatory_logger = log_create("./cfg/obligatory.log", "obligatory", true, LOG_LEVEL_INFO); 
    optional_logger = log_create("./cfg/optional.log", "optional", showConsole, LOG_LEVEL_INFO);
    char* IP_GAMECARD = config_get_string_value(config,"IP_GAMECARD");
    char* PUERTO_GAMECARD = config_get_string_value(config,"PUERTO_GAMECARD");
    PUERTO_BROKER = config_get_string_value(config,"PUERTO_BROKER");
    IP_BROKER = config_get_string_value(config,"IP_BROKER");
    PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"); 

    iniciarTallGrass();
    iniciarMutex();

    request = &receiveMessage; 

    pthread_mutex_init(&mutexthreadSubscribeList, NULL);    
    pthread_mutex_init(&mutexBitmap, NULL);
    pthread_mutex_init(&metadata_create, NULL);
    pthread_mutex_init(&createBlock, NULL);
    pthread_mutex_init(&createAndDetachMutex, NULL);
    sem_init(&semServeClient,0,0);

    threadSubscribeList = list_create();

    pthread_create(&threadForThreads,NULL,(void*)suscribirseATodo, NULL);

    sem_wait(&semServeClient);
    start_server(IP_GAMECARD,PUERTO_GAMECARD,request);

    pthread_join(server, NULL);
    pthread_join(suscripcionNewPokemon,NULL);
    pthread_join(suscripcionCatchPokemon,NULL);
    pthread_join(suscripcionGetPokemon,NULL);
    pthread_join(threadForThreads, NULL);

    finalizarGameCard();
}

void suscribirseATodo(){ 

    structNewPokemon = malloc(sizeof(threadSubscribe));
    structNewPokemon->socket = 0;
    pthread_mutex_init(&structNewPokemon->mutex, NULL);

    structCatchPokemon = malloc(sizeof(threadSubscribe));
    structCatchPokemon->socket = 0;
    pthread_mutex_init(&structCatchPokemon->mutex, NULL);

    structGetPokemon = malloc(sizeof(threadSubscribe));
    structGetPokemon->socket = 0;
    pthread_mutex_init(&structGetPokemon->mutex, NULL);

    sem_post(&semServeClient);

    uint32_t socket_new_pokemon = crear_conexion(IP_BROKER,PUERTO_BROKER);
    crearSuscripcion(socket_new_pokemon, NEW_POKEMON, &suscripcionNewPokemon);
    
    uint32_t socket_catch_pokemon = crear_conexion(IP_BROKER,PUERTO_BROKER);
    crearSuscripcion(socket_catch_pokemon, CATCH_POKEMON, &suscripcionCatchPokemon);

    uint32_t socket_get_pokemon = crear_conexion(IP_BROKER,PUERTO_BROKER);
    crearSuscripcion(socket_get_pokemon, GET_POKEMON, &suscripcionGetPokemon);
}

void crearSuscripcion(uint32_t socket,op_code codeOperation, pthread_t* threadName){
    
    send_new_connection(socket);
    args_pthread* arguments = malloc(sizeof(args_pthread)); 
    arguments->codigoCola = codeOperation;
    arguments->socket = malloc(sizeof(uint32_t));
    *arguments->socket = socket;

    pthread_create(threadName,NULL,(void*)subscribeAndConnect, arguments);
   
}

void subscribeAndConnect(args_pthread* arguments){
    uint32_t id_connection = receive_connection_id(*arguments->socket);
    suscribirseA(arguments->codigoCola,*arguments->socket);

    switch(arguments->codigoCola){
        case NEW_POKEMON:
            
            pthread_mutex_lock(&structNewPokemon->mutex);
            structNewPokemon->idCola = NEW_POKEMON;
            structNewPokemon->socket = *arguments->socket;
            structNewPokemon->idConnection = id_connection;
            pthread_mutex_unlock(&structNewPokemon->mutex);
           
            pthread_mutex_lock(&mutexthreadSubscribeList);
            list_add(threadSubscribeList, structNewPokemon);
            pthread_mutex_unlock(&mutexthreadSubscribeList);
            
            connect_client(*arguments->socket, NEW_POKEMON);
            break;
        case CATCH_POKEMON:

            pthread_mutex_lock(&structCatchPokemon->mutex);
            structCatchPokemon->idCola = CATCH_POKEMON;
            structCatchPokemon->socket = *arguments->socket;
            structCatchPokemon->idConnection = id_connection;
            pthread_mutex_unlock(&structCatchPokemon->mutex);

            pthread_mutex_lock(&mutexthreadSubscribeList);
            list_add(threadSubscribeList, structCatchPokemon);
            pthread_mutex_unlock(&mutexthreadSubscribeList);

            connect_client(*arguments->socket, CATCH_POKEMON);
            break;
        case GET_POKEMON:

            pthread_mutex_lock(&structGetPokemon->mutex);
            structGetPokemon->idCola = GET_POKEMON;
            structGetPokemon->socket = *arguments->socket;
            structGetPokemon->idConnection = id_connection;
            pthread_mutex_unlock(&structGetPokemon->mutex);
            
            pthread_mutex_lock(&mutexthreadSubscribeList);
            list_add(threadSubscribeList, structGetPokemon);
            pthread_mutex_unlock(&mutexthreadSubscribeList);

            connect_client(*arguments->socket, GET_POKEMON);
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
    
    threadSubscribe* thread = list_find_with_args(threadSubscribeList, compareSockets, (void*)socket);

    while(1){
        serve_client(process_request);
        close(socket);
        socket = crear_conexion(IP_BROKER, PUERTO_BROKER);
        process_request = malloc(sizeof(t_process_request)); 
        (*process_request).socket = malloc(sizeof(uint32_t));
        *(*process_request).socket = socket; 
        (*process_request).request_receiver = request;
        pthread_mutex_lock(&thread->mutex);
        send_reconnect(socket, thread->idConnection);
        pthread_mutex_unlock(&thread->mutex);
    } 

    pthread_join(server, NULL);
}

void finalizarGameCard(){
    log_destroy(obligatory_logger);
    log_destroy(optional_logger);
    config_destroy(config);
    close(socket_broker);
    list_destroy_and_destroy_elements(mutexListDirectory,free);
}
