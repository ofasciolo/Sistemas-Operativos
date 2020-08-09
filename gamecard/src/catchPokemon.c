#include "catchPokemon.h"

//void catchPokemon(){
    //Verificar si esta el pokemon en el directorio Pokemon. Si no existe informar un ERROR
    //Verificar si no hay otro proceso que lo esté abriendo. En caso de que si lo haya finalizar hilo y reintentar en "TIEMPO_DE_REINTENTO_OPERACION"
    //Verificar si las posiciones ya existen dentro del archivo. En caso de no existir se debe informar un ERROR.
    //En caso que la cantidad del Pokémon sea “1”, se debe eliminar la línea. En caso contrario se debe decrementar la cantidad en uno.
    //Cerrar el archivo.
    //Todo resultado deberá informarse enviando un mensaje a la Cola de Mensajes "CAUGHT_POKEMON" indicando: ID del mensaje recibido y el Resultado (Fail o success).
    /**En caso de que no se pueda establecer conexion con el broker notificarlo por Logs y continuar**/
//}

void catchPokemonTallGrass(threadPokemonMessage* threadCatchPokemonMessage){
    catch_pokemon* catchPokemon = threadCatchPokemonMessage->pokemon;

    char buffer[100];   
    char* stream = malloc(catchPokemon->sizePokemon + 1);
    memcpy(stream, catchPokemon->pokemon, catchPokemon->sizePokemon); 
    stream[catchPokemon->sizePokemon] = '\0';

    strcpy(buffer, "");
    strcat(buffer, filesPath);
    strcat(buffer, "/");
    strcat(buffer, stream);

    char* directory = malloc(strlen(filesPath) + catchPokemon->sizePokemon + 3);
    memcpy(directory, buffer, strlen(filesPath) + catchPokemon->sizePokemon + 2);
    memcpy(directory + strlen(filesPath) + catchPokemon->sizePokemon + 2, "\0", sizeof(char));

    char* metadata = "/Metadata.bin";
    char* directorioMetadata = malloc(strlen(directory) + strlen(metadata) + 1);
    strcpy(directorioMetadata,"");
    strcat(directorioMetadata,directory);
    strcat(directorioMetadata, metadata);

    caught_pokemon* caughtPokemon = malloc(sizeof(caughtPokemon));

    if(access( directorioMetadata, F_OK ) != -1){
        FILE* file = fopen(directorioMetadata, "rb");
        flock(fileno(file), LOCK_EX);
    
        fseek(file, 0, SEEK_END);
        int sizeFile = ftell(file);
        fseek(file, 0, SEEK_SET);
    
        flock(fileno(file), LOCK_UN);
        fclose(file);

        if(sizeFile != 0){
            intentarAbrirMetadata(directorioMetadata, stream);
            sacarDatosYOrdenarBloques(directorioMetadata, catchPokemon);
            cerrarMetadata(directorioMetadata, stream); 
            caughtPokemon->success = 1;
        }else{
            log_error(obligatory_logger, "No existen posiciones para ese pokemon");
            caughtPokemon = 0;
            //hacer fail
        }
    }else{
        caughtPokemon = 0;
        log_error(obligatory_logger, "No existe el pokemon.");
    }

    if(threadCatchPokemonMessage->client_fd != 0){
        send_caught(caughtPokemon, threadCatchPokemonMessage->client_fd, threadCatchPokemonMessage->id_mensaje);
    }

    free(caughtPokemon);
    free(stream);
    free(directory);
    free(directorioMetadata);
    free(threadCatchPokemonMessage->pokemon);
    free(catchPokemon->pokemon);
    free(threadCatchPokemonMessage->id_mensaje);
    free(threadCatchPokemonMessage);
}

void sacarDatosYOrdenarBloques(char* metadata, catch_pokemon* catchPokemon){

    char* stream = malloc(catchPokemon->sizePokemon + 1);
    memcpy(stream, catchPokemon->pokemon, catchPokemon->sizePokemon); 
    stream[catchPokemon->sizePokemon] = '\0';

    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);

    char** bloques = config_get_array_value(configMetadataTallGrass,"BLOCKS");
    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);

    char* posX = malloc(10);
    strcpy(posX,"");
    sprintf(posX,"%d",catchPokemon->position.posx);

    char* posY = malloc(10);
    strcpy(posY,"");
    sprintf(posY,"%d",catchPokemon->position.posy);

    char* quantity = malloc(10);
    strcpy(quantity,"");
    sprintf(quantity,"%d",1);

    t_list* lista = levantarBloquesAMemoria(bloques, cantidadBloques);

    positionQuantity* posicionPokemonSacar = malloc(sizeof(positionQuantity));
    posicionPokemonSacar->posicionX = atoi(posX); 
    posicionPokemonSacar->posicionY = atoi(posY);
    posicionPokemonSacar->cantidad = atoi(quantity);

    positionQuantity* posicionEncontrada = list_find_with_args(lista, coincidePosicion, (void*)posicionPokemonSacar);

    if(posicionEncontrada != NULL){
        posicionEncontrada->cantidad = posicionEncontrada->cantidad - posicionPokemonSacar->cantidad;
        if(posicionEncontrada->cantidad == 0){
            for(int i = 0; i < lista->elements_count; i++){
                positionQuantity* posicionEncontradaSacar = list_get(lista,i); 
                if(coincidePosicion(posicionEncontradaSacar, posicionEncontrada)){
                    void* elem = list_remove(lista, i);
                    free(elem);
                }
            }  
        }
        
    }else{
        log_info(optional_logger, "No estaba esa posicion");
    }

    char* sizeMetadata = bajarBloquesADisco(lista, bloques, cantidadBloques, stream, catchPokemon->position.posx, catchPokemon->position.posy, 1, metadata);

    int cantidadBloquesUpdated = ceil((float) atoi(sizeMetadata)/configM.blockSize);

    if(cantidadBloquesUpdated < cantidadBloques){
        removeLastBlock(metadata, catchPokemon, posicionPokemonSacar);
    }

    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataUpdated = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);
    config_set_value(configMetadataUpdated, "SIZE", sizeMetadata);

    pthread_mutex_lock(&metadata_create);
    config_save(configMetadataUpdated); 
    pthread_mutex_unlock(&metadata_create);

    list_destroy_and_destroy_elements(lista,free);

    free(posicionPokemonSacar);
    free(posY);
    free(posX);
    free(quantity);
    free(sizeMetadata);
    config_destroy(configMetadataUpdated);
    config_destroy(configMetadataTallGrass);
    //for(int i = 0; i<cantidadBloques; i++){
        //free(bloques[i]);
    //}
    //free(bloques);
    free(stream);
}

void removeLastBlock(char* metadata, catch_pokemon* catchPokemon, positionQuantity* posicionPokemonSacar){
    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);

    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);
    char** bloques = config_get_array_value(configMetadataTallGrass,"BLOCKS");

    char* lineaBorrada = structALinea(posicionPokemonSacar);
    int sizeBloqueBorrado = strlen(lineaBorrada);

    size -= sizeBloqueBorrado; 

    if(size == 1){ //Porque el EOF es 1 byte
        size--;
    }

    if(cantidadBloques > 0){
        uint32_t bloque = atoi(bloques[cantidadBloques - 1]);
        bool testBit = bitarray_test_bit(bitmap, bloque - 1);
        if(testBit == 1){
            bitarray_clean_bit(bitmap, bloque - 1);
        }
    }

    int cantidadBloquesUpdated = ceil((float)size / configM.blockSize);

    char* bloquesConfig = malloc(sizeof(char)*3*(cantidadBloquesUpdated + 1) + 1);
    strcpy(bloquesConfig,"");
    strcat(bloquesConfig,"[");
    for(int i = 0; i < cantidadBloquesUpdated; i++){ 
        strcat(bloquesConfig,bloques[i]);
        if(i < cantidadBloquesUpdated - 1){
            strcat(bloquesConfig,",");
        }
    }
    strcat(bloquesConfig,"]");
    
    
    //[[29][31]] bloques[1]
   // imprimirBITARRAY(bitmap);
    char* sizeChar = malloc(sizeof(uint32_t));
    strcpy(sizeChar, "");
    sprintf(sizeChar, "%d", size);

    config_set_value(configMetadataTallGrass, "SIZE", sizeChar);
    config_set_value(configMetadataTallGrass,"BLOCKS", bloquesConfig);

    pthread_mutex_lock(&metadata_create);
    config_save(configMetadataTallGrass);
    pthread_mutex_unlock(&metadata_create);
    log_info(obligatory_logger, "Se ha liberado un bloque de un pokemon.");
    if(size == 0){
        log_info(obligatory_logger, "Un pokemon fue eliminado logicamente.");
    }

    config_destroy(configMetadataTallGrass);

    free(bloquesConfig);
    for(int i = 0; i<cantidadBloques -1 ; i++){
        free(bloques[i]);
    }
    free(bloques);
    free(sizeChar);
    free(lineaBorrada);
}