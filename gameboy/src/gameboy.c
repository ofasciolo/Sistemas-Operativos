#include "gameboy.h"

void closeAll(t_log* optional_logger,t_log* obligatory_logger, t_config* config, int connection){
    
    //Destruyo todo: logger opcional, logger obligatorio, config y conexion.
    log_destroy(optional_logger);
    log_destroy(obligatory_logger);
    config_destroy(config);
    close(connection);
}

void countTime(void* timePassed){ 
    sleep((uint32_t) timePassed);
    exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv){

    if(strcmp(argv[1],"-test")==0){
        run_tests_gameboy();
    }else{
    //Inicializando la config
    config = config_create("./cfg/gameboy.config");

    int showConsole = config_get_int_value(config,"LOG_SHOW");

    //Inicializando el logger
    obligatory_logger = log_create("./cfg/obligatory.log", "obligatory", 1, LOG_LEVEL_INFO); 
    optional_logger = log_create("./cfg/optional.log", "optional", showConsole, LOG_LEVEL_INFO);

    char* ip; 
    char* port;

    char* server = argv[1];
    //Busco el ip y el puerto
    if(strcmp(server,"BROKER") == 0 || strcmp(server, "SUSCRIPTOR") == 0){
        ip = config_get_string_value(config, "IP_BROKER");
        port = config_get_string_value(config, "PUERTO_BROKER");
    }else if(strcmp(server,"TEAM") == 0){
        ip = config_get_string_value(config, "IP_TEAM");
        port = config_get_string_value(config, "PUERTO_TEAM");
    }else if(strcmp(server,"GAMECARD") == 0){
        ip = config_get_string_value(config, "IP_GAMECARD");
        port = config_get_string_value(config, "PUERTO_GAMECARD");
    }else{
        log_error(optional_logger,"No se pudo conectar");
        exit(4);
    }

    //Creo la conexion
    int conexion = crear_conexion(ip, port);    

    //Me suscribo si el mensaje es SUSCRIPTOR
    if(strcmp(server, "SUSCRIPTOR") == 0){
        id_queue_to_subscribe = stringToEnum(argv[2]);
        log_info(obligatory_logger, "Suscrito con exito a la cola %d", id_queue_to_subscribe);
        t_process_request* process_request = malloc(sizeof(t_process_request)); 
        process_request->socket = malloc(sizeof(uint32_t));
        *process_request->socket = conexion; 
        process_request->request_receiver = receiveMessageSubscriptor;
    
        pthread_t threadConnection; //Creo un hilo asi cuenta el tiempo de conexion
        pthread_create(&threadConnection, NULL, (void*) countTime, (void*) atoi(argv[3]));

        send_new_connection(conexion); //Mando el mensaje
        id_connection = receive_connection_id(conexion);
        send_message(argv, conexion, optional_logger);
        while(1){
            serve_client(process_request);
            conexion = crear_conexion(ip, port);
            process_request = malloc(sizeof(t_process_request)); 
            process_request->socket = malloc(sizeof(uint32_t));
            *process_request->socket = conexion; 
            process_request->request_receiver = receiveMessageSubscriptor;
            send_reconnect(conexion, id_connection);
        }     
    }else{
        send_message(argv, conexion, optional_logger); //Mando el mensaje
    }

    //Cierro y elimino todo
    closeAll(optional_logger, obligatory_logger,config,conexion);
    
    return 0;
    }
}