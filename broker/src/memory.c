#include "memory.h"

void initializeMemory(){
    memory.partitions = list_create();
    memory.configuration.memoryAlgorithm = cfg_values.algoritmo_memoria;
    memory.configuration.replaceAlgorithm = cfg_values.algoritmo_reemplazo;
    memory.configuration.freePartitionAlgorithm = cfg_values.algoritmo_particion_libre;
    memory.configuration.size = cfg_values.tamano_memoria;
    memory.configuration.minimunPartitionSize = cfg_values.tamano_minimo_particion;
    memory.configuration.countFailedSearchForCompact = cfg_values.frecuencia_compactacion;
    memory.failedSearchCount = 0;
    memory.data = malloc(memory.configuration.size);
    memory.m_partitions_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(memory.m_partitions_modify, NULL);
    memory.m_failed_search_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(memory.m_failed_search_modify, NULL );
    t_data* data = malloc(sizeof(t_data));
    data->size = memory.configuration.size;
    data->id = 0;
    data->id_correlational = 0;
    data->partition_size = data->size;
    data->offset = 0;
    data->state = FREE;

    list_add(memory.partitions, data);
}

void* mallocMemory(uint32_t idMensaje, uint32_t size){
    t_data* freePartition = seekPartitionAvailable(size);
    freePartition->id = idMensaje;
    allocateData(size, freePartition);
    return memory.data + freePartition->offset;
}
void setIdQueue(uint32_t idQueue, uint32_t idMensaje){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* partition = (t_data*)list_find_with_args(memory.partitions, partition_match_id_mensaje,(void*)idMensaje);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if(partition == NULL) return;
    partition->idQueue = idQueue;
}
void* getData(uint32_t idMensaje){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* partition = (t_data*)list_find_with_args(memory.partitions, partition_match_id_mensaje,(void*)idMensaje);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if(partition == NULL) return NULL;
    //partition->lastTimeUsed = time(NULL);
    return memory.data + partition->offset;
}
bool partition_match_id_mensaje(void* data, void* idMensaje){
    return data ? ((t_data*) data)->id == (uint32_t) idMensaje && ((t_data*) data)->state == USING : false;
}

t_data* seekPartitionAvailable(uint32_t sizeData){
    uint32_t partition_size = sizeData > memory.configuration.minimunPartitionSize ? 
                sizeData : memory.configuration.minimunPartitionSize;
    t_data* freePartition = getPartitionAvailable(partition_size);

    if(freePartition == NULL){
        if(verifMustCompact()){
            compact();
        }else{
            destroyPartition();
            if(strcmp(memory.configuration.memoryAlgorithm, "BS") == 0){
                pthread_mutex_lock(memory.m_partitions_modify);
                BS_compact();
                pthread_mutex_unlock(memory.m_partitions_modify);
            }
        }
        return seekPartitionAvailable(partition_size);
    }else{
        return freePartition;
    }
}

t_data* getPartitionAvailable(uint32_t sizeData){
    if(strcmp(memory.configuration.freePartitionAlgorithm, "FF") == 0){
        return FF_getPartitionAvailable(sizeData);
    }else{
        return BF_getPartitionAvailable(sizeData);
    }
}

bool verifMustCompact(){
    if(memory.configuration.countFailedSearchForCompact == -1){
        pthread_mutex_lock(memory.m_partitions_modify);
        bool response = list_all_satisfy(memory.partitions, partition_is_free);
        pthread_mutex_unlock(memory.m_partitions_modify);
        return response;
    }
    pthread_mutex_lock(memory.m_failed_search_modify);
    bool response = memory.failedSearchCount == memory.configuration.countFailedSearchForCompact;
    pthread_mutex_unlock(memory.m_failed_search_modify);
    return response;
}

bool partition_is_free(void* data) {
    t_data* partition = (t_data*)data;
    return partition->state == FREE;
}

void compact(){
    memory.failedSearchCount = 0;
    if(strcmp(memory.configuration.memoryAlgorithm, "BS") == 0){
       // BS_compact();
    }else{
        DP_compact();
    }
}

void destroyPartition(){
    pthread_mutex_lock(memory.m_failed_search_modify);
    memory.failedSearchCount++;
    pthread_mutex_unlock(memory.m_failed_search_modify);
    if(strcmp(memory.configuration.replaceAlgorithm, "FIFO") == 0){
        FIFO_destroyPartition();
    }else{
        LRU_destroyPartition();
    }
}

void allocateData(uint32_t sizeData, t_data* freePartition){
    uint32_t partition_size = sizeData > memory.configuration.minimunPartitionSize ? 
                sizeData : memory.configuration.minimunPartitionSize;
    if(strcmp(memory.configuration.memoryAlgorithm, "BS") == 0){
        BS_allocateData(partition_size, freePartition);
    }else{
        DP_allocateData(partition_size, freePartition);
    }
}


//region memory algorithms
t_data* FF_getPartitionAvailable(uint32_t sizeData){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* response = (t_data*)list_find_with_args(memory.partitions, partition_size_validation,(void*)sizeData);
    pthread_mutex_unlock(memory.m_partitions_modify);
    return response;
}
t_data* BF_getPartitionAvailable(uint32_t sizeData){
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t minimunSize = sizeData;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == FREE && dataAux->partition_size < minimunSize){
            minimunSize = dataAux->partition_size;
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
    return FF_getPartitionAvailable(minimunSize);
}

bool partition_size_validation(void* data, void* sizeData){
    return data ? ((t_data*) data)->partition_size >= (uint32_t) sizeData && ((t_data*) data)->state == FREE : false;
}

void BS_compact(){
    //Debería unir solo cuando son particiones del "mismo bloque"
    uint32_t sizeList =  list_size(memory.partitions);
    t_data* previousPartition = NULL;
    t_data* dataAux;
    bool mustFinish = false;
    for(int i = 0; i < sizeList && !mustFinish; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(previousPartition == NULL){
            previousPartition = dataAux;
        }else{
            if(previousPartition->partition_size == dataAux->partition_size
            && previousPartition->offset == (dataAux->offset^previousPartition->partition_size)
            && dataAux->offset == (previousPartition->offset^dataAux->partition_size)
            ){
                if(previousPartition->state == FREE && dataAux->state == FREE){
                    //I join them
                    log_info(optional_logger, "Se asocian las particiones en las posiciones %d y %d bajo Buddy System.", 
                        previousPartition->offset, dataAux->offset);
                    previousPartition->partition_size += dataAux->partition_size;
                    list_remove(memory.partitions, i);
                    free(dataAux);
                    mustFinish = true;
                    break;
                }else{
                    previousPartition = NULL;
                }
            }else{
                previousPartition = dataAux;
            }
        }
    }
    if(mustFinish) BS_compact();
}

bool sortByState(void* elem1, void* elem2){
    t_data* data1 = elem1, *data2 = elem2;
    if (data1->state == FREE && data2->state == USING){
        void* temp = malloc(data2->partition_size);
        memcpy(temp, memory.data + data2->offset, data2->partition_size);
        memcpy(memory.data + data1->offset, temp, data2->partition_size);
        data2->offset = data1->offset;
        data1->offset = data2->offset + data2->partition_size;
        free(temp);
        return false;
    } else {
        return true;
    }
}



void DP_compact(){
    //It moves the partitions with state using to the init an join the free partitions
    pthread_mutex_lock(memory.m_partitions_modify);
    log_info(obligatory_logger, "Se procede a compactar la partición dinámica.");
    if (list_size(memory.partitions) == 1) return;
    list_sort(memory.partitions, sortByState);
    for (int i = list_size(memory.partitions) - 2 ; i >= 0 ; i--){
        t_data* elem1 = list_get(memory.partitions, i);
        t_data* elem2 = list_get(memory.partitions, i + 1 );
        if (elem1 && elem2 && elem1->state == FREE && elem2->state == FREE){
            elem1->partition_size += elem2->partition_size;
            list_remove(memory.partitions, i + 1);
            free(elem2);
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}


void FIFO_destroyPartition(){
    uint32_t minimumId = 4294967295;
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t indexFound = 0;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == USING && dataAux->id < minimumId){
            minimumId = dataAux->id;
            indexFound = i;
        }
    }
    t_data* partitionSelected = (t_data*)list_get(memory.partitions, indexFound);
    if (partitionSelected && partitionSelected->state == USING){
        log_info(obligatory_logger, "Se destruye la particion de memoria en la posicion: %d", partitionSelected->offset);
        partitionSelected->state = FREE;
        pthread_mutex_lock(partitionSelected->m_receivers_modify);
        list_destroy_and_destroy_elements(partitionSelected->receivers, free);
        pthread_mutex_unlock(partitionSelected->m_receivers_modify);
        pthread_mutex_destroy(partitionSelected->m_receivers_modify);
        free(partitionSelected->m_receivers_modify);
        condense(indexFound);
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

void condense(int indexFound){
    t_data* partitionSelected = list_get(memory.partitions, indexFound );
    if (indexFound + 1 <= list_size(memory.partitions) - 1){
        t_data* nextPartition = list_get(memory.partitions, indexFound + 1 );
        if (nextPartition->state == FREE){
            list_remove(memory.partitions, indexFound + 1);
            partitionSelected->partition_size += nextPartition->partition_size;
            partitionSelected->creationTime = timestamp();
            free(nextPartition);
        }
    }
    if (indexFound - 1 >= 0){
        t_data* previousPartition = list_get(memory.partitions, indexFound -1 );
        if (previousPartition->state == FREE){
            list_remove(memory.partitions, indexFound - 1);
            partitionSelected->partition_size += previousPartition->partition_size; 
            partitionSelected->size = partitionSelected->partition_size; 
            partitionSelected->offset = previousPartition->offset;
            partitionSelected->creationTime = timestamp();
            free(previousPartition);
        }
    }
}

void LRU_destroyPartition(){
    uint64_t oldestTime = timestamp();
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t indexFinded = 0;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == USING && dataAux->lastTimeUsed < oldestTime){
            oldestTime = dataAux->lastTimeUsed;
            indexFinded = i;
        }
    }
    t_data* partitionSelected = (t_data*)list_get(memory.partitions, indexFinded);
    if (partitionSelected && partitionSelected->state == USING){
        log_info(obligatory_logger, "Se destruye la particion de memoria en la posicion: %d", partitionSelected->offset);
        partitionSelected->state = FREE;
        pthread_mutex_lock(partitionSelected->m_receivers_modify);
        list_destroy_and_destroy_elements(partitionSelected->receivers, free);
        pthread_mutex_unlock(partitionSelected->m_receivers_modify);
        pthread_mutex_destroy(partitionSelected->m_receivers_modify);
        free(partitionSelected->m_receivers_modify);
        condense(indexFinded);
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

bool _offsetAscending(void* data1, void*data2) {
        return ((t_data*) data1)->offset < ((t_data*) data2)->offset;
}

void BS_allocateData(uint32_t sizeData, t_data* freePartitionData){
    if(sizeData <= freePartitionData->partition_size / 2){
        t_data* newData = malloc(sizeof(t_data));
        freePartitionData->partition_size = freePartitionData->partition_size / 2;
        newData->partition_size = freePartitionData->partition_size;
        newData->offset = freePartitionData->offset + freePartitionData->partition_size;
        newData->state = FREE;
        newData->id = 0;
        newData->id_correlational = 0;
        pthread_mutex_lock(memory.m_partitions_modify);
        list_add(memory.partitions, newData);
        pthread_mutex_unlock(memory.m_partitions_modify);
        BS_allocateData(sizeData, freePartitionData);
    }else{
        freePartitionData->creationTime = timestamp();
        freePartitionData->lastTimeUsed = freePartitionData->creationTime;
        freePartitionData->state = USING;
        freePartitionData->id = 0;
        freePartitionData->id_correlational = 0;
        pthread_mutex_lock(memory.m_partitions_modify);
        list_sort(memory.partitions, _offsetAscending);
        pthread_mutex_unlock(memory.m_partitions_modify);
    }
}


void DP_allocateData(uint32_t sizeData, t_data* freePartitionData){   
    if(sizeData != freePartitionData->size){
        //If the size of the data is bigger than the free space, its create a new partition
        t_data* newData = malloc(sizeof(t_data));
        newData->partition_size = freePartitionData->partition_size - sizeData;
        newData->size = newData->partition_size;
        newData->offset = freePartitionData->offset + sizeData;
        newData->state = FREE;
        newData->id = 0;
        newData->id_correlational = 0;
        pthread_mutex_lock(memory.m_partitions_modify);
        list_add(memory.partitions, newData);
        list_sort(memory.partitions, _offsetAscending);
        pthread_mutex_unlock(memory.m_partitions_modify);
    }
    freePartitionData->partition_size = sizeData;
    freePartitionData->size = sizeData;
    freePartitionData->id = 0;
    freePartitionData->id_correlational = 0;
    freePartitionData->creationTime = timestamp();
    freePartitionData->lastTimeUsed = freePartitionData->creationTime;
    freePartitionData->state = USING;
}

void dumpMemory(){
    log_info(optional_logger, "Dumping cache into %s", cfg_values.dump_file);
    log_info(obligatory_logger, "Se solicitó el Dump de la Caché.");
    FILE* file = txt_open_for_append(cfg_values.dump_file);

    txt_write_in_file(file, "------------------------------------------------------------------------------\n");
    dump_write_time(file);
    dump_partitions(file);
    txt_write_in_file(file, "------------------------------------------------------------------------------\n");

    txt_close_file(file);
    log_info(optional_logger, "Finished dumping.", cfg_values.dump_file);
}
void dump_write_time(FILE* file){
    time_t timer;
    char* timeFormated = malloc(26);
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(timeFormated, 26, "%d/%m/%Y %H:%M:%S", tm_info);
    char* prefix = "Dump: ";
    char* text = malloc(26 + strlen(prefix) + 1);
    strcpy(text, prefix);
    strcat(text, timeFormated);
    txt_write_in_file(file, text);
    txt_write_in_file(file, "\n");
}
void dump_partitions(FILE* file){
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList = list_size(memory.partitions);
    char* strFormat_using = "Partición %d: %p - %p. [X] Size: %db LRU: %lld Cola: %d ID:%d\n";
    char* str_using = malloc(strlen(strFormat_using) + sizeof(void*)*2 + sizeof(int) * 4 + sizeof(long));
    char* strFormat_free = "Partición %d: %p - %p. [L] Size: %db\n";
    char* str_free = malloc(strlen(strFormat_free) + sizeof(void*)*2 + sizeof(int) * 2);
    void* initialPointer;
    void* endPointer;
    for(int i = 0; i < sizeList; i++){
        t_data* partition = list_get(memory.partitions, i);
        initialPointer = memory.data + partition->offset;
        endPointer = initialPointer + partition->partition_size;//TODO mis dudas
        if(partition->state == FREE){
            sprintf(str_free, strFormat_free, i, initialPointer, endPointer, partition->partition_size);
            txt_write_in_file(file, str_free);
        }else{
            sprintf(str_using, strFormat_using, i, initialPointer, endPointer, partition->partition_size, partition->lastTimeUsed, partition->idQueue, partition->id);
            txt_write_in_file(file, str_using);
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

bool isRepeated(uint32_t id_corr){
    bool matchesIdCorrelational(void* elem){
        return ((t_data*) elem)->id_correlational == id_corr && id_corr != 0;
    }
    pthread_mutex_lock(memory.m_partitions_modify);
    bool res = list_count_satisfying(memory.partitions, matchesIdCorrelational) > 1;
    pthread_mutex_unlock(memory.m_partitions_modify);
    return res;
}

t_data* assign_and_return_message(uint32_t id_queue, uint32_t sizeofrawstream, void* stream){
    uint32_t sizeofdata;
    t_data* freePartition;
    switch(id_queue){
        case NEW_POKEMON:
        case CATCH_POKEMON:
        case GET_POKEMON:
            sizeofdata = sizeofrawstream - sizeof(uint32_t);
            break;
        case APPEARED_POKEMON:
        case CAUGHT_POKEMON:
        case LOCALIZED_POKEMON:
            sizeofdata = sizeofrawstream - 2 * sizeof(uint32_t);
            break;
        default:
            log_info(optional_logger, "El id queue no es correcto y entró a la asignación de memoria. Algo salió horriblemente mal.");
            pthread_mutex_unlock(&m_new_partition);
            return NULL;
    }
    if(sizeofdata > memory.configuration.size){
        log_info(optional_logger, 
            "El tamano del mensaje (%db) es mayor a la capacidad de la memoria(%db). Se ignorara el mensaje.",
            sizeofdata, memory.configuration.size
        );
        return (void*) 1;
    }
    freePartition = seekPartitionAvailable(sizeofdata);
    allocateData(sizeofdata, freePartition);
    switch(id_queue){
        case APPEARED_POKEMON:
        case CAUGHT_POKEMON:
        case LOCALIZED_POKEMON:
            memcpy(&freePartition->id_correlational, stream + sizeofdata + sizeof(uint32_t), sizeof(uint32_t));
        default:;
    }
    log_debug(optional_logger, "Creating new partition at position: %d", freePartition->offset);
    void* data = memory.data + freePartition->offset;
    pthread_mutex_lock(memory.m_partitions_modify);
    memcpy(data, stream, sizeofdata);
    pthread_mutex_lock(&m_id_message);
    id_message++;
    freePartition->id = id_message;
    pthread_mutex_unlock(&m_id_message);
    log_info(obligatory_logger, "Se almacena el mensaje ID %d en memoria en la posicion: %d", freePartition->id, freePartition->offset);
    freePartition->size = sizeofdata;
    freePartition->idQueue = id_queue;
    freePartition->receivers = list_create();
    freePartition->m_receivers_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(freePartition->m_receivers_modify,NULL);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if (isRepeated(freePartition->id_correlational)){
        log_info(optional_logger, 
            "No se va a agregar a la cola de mensajes %d el mensaje de la posicion %d porque el id_correlativo %d ya se encontraba en memoria.",
            id_queue, freePartition->offset, freePartition->id_correlational
        );
        return (void*)1;
    } else {
        return freePartition;
    }
} 

void send_all_messages(t_connection* conn, uint32_t id_queue){
    pthread_mutex_lock(memory.m_partitions_modify);
    bool isFromQueue(void* elem){ //intellisense no lo reconoce pero compila
        return elem && ((t_data*) elem)->state == USING && ((t_data*) elem)->idQueue == id_queue;
    }
    t_list* queueMessages = list_filter(memory.partitions, isFromQueue);
    void sendMessage(void* data){
        t_data* message = data;
        bool hasReceiver(void* receiver){
            return ((t_receiver*) receiver)->conn->id_connection == conn->id_connection && ((t_receiver*) receiver)->conn->is_connected;
        }
        pthread_mutex_lock(message->m_receivers_modify);
        t_receiver* rec = list_find(message->receivers, hasReceiver);
        pthread_mutex_unlock(message->m_receivers_modify);
        if(rec == NULL){
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
            t_paquete* package = stream_to_package(id_queue, stream, buffer_size);
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
            free(package->buffer);
            free(package);
            free(a_enviar);
        }
    }
    list_iterate(queueMessages, sendMessage);
    list_destroy(queueMessages);
    pthread_mutex_unlock(memory.m_partitions_modify);
}

//end region