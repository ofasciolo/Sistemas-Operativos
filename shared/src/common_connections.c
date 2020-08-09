#include "common_connections.h"
#define SIZEOP 13

char* operation_names[SIZEOP] = {
	"NEW POKEMON",
	"APPEARED POKEMON",
	"CATCH POKEMON",
	"CAUGHT POKEMON",
	"GET POKEMON",
	"LOCALIZED POKEMON",
	"SUSCRIPTOR",
	"NEW CONNECTION",
	"CONNECTION",
	"RECONNECT",
	"ACKNOWLEDGEMENT",
	"ERROR",
	"MENSAJE"
};

void start_server(char* ip, char* port, on_request request_receiver){
    uint32_t socket_servidor, isBinded=-1, sleep_time = 10;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, port, &hints, &servinfo);

    for (p=servinfo; isBinded==-1;)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            log_error(optional_logger, "Could not create socket. Trying again in %d seconds", sleep_time);
            sleep(sleep_time);
            continue;
        }

        if ((isBinded = bind(socket_servidor, p->ai_addr, p->ai_addrlen)) == -1) {
            close(socket_servidor);
            log_error(optional_logger, "Could not bind socket. Trying again in %d seconds", sleep_time);
            sleep(sleep_time);
            continue;
        }
        break;
    }
    log_info(optional_logger, "Starting new listening thread on %s:%s", ip, port);

    freeaddrinfo(servinfo);
	t_process_request* server_processor = malloc(sizeof(t_process_request));
	server_processor->socket = malloc(sizeof(uint32_t)); 
    *server_processor->socket = socket_servidor;
	server_processor->request_receiver = request_receiver;
	pthread_create(&server, NULL, (void*)run_server, server_processor);
}

void run_server(void * server_processor){
    mask_sig();
	uint32_t socket = *((t_process_request*)server_processor)->socket;
    log_info(optional_logger,"listening socket: %d", socket);
	listen(socket, SOMAXCONN);
	on_request request_receiver = ((t_process_request*)server_processor)->request_receiver;
    free(((t_process_request*)server_processor)->socket);
    free(((t_process_request*)server_processor));
	while(true){
		receive_new_connections(socket,request_receiver);
	}
}

void receive_new_connections(uint32_t socket_escucha, on_request request_receiver){
    uint32_t connfd;
    struct sockaddr_in cli; 
    uint32_t len = sizeof(cli); 
  
    connfd = accept(socket_escucha, (struct sockaddr*)&cli, &len);
    if (connfd < 0) { 
        log_info(optional_logger, "Server accept failed..."); 
    } else {
        log_info(optional_logger, "Server accepted a new client on socket: %d", connfd);
        log_info(obligatory_logger, "Se conectó un proceso en el socket %d", connfd);
        t_process_request* processor = malloc(sizeof(t_process_request));
        processor->socket = malloc(sizeof(uint32_t));
        *processor->socket = connfd;
        processor->request_receiver = request_receiver;
        pthread_create_and_detach( 
            (void*) serve_client, 
            (void*) processor);
    }
}

void serve_client(void* processor){
    mask_sig();
    uint32_t socket = *((t_process_request*) processor)->socket, size = 0, cod_op=0;
    on_request request_receiver = ((t_process_request*)processor)->request_receiver;
    free(((t_process_request*) processor)->socket);
    free(((t_process_request*) processor));
	while(1){
        if(recv(socket,(void*) &cod_op, sizeof(uint32_t), MSG_WAITALL)<=0) break;
        if( cod_op >= 1 && cod_op <= SIZEOP ){
            log_info(optional_logger, "Received %s by socket: %d", operation_names[cod_op-1], socket);
        } else {
            log_info(optional_logger, "Received %d by socket: %d", cod_op, socket);
        }
        if(recv(socket,(void*) &size, sizeof(uint32_t), MSG_WAITALL)<=0) break;
        //log_info(optional_logger, "Size of stream: %d", size);
        request_receiver(cod_op, size, socket);
    }
    log_info(optional_logger, "Socket %d has disconnected", socket);
    if (connections != NULL){ //turrada para broker, no dar bola
        pthread_mutex_lock(&m_connections);
        t_connection* conn = list_find_with_args(connections, has_socket_fd, (void*) socket);
        if (conn){
            conn->is_connected = false;
            conn->socket = 0;
        }
        pthread_mutex_unlock(&m_connections);     
    }
    close(socket);
}

t_paquete* stream_to_package(op_code code,void* payload, uint32_t size_payload){
    t_paquete* package = malloc(sizeof(t_paquete));
    package->buffer = malloc(sizeof(t_buffer));
    package->codigo_operacion = code;
    package->buffer->size = size_payload;
    package->buffer->stream = payload;
    return package;
}

void* serializar_paquete(t_paquete* paquete, uint32_t bytes){
	void * magic = malloc(bytes);
	uint32_t desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void devolver_mensaje(void* payload, uint32_t socket_cliente){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(payload);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);

	uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

uint32_t crear_conexion(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	uint32_t socketfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    while(connect(socketfd, server_info->ai_addr, server_info->ai_addrlen) == -1){
        log_info(optional_logger, "Could not connect to server on %s:%s.", ip, puerto);
        uint32_t timeConfig = config_get_int_value(config,"CONNECTION_TIME");
        log_info(optional_logger, "Trying to connect again in: %d", timeConfig);
        sleep(timeConfig);
    }

	log_info(optional_logger, "Connected successfully with %s:%s.", ip, puerto);
	log_info(obligatory_logger, "Conectado con exito con el proceso en %s:%s", ip, puerto);
	freeaddrinfo(server_info);

	return socketfd;
}


void enviar_mensaje(char* mensaje, uint32_t socketfd){
	t_buffer* buffer = (t_buffer*) malloc(sizeof(buffer));
	buffer->size = strlen(mensaje)+1;
	void* stream = malloc(buffer->size);
	uint32_t offset = 0;
	strcpy(stream + offset,mensaje);
	buffer->stream = stream;
	t_paquete* paquete = (t_paquete*) malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = buffer;
	void* a_enviar = serializar_paquete(paquete, buffer->size + sizeof(uint32_t)*2);
	send(socketfd, a_enviar, buffer->size + sizeof(uint32_t)*2, MSG_WAITALL);
}


void process_message(uint32_t client_fd, void* stream){
	void* msg;
	msg = (char*) stream;
	log_info(optional_logger, "The message received is: %s", (char*)msg);
	devolver_mensaje(msg, client_fd);
	free(msg);
}

void pthread_create_and_detach(void* function, void* args ){
	pthread_t thread;
	pthread_create(&thread, NULL, function, args);
	pthread_detach(thread);
}

void free_package(t_paquete* package){
    free(package->buffer->stream);
    free(package->buffer);
    free(package);
}

void mask_sig(void)
{
	sigset_t mask;
	sigemptyset(&mask); 
    sigaddset(&mask, SIGUSR1);         
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void* suscribirseA(op_code codigoOp,uint32_t socket_broker){
    subscribe* suscripcion = init_subscribe(codigoOp);
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = SUSCRIPTOR;
    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = subscribe_to_stream(suscripcion);
    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);
    void* a_enviar = (void *) serializar_paquete(paquete, bytes);
	send(socket_broker, a_enviar, bytes, 0);
    free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
    free_subscribe(suscripcion);
    return NULL;
}

void send_new_connection(uint32_t socket_broker){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    new_connection* newConnection = init_new_connection();

    paquete->codigo_operacion = NEW_CONNECTION;
    paquete->buffer->size = sizeof(uint32_t); 
    paquete->buffer->stream = new_connection_to_stream(newConnection);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	send(socket_broker, a_enviar, bytes, 0);

	free(a_enviar);
	free_package(paquete);
    free_new_connection(newConnection);
}

void send_reconnect(uint32_t socket_broker, uint32_t id_connection){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    reconnect* reconnectToBroker = init_reconnect(id_connection);

    paquete->codigo_operacion = RECONNECT;
    paquete->buffer->size = sizeof(u_int32_t); // revisar
    paquete->buffer->stream = reconnect_to_stream(reconnectToBroker);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	send(socket_broker, a_enviar, bytes, 0);

	free(a_enviar);
	free_package(paquete);
    free_reconnect(reconnectToBroker);
}

void send_ack(uint32_t socket_broker, uint32_t id_message){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    ack* ackBroker = init_ack(id_message);

    paquete->codigo_operacion = ACK; 
    paquete->buffer->size = sizeof(u_int32_t); // revisar
    paquete->buffer->stream = ack_to_stream(ackBroker);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	send(socket_broker, a_enviar, bytes, 0);

	free(a_enviar);
    free_package(paquete);
    free_ack(ackBroker);
    
}

uint32_t receive_connection_id(uint32_t socket_broker){
    uint32_t codop, size, id_connection;
    recv(socket_broker, &codop, sizeof(uint32_t), MSG_WAITALL);
    recv(socket_broker, &size, sizeof(uint32_t), MSG_WAITALL);
    recv(socket_broker, &id_connection, sizeof(uint32_t), MSG_WAITALL);

    return id_connection;
}

void send_appeared(appeared_pokemon* appearedPokemon, uint32_t socket, uint32_t* id_message){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    uint32_t* id_appeared = malloc(sizeof(uint32_t));
    *id_appeared = -1;

    paquete->codigo_operacion = APPEARED_POKEMON; 
    paquete->buffer->size = size_of_appeared_pokemon(appearedPokemon);
    paquete->buffer->stream = appeared_pokemon_to_stream(appearedPokemon, id_appeared, id_message);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	ssize_t bytesSent = send(socket, a_enviar, bytes, 0);
    if (bytesSent == -1){
        log_error(obligatory_logger, "No se pudo enviar el mensaje APPEARED POKEMON.");
    } else {
        log_info(obligatory_logger, "El mensaje APPEARED POKEMON se ha enviado con exito.");
    }

	free(a_enviar);
    free_package(paquete);
    free(id_appeared);
}

void send_caught(caught_pokemon* caughtPokemon, uint32_t socket, uint32_t* id_message){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    uint32_t* id_appeared = malloc(sizeof(uint32_t));
    *id_appeared = -1;

    paquete->codigo_operacion = CAUGHT_POKEMON; 
    paquete->buffer->size = size_of_caught_pokemon(caughtPokemon); // revisar
    paquete->buffer->stream = caught_pokemon_to_stream(caughtPokemon, id_appeared, id_message);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	ssize_t bytesSent = send(socket, a_enviar, bytes, 0);
    if (bytesSent == -1){
        log_error(obligatory_logger, "No se pudo enviar el mensaje CAUGHT POKEMON.");
    } else {
        log_info(obligatory_logger, "El mensaje CAUGHT POKEMON se ha enviado con exito.");
    }

	free(a_enviar);
    free_package(paquete);
    free(id_appeared);
}

void send_localized(localized_pokemon* localizedPokemon, uint32_t socket, uint32_t* id_message){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    uint32_t* id_appeared = malloc(sizeof(uint32_t));
    *id_appeared = -1;

    paquete->codigo_operacion = LOCALIZED_POKEMON; 
    paquete->buffer->size = size_of_localized_pokemon(localizedPokemon); // revisar
    paquete->buffer->stream = localized_pokemon_to_stream(localizedPokemon, id_appeared, id_message);

    uint32_t bytes = paquete->buffer->size + 2*sizeof(uint32_t);

    void* a_enviar = (void *) serializar_paquete(paquete, bytes);

	ssize_t bytesSent = send(socket, a_enviar, bytes, 0);
    if (bytesSent == -1){
        log_error(obligatory_logger, "No se pudo enviar el mensaje LOCALIZED POKEMON.");
    } else {
        log_info(obligatory_logger, "El mensaje LOCALIZED POKEMON se ha enviado con exito.");
    }

	free(a_enviar);
    free_package(paquete);
    free(id_appeared);

}