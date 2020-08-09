#include "server_gameboy.h"

void send_message(char** message, int socket_cliente,t_log*  optional_logger){
	t_paquete* paquete = malloc(sizeof(t_paquete));
    uint32_t* id_message = malloc(sizeof(uint32_t)); 
    uint32_t* id_correlational = malloc(sizeof(uint32_t));
    //A quien se lo mando
    char* receiver = message[1];

    //El nombre de la funcion esta en el parametro 2 del argv que le paso como parametro
    char* message_function = message[2]; 

    //Encuentro el op_code correspondiente con lo que recibi de mensaje.
    op_code operation_code;
    if(strcmp(message[1], "SUSCRIPTOR") == 0){
        operation_code = stringToEnum(message[1]);    
    }else{
        operation_code = stringToEnum(message_function);
    }
    
    paquete->buffer = malloc(sizeof(t_buffer));
    //Filtro por codigo de operacion segun el enum y creo el mensaje al servidor

    switch(operation_code){
        case NEW_POKEMON:;
            new_pokemon* newPokemonMessage = init_new_pokemon(message[3], atoi(message[4]), atoi(message[5]), atoi(message[6]));
        
            paquete->codigo_operacion = NEW_POKEMON;
            paquete->buffer->size = size_of_new_pokemon(newPokemonMessage);

            if(strcmp(receiver, "GAMECARD") == 0){
                *id_message = atoi(message[7]);
            }else{
                *id_message = -1;
            }
            paquete->buffer->stream = new_pokemon_to_stream(newPokemonMessage, id_message);
            free_new_pokemon(newPokemonMessage);
            break;
        case APPEARED_POKEMON:; 
            appeared_pokemon* appearedPokemonMessage = init_appeared_pokemon(message[3], atoi(message[4]), atoi(message[5]));

            paquete->codigo_operacion = APPEARED_POKEMON;
            paquete->buffer->size =  size_of_appeared_pokemon(appearedPokemonMessage);
            
            *id_message = -1;
            if(strcmp(receiver, "BROKER") == 0){
                *id_correlational = atoi(message[6]);
            }else{
                *id_correlational = -1;
            }
            paquete->buffer->stream = appeared_pokemon_to_stream(appearedPokemonMessage, id_message, id_correlational);
            free_appeared_pokemon(appearedPokemonMessage);
            break;
        case CATCH_POKEMON:;
            catch_pokemon* catchPokemonMessage = init_catch_pokemon(message[3], atoi(message[4]), atoi(message[5]));

            paquete->codigo_operacion = CATCH_POKEMON;
            paquete->buffer->size = size_of_catch_pokemon(catchPokemonMessage);
            
            if(strcmp(receiver, "GAMECARD") == 0){
                *id_message = atoi(message[6]);
            }else{
                *id_message = -1;
            }
            paquete->buffer->stream = catch_pokemon_to_stream(catchPokemonMessage, id_message);
            free_catch_pokemon(catchPokemonMessage);
            break;
        case CAUGHT_POKEMON:; 
            caught_pokemon* caughtPokemonMessage; 

            if(strcmp(message[4],"OK") == 0){
                caughtPokemonMessage = init_caught_pokemon(1);
            }else{
                caughtPokemonMessage = init_caught_pokemon(0);
            }

            *id_message = -1; 
            *id_correlational = atoi(message[3]);

            paquete->codigo_operacion = CAUGHT_POKEMON;
            paquete->buffer->size = size_of_caught_pokemon(caughtPokemonMessage);
            paquete->buffer->stream = caught_pokemon_to_stream(caughtPokemonMessage, id_message, id_correlational);
            free_caught_pokemon(caughtPokemonMessage);
            break;
        case GET_POKEMON:;
            get_pokemon* getPokemonMessage = init_get_pokemon(message[3]);
            if(strcmp(receiver, "GAMECARD") == 0){
                *id_message = atoi(message[3]);
            }else{
                *id_message = -1;
            }
            
            paquete->codigo_operacion = GET_POKEMON;
            paquete->buffer->size = size_of_get_pokemon(getPokemonMessage);
            paquete->buffer->stream = get_pokemon_to_stream(getPokemonMessage, id_message);
            free_get_pokemon(getPokemonMessage);
            break;
        case LOCALIZED_POKEMON:;
            t_list* positions = list_create();
            uint32_t sizePositions = atoi(message[4]);
            for (int i = 0; i < sizePositions; i++){
                t_position* position = malloc(sizeof(position));
                position->posx = atoi(message[i*2 + 5]);
                position->posy = atoi(message[i*2 + 6]);
                list_add(positions, position);
            }
            if(strcmp(receiver, "TEAM") == 0){
                *id_correlational = atoi(message[6 + sizePositions*2]);
                *id_message = atoi(message[5 + sizePositions*2]);
            }else{
                *id_message = -1;
                *id_correlational = atoi(message[5 + sizePositions*2]);
            }
                
            localized_pokemon* localizedPokemonMessage = init_localized_pokemon(message[3], positions);
            paquete->codigo_operacion = LOCALIZED_POKEMON;
            paquete->buffer->size = size_of_localized_pokemon(localizedPokemonMessage);
            paquete->buffer->stream = localized_pokemon_to_stream(localizedPokemonMessage, id_message, id_correlational);
            free_localized_pokemon(localizedPokemonMessage);
            break;
        case SUSCRIPTOR:;
            subscribe* subscriber = init_subscribe(stringToEnum(message[2]));

            paquete->codigo_operacion = SUSCRIPTOR; 
            paquete->buffer->size = sizeof(uint32_t);
            paquete->buffer->stream = subscribe_to_stream(subscriber);
            free_subscribe(subscriber);
            break;
        case ERROR:;
            paquete->codigo_operacion = ERROR;
            break;
        default:;
            log_info(optional_logger, "Incorrect format");
    }

	int bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    //Creo el paquete y serializo 
	void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free_package(paquete);
    free(id_correlational);
    free(id_message);
}

op_code stringToEnum(char* message_function){

    //Encuentro el string que corresponde al numero del enum
    for(int i = 0; i < 8; i++){
        if(strcmp(message_string[i].name, message_function) == 0){
            return message_string[i].operation;
        }
    }

    return ERROR;
}

void receiveMessageSubscriptor(uint32_t cod_op, uint32_t sizeofstruct, uint32_t socketfd){

    void* stream = malloc(sizeofstruct);
    uint32_t* id_message = malloc(sizeof(uint32_t)); 
    uint32_t* id_correlational = malloc(sizeof(uint32_t)); 
    recv(socketfd, stream, sizeofstruct, MSG_WAITALL);
    
    switch(cod_op){
        case NEW_POKEMON:;
            new_pokemon* newPokemonMessage = stream_to_new_pokemon(stream, id_message, false);
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "New pokemon!");
            log_info(optional_logger, "This is the pokemon: %.*s",newPokemonMessage->sizePokemon, newPokemonMessage->pokemon); 
            log_info(optional_logger, "This is the position x: %d", newPokemonMessage->position.posx);
            log_info(optional_logger, "This is the position y: %d", newPokemonMessage->position.posy);
            log_info(optional_logger, "This is the quantity: %d", newPokemonMessage->quantity);
            send_ack(socketfd, *id_message);
            break;
        case APPEARED_POKEMON:;
            appeared_pokemon* appearedPokemonMessage = stream_to_appeared_pokemon(stream, id_message, id_correlational, false);
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "Appeared pokemon!");
            log_info(optional_logger, "This is the pokemon: %.*s", appearedPokemonMessage->sizePokemon, appearedPokemonMessage->pokemon); 
            log_info(optional_logger, "This is the position x: %d", appearedPokemonMessage->position.posx);
            log_info(optional_logger, "This is the position y: %d", appearedPokemonMessage->position.posy);
            send_ack(socketfd, *id_message);
            break;
        case CATCH_POKEMON:;

            catch_pokemon* catchPokemonMessage = stream_to_catch_pokemon(stream, id_message, false);
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "Catch pokemon!");
            log_info(optional_logger, "This is the pokemon: %.*s", catchPokemonMessage->sizePokemon, catchPokemonMessage->pokemon); 
            log_info(optional_logger, "This is the position x: %d", catchPokemonMessage->position.posx);
            log_info(optional_logger, "This is the position y: %d", catchPokemonMessage->position.posy);
            send_ack(socketfd, *id_message);
            break;
        case CAUGHT_POKEMON:;

            caught_pokemon* caughtPokemonMessage = stream_to_caught_pokemon(stream, id_message, id_correlational, false);
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "Caught pokemon!");
            log_info(optional_logger, "Was the pokemon caught?: %s", caughtPokemonMessage->success ? "OK" : "FAIL");
            send_ack(socketfd, *id_message);
            break;
        case GET_POKEMON:;

            get_pokemon* getPokemonMessage = stream_to_get_pokemon(stream, id_message, false); 
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "Get pokemon!");
            log_info(optional_logger, "This is the pokemon: %s", getPokemonMessage->pokemon); 
            send_ack(socketfd, *id_message);
            break;
        case LOCALIZED_POKEMON:;

            localized_pokemon* localizedPokemonMessage = stream_to_localized_pokemon(stream, id_message, id_correlational, false);
            log_info(obligatory_logger, "Llego un nuevo mensaje de la cola %d", cod_op);
            log_info(optional_logger, "Localized pokemon!");
            log_info(optional_logger, "This is the pokemon: %.*s", localizedPokemonMessage->sizePokemon,localizedPokemonMessage->pokemon); 
            log_info(optional_logger, "This is the size of the list of positions: %d", (*localizedPokemonMessage->positions).elements_count);
            send_ack(socketfd, *id_message);
            break;
        case SUSCRIPTOR:; 

            subscribe* subscribeMessage = stream_to_subscribe(stream);
            
            log_info(optional_logger, "Subscribe!");
            log_info(optional_logger, "This is the queue: %d", subscribeMessage->colaMensajes);            
            break;
        case NEW_CONNECTION:; 

            new_connection* newConnectionMessage = stream_to_new_connection(stream);
            free(newConnectionMessage);
            log_info(optional_logger, "New connection!");
            break;
        case CONNECTION:;

            connection* connectionMessage = stream_to_connection(stream);
            id_connection = connectionMessage->id_connection;
            suscribirseA(id_queue_to_subscribe, socketfd);
            log_info(optional_logger, "Connection!"); 
            log_info(optional_logger, "This is the id connection: %d", connectionMessage->id_connection);
            log_info(optional_logger, "Subscribing to queue %d", id_queue_to_subscribe);
            break;
        case RECONNECT:;

            reconnect* reconnectMessage = stream_to_reconnect(stream);

            log_info(optional_logger, "Reconnect!");
            log_info(optional_logger, "This is the id connection: %d",reconnectMessage->id_connection);
            break; 
        case ACK:;

            ack* acknowledgementMessage = stream_to_ack(stream);

            log_info(optional_logger, "Acknowledgement!");
            log_info(optional_logger, "This is the id message: %d", acknowledgementMessage->id_message);
            break;
        case -1:;
            break;
        default:;
            log_info(optional_logger, "Cannot understand op_code received.");
    }

    free(id_message);
    free(id_correlational);
    free(stream);
}