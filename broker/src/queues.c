#include "queues.h"

void handle_new_connection(uint32_t client_fd){
    t_connection *conn = malloc(sizeof(t_connection));
    conn->socket = client_fd;
    pthread_mutex_lock(&m_id_connection);
    id_connection++;
    conn->id_connection = id_connection;
    pthread_mutex_unlock(&m_id_connection);
    conn->is_connected = true;
    pthread_mutex_lock(&m_connections);
    list_add(connections, conn);
    pthread_mutex_unlock(&m_connections);
    connection* response_conn = init_connection(conn->id_connection);
    void* stream = connection_to_stream(response_conn);
    uint32_t size = sizeof(uint32_t)*3;
    t_paquete* package = stream_to_package(CONNECTION, stream, sizeof(uint32_t));
    void* response = serializar_paquete(package, size );
    send(client_fd, response, size, 0);
    free_connection(response_conn);
    free_package(package);
    free(response);
}

bool has_connection_id(void* data, void* id){
    return ((t_connection*) data)->id_connection == (uint32_t) id;
}

bool has_queue_id(void* data, void* id){
    return data ? ((t_message_queue*) data)->id_queue == (uint32_t) id : false;
}

void handle_reconnect(uint32_t client_fd, reconnect* reconn){
    pthread_mutex_lock(&m_connections);
    t_connection* conn = (t_connection*) 
        list_find_with_args(
            connections, 
            has_connection_id, 
            (void*) reconn->id_connection
        );
    pthread_mutex_unlock(&m_connections);
    if (conn && !conn->is_connected){
        conn->socket = client_fd;
        conn->is_connected = true;
    } else {
        handle_new_connection(client_fd);
    }
    free(reconn);
}

void handle_subscribe(uint32_t client_fd, subscribe* subs){
    t_message_queue* queue = list_find_with_args(list_queues, has_queue_id, (void*) subs->colaMensajes);
    pthread_mutex_lock(&m_connections);
    t_connection* conn = list_find_with_args(connections, has_socket_fd,(void*)client_fd);
    pthread_mutex_unlock(&m_connections);
    if (queue && conn){
        log_info(obligatory_logger, "Se suscribió el proceso de socket %d y ID %d a la cola de mensajes ID %d",
            client_fd, conn->id_connection, subs->colaMensajes
        );
        pthread_mutex_lock(queue->m_subscribers_modify);
        list_add(queue->subscribers, conn);
        pthread_mutex_unlock(queue->m_subscribers_modify);
        send_all_messages(conn, queue->id_queue);
    }
    free(subs);
}

void* connection_to_receiver(void* connection){
    t_receiver* receiver = malloc(sizeof(t_receiver));
    receiver->conn = (t_connection*) connection;
    receiver->sent = false;
    receiver->received = false;
    return (void*) receiver;
}

void add_message_to_queue(void* message, uint32_t queue_id){
    t_message_queue* queue = list_find_with_args(list_queues, has_queue_id,(void*) queue_id);
    pthread_mutex_lock(queue->m_queue_modify);
    queue_push(queue->messages, message);
    pthread_mutex_unlock(queue->m_queue_modify);
    sem_post(queue->sem_message);
}

void queue_message_sender(void* args){
    t_message_queue* queue = (t_message_queue*) args;
    //log_info(optional_logger, "Starting queue number %d", queue->id_queue);
    while(1){   
        sem_wait(queue->sem_message);
        pthread_mutex_lock(memory.m_partitions_modify);
        pthread_mutex_lock(queue->m_queue_modify);
        t_data* message = queue_pop(queue->messages);
        pthread_mutex_unlock(queue->m_queue_modify);
        pthread_mutex_lock(queue->m_subscribers_modify);
        for(int i=0; i<list_size(queue->subscribers); i++){
            t_connection* conn = list_get(queue->subscribers, i);
            pthread_mutex_lock(&m_connections);
            if(conn->is_connected){
                log_info(obligatory_logger, "Se envía el mensaje ID %d al proceso con ID %d", message->id, conn->id_connection);
                void* mensaje = memory.data + message->offset;
                void* stream;
                uint32_t buffer_size;
                if(message->id_correlational){
                    buffer_size = message->size + 2*sizeof(uint32_t);
                    stream = malloc(buffer_size);
                    memcpy(stream, mensaje, message->size);
                    memcpy(stream + message->size, &message->id, sizeof(uint32_t));
                    memcpy(stream + message->size + sizeof(uint32_t), &message->id_correlational, sizeof(uint32_t));
                } else {
                    buffer_size = message->size + sizeof(uint32_t);
                    stream = malloc(buffer_size);
                    memcpy(stream, mensaje, message->size);
                    memcpy(stream + message->size, &message->id, sizeof(uint32_t));
                }
                t_paquete* package = stream_to_package(queue->id_queue, stream, buffer_size);
                void* a_enviar = serializar_paquete(package,sizeof(uint32_t)*2 + package->buffer->size);
                send(conn->socket, a_enviar, sizeof(uint32_t)*2 + package->buffer->size, 0);
                message->lastTimeUsed = timestamp();
                t_receiver* receiver = malloc(sizeof(t_receiver));
                receiver->conn = conn;
                receiver->sent = true;
                pthread_mutex_lock(message->m_receivers_modify);
                list_add(message->receivers, receiver);
                pthread_mutex_unlock(message->m_receivers_modify);
                free(stream);
                free(a_enviar);
                free(package->buffer);
                free(package);
            }
            pthread_mutex_unlock(&m_connections);
        }
        pthread_mutex_unlock(queue->m_subscribers_modify);
        pthread_mutex_unlock(memory.m_partitions_modify);
    }
}

bool has_message_id(void* message, void* id){
    return message ? ((t_data*) message)->id == (uint32_t) id : false;
}

void* queue_to_first_message(void* queue){
    return queue_peek(((t_message_queue*) queue)->messages);
}

bool hasReceived(void* receiver){
    t_receiver* rec = (t_receiver*) receiver;
    return rec ? rec->conn->is_connected && rec->received : false; 
}

bool receiver_has_socket_fd(void* receiver, void* socket){
    return receiver ? 
        ((t_receiver*) receiver)->conn->socket == (uint32_t) socket 
        : false;
}

void handle_ack(uint32_t client_fd, ack* acknowledgement){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* message = list_find_with_args(memory.partitions, has_message_id, (void*) acknowledgement->id_message);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if(message && message->state == USING){
        pthread_mutex_lock(message->m_receivers_modify);
        t_receiver* receiver = list_find_with_args(
            message->receivers, 
            receiver_has_socket_fd,
            (void*) client_fd);
        if(receiver){
            log_info(obligatory_logger, "Se confirma la recepción del mensaje ID %d por parte del proceso ID %d", 
                message->id, receiver->conn->id_connection);
            receiver->received = true;
        }
        pthread_mutex_unlock(message->m_receivers_modify);
    }
    free(acknowledgement);
}

