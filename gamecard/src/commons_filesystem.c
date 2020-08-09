#include "commons_filesystem.h"

char* crearBloque(new_pokemon* newPokemon){
    char* extension = ".bin";
    pthread_mutex_lock(&mutexBitmap);
    size_t sizeBitmap = bitarray_get_max_bit(bitmap);
    int bin = 1;  
    while(bin <= (int)sizeBitmap + 1){
        
        bool testBit = bitarray_test_bit(bitmap, bin - 1);
        if(testBit == 0){
            char* binChar = malloc(10);
            
            strcpy(binChar,"");
            sprintf(binChar, "%d", bin);
            char* directorioBloques = malloc(strlen(blocksPath) + strlen(binChar) + strlen(extension) + 2); //ver de cambiarlo 
            strcpy(directorioBloques,"");
            strcat(directorioBloques,blocksPath);
            strcat(directorioBloques, "/");
            strcat(directorioBloques,binChar);
            strcat(directorioBloques,extension);
            
            FILE* binary = fopen(directorioBloques,"wb"); 

            char* posX = malloc(10);
            strcpy(posX,"");
            sprintf(posX,"%d",newPokemon->position.posx);

            char* posY = malloc(10);
            strcpy(posY,"");
            sprintf(posY,"%d",newPokemon->position.posy);

            char* quantity = malloc(10);
            strcpy(quantity,"");
            sprintf(quantity,"%d",newPokemon->quantity); 

            char* writeBinary = malloc(strlen(posX) + strlen("-") + strlen(posY) + strlen("=") + strlen("\n") + 5); 
            strcpy(writeBinary,"");
            strcat(writeBinary,posX);
            strcat(writeBinary,"-");
            strcat(writeBinary,posY);
            strcat(writeBinary, "=");
            strcat(writeBinary,quantity);
            strcat(writeBinary, "\n");

            fwrite(writeBinary, strlen(writeBinary) + 1, 1, binary);

            fclose(binary);
        
            bitarray_set_bit(bitmap, bin-1);
            
            free(posX);
            free(posY);
            free(quantity);
            free(directorioBloques);
            free(writeBinary);
            //imprimirBITARRAY(bitmap);
            pthread_mutex_unlock(&mutexBitmap);
            return binChar;
        }else{
            bin++;
        }
    }
    pthread_mutex_unlock(&mutexBitmap);
    return NULL;
}

int metadataBlocks(char* metadata, char* pokemon){
    
    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    

    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);

    config_destroy(configMetadataTallGrass);
    pthread_mutex_unlock(&metadata_create);
    return cantidadBloques;
}

t_list* levantarBloquesAMemoria(char** bloques, int cantidadBloques){
    char* extension = ".bin";
    t_list* listaPosiciones = list_create();
    int caracterActual = 0; 
    char buffer[10];
    memset(buffer,'\0',10);
    positionQuantity* lineaBloque = malloc(sizeof(positionQuantity));
    bool posx = true;
    bool posy = false;
    bool cant = false;
        
    for(int i = 0; i<cantidadBloques; i++){
        char* direccionBinario = malloc(strlen(blocksPath) + strlen(bloques[i]) + strlen(extension) + 2);

        strcpy(direccionBinario,"");
        strcat(direccionBinario,blocksPath);
        strcat(direccionBinario, "/");
        strcat(direccionBinario,bloques[i]);
        strcat(direccionBinario,extension);

        FILE* fileBloque = fopen(direccionBinario, "rb");

        int c; 
        
        while((c=fgetc(fileBloque)) != EOF){
            char position = (char) c; 
            if(posx){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++;
                }else{
                    lineaBloque->posicionX = atoi(buffer);
                    posx = false;
                    posy = true;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    continue;
                }     
            }else if(posy){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++; 
                }else{
                    lineaBloque->posicionY = atoi(buffer);
                    posy = false;
                    cant = true;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    continue;
                } 
            }else if(cant){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++; 
                }else{
                    lineaBloque->cantidad = atoi(buffer);
                    posx = true;
                    cant = false;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    list_add(listaPosiciones, lineaBloque);
                    lineaBloque = malloc(sizeof(positionQuantity));
                    continue;
                } 
            }
        }
    
        fclose(fileBloque);
        free(direccionBinario);
    }
    
    free(lineaBloque);

    return listaPosiciones;
}

bool coincidePosicion(void* elem, void* args){
    positionQuantity* posLista = (positionQuantity*) elem; 
    positionQuantity* posNewPokemon = (positionQuantity*) args;

    if(posLista->posicionX == posNewPokemon->posicionX &&
        posLista->posicionY == posNewPokemon->posicionY){
            return true;
    }
    return false; 
}

char* bajarBloquesADisco(t_list* lista, char** bloques, int cantidadBloques, char* nombrePokemon, uint32_t posx, uint32_t posy, uint32_t quantity, char* metadata){
    t_list* writeListBinary = list_map(lista, structALinea);
    char* listaConcatenada = concatenarStrings(writeListBinary);
    uint32_t sizeTotal = strlen(listaConcatenada);  
    int j=0;
    char* extension = ".bin";

    for(int i = 0; i<cantidadBloques; i++){
        char* direccionBinario = malloc(strlen(blocksPath) + strlen(bloques[i]) + strlen(extension) + 2);

        strcpy(direccionBinario,"");
        strcat(direccionBinario,blocksPath);
        strcat(direccionBinario, "/");
        strcat(direccionBinario,bloques[i]);
        strcat(direccionBinario,extension);

        FILE* fileBloque = fopen(direccionBinario, "wb");

        int sizeArchivo = 0; 

        while(sizeArchivo < configM.blockSize){
            if(strlen(listaConcatenada) < j){
                break;
            }
            fputc(listaConcatenada[j],fileBloque);
            j++;
            sizeArchivo++;
        }

        fclose(fileBloque);

        if(sizeTotal > sizeArchivo && sizeArchivo >= configM.blockSize && bloques[i] == bloques[cantidadBloques - 1]){
            new_pokemon* newPokemon = malloc(sizeof(new_pokemon));
            newPokemon->sizePokemon = strlen(nombrePokemon) + 2; 
            newPokemon->pokemon = nombrePokemon;
            newPokemon->position.posx = posx;
            newPokemon->position.posy = posy; 
            newPokemon->quantity = quantity;
            //sizeTotal++;
            char* block = crearBloque(newPokemon);
            addBlockMetadata(metadata, block, newPokemon);
            free(block);

            cantidadBloques++;
            pthread_mutex_lock(&metadata_create);
            t_config* configMetadataTallGrass = config_create(metadata);
            pthread_mutex_unlock(&metadata_create);
            for(int i = 0; i<cantidadBloques; i++){
                free(bloques[i]);
            }
            free(bloques);
            bloques = config_get_array_value(configMetadataTallGrass, "BLOCKS");
            free(newPokemon);
            config_destroy(configMetadataTallGrass);
        }

        if(sizeArchivo == 1 && bloques[i] == bloques[cantidadBloques - 1]){
            //sizeTotal--;
            cantidadBloques--;
            pthread_mutex_lock(&metadata_create);
            t_config* configMetadataTallGrass = config_create(metadata);
            pthread_mutex_unlock(&metadata_create);
            for(int i = 0; i<cantidadBloques; i++){
                free(bloques[i]);
            }
            free(bloques);
            bloques = config_get_array_value(configMetadataTallGrass, "BLOCKS");
            config_destroy(configMetadataTallGrass);
        }
        
        free(direccionBinario);
    }

    list_destroy_and_destroy_elements(writeListBinary, free);
    free(listaConcatenada);


    char* sizeTotalChar = malloc(20);
    strcpy(sizeTotalChar, "");
    sprintf(sizeTotalChar, "%d", sizeTotal);
    for(int i = 0; i<cantidadBloques; i++){
        free(bloques[i]);
    }
    free(bloques);
    return sizeTotalChar; 
}

void* structALinea(void* posicion){
    positionQuantity* lineaStruct = (positionQuantity*)posicion;

    char* posX = malloc(10);
    strcpy(posX,"");
    sprintf(posX,"%d",lineaStruct->posicionX);

    char* posY = malloc(10);
    strcpy(posY,"");
    sprintf(posY,"%d",lineaStruct->posicionY);

    char* quantity = malloc(10);
    strcpy(quantity,"");
    sprintf(quantity,"%d",lineaStruct->cantidad); 

    char* writeBinary = malloc(strlen(posX) + strlen("-") + strlen(posY) + strlen("=") + strlen(quantity) + 3); 
    strcpy(writeBinary,"");
    strcat(writeBinary,posX);
    strcat(writeBinary,"-");
    strcat(writeBinary,posY);
    strcat(writeBinary, "=");
    strcat(writeBinary,quantity);
    strcat(writeBinary, "\n");

    free(posX);
    free(posY);
    free(quantity);

    return (void*)writeBinary;
}

char* concatenarStrings(t_list* lista){
    uint32_t tamano = 0;
    for(int i = 0; i<list_size(lista); i++){
        tamano += strlen(list_get(lista, i));
    }
    tamano++;
    char* concatenacion = malloc(tamano);
    strcpy(concatenacion,"");

    for(int i = 0; i<list_size(lista); i++){
        strcat(concatenacion, (char*)list_get(lista,i));
    }

    return concatenacion;
}

void bloquearMetadata(char* pokemon){
    pthread_mutex_lock(&mutexListOfMutex);
    mutexDirectory* mutexDirectorioPokemon = list_find_with_args(mutexListDirectory, esPokemon, pokemon);    
    pthread_mutex_unlock(&mutexListOfMutex);

    pthread_mutex_lock(&mutexDirectorioPokemon->mutex);
}

void desbloquearMetadata(char* pokemon){
    pthread_mutex_lock(&mutexListOfMutex);
    mutexDirectory* mutexDirectorioPokemon = list_find_with_args(mutexListDirectory, esPokemon, pokemon);    
    pthread_mutex_unlock(&mutexListOfMutex);

    pthread_mutex_unlock(&mutexDirectorioPokemon->mutex);
}

bool esPokemon(void* elem, void* args){
    mutexDirectory* directorio = (mutexDirectory*) elem; 
    char* pokemon = (char*) args; 

    if(strcmp(directorio->nombreDirectorio,pokemon) == 0){
        return true;
    }
    return false;
}

void abrirMetadata(char* metadata, char* pokemon){

    bloquearMetadata(pokemon);

    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);
    
    config_set_value(configMetadataTallGrass, "OPEN", "Y");

    pthread_mutex_lock(&metadata_create);
    config_save(configMetadataTallGrass);
    pthread_mutex_unlock(&metadata_create);

    
    //desbloquearMetadata(pokemon);
    config_destroy(configMetadataTallGrass);
}

void cerrarMetadata(char* metadata, char* pokemon){

    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);

    int time = config_get_int_value(config, "TIEMPO_RETARDO_OPERACION");
    sleep(time);

    config_set_value(configMetadataTallGrass, "OPEN", "N");

    pthread_mutex_lock(&metadata_create);
    config_save(configMetadataTallGrass);
    pthread_mutex_unlock(&metadata_create);

    desbloquearMetadata(pokemon);
    config_destroy(configMetadataTallGrass);
}

void intentarAbrirMetadata(char* metadata, char* pokemon){
    //pthread_mutex_lock(&metadata_create);
    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);
    //pthread_mutex_unlock(&metadata_create);

    char* valorOpen = malloc(sizeof(char)*5 + 1);
    
    strcpy(valorOpen, "");
    strcat(valorOpen, config_get_string_value(configMetadataTallGrass, "OPEN"));
    
    uint32_t tiempoReintento = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");

    while(strcmp(valorOpen, "Y") == 0){
        //desbloquearMetadata(pokemon);
        log_error(obligatory_logger, "Un proceso intento abrir un archivo que esta abierto (Reintentando en %d segundos)", tiempoReintento);
        sleep(tiempoReintento);
        
        config_destroy(configMetadataTallGrass);
        pthread_mutex_lock(&metadata_create);
        configMetadataTallGrass = config_create(metadata);
        pthread_mutex_unlock(&metadata_create);
        strcpy(valorOpen, "");
        strcat(valorOpen, config_get_string_value(configMetadataTallGrass, "OPEN"));
    }
    config_destroy(configMetadataTallGrass);
    free(valorOpen);
    abrirMetadata(metadata, pokemon);
}

void addBlockMetadata(char* metadata,char* block, new_pokemon* newPokemon){
    pthread_mutex_lock(&metadata_create);
    t_config* configMetadataTallGrass = config_create(metadata);
    pthread_mutex_unlock(&metadata_create);
    
    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);
    
    char** bloques = config_get_array_value(configMetadataTallGrass,"BLOCKS");

    char* bloquesConfig = malloc(sizeof(char)*3*(cantidadBloques + 1) + 1);
    strcpy(bloquesConfig,"");
    strcat(bloquesConfig,"[");
    for(int i = 0; i < cantidadBloques; i++){
        strcat(bloquesConfig,bloques[i]);
        strcat(bloquesConfig,",");
    }
    strcat(bloquesConfig,block); 
    strcat(bloquesConfig,"]");

    char* extension = ".bin";

    char* bloque = malloc(strlen(blocksPath) + strlen(block) + strlen(extension) + 2);
    strcpy(bloque,"");
    strcat(bloque, blocksPath);
    strcat(bloque, "/"); 
    strcat(bloque, block); 
    strcat(bloque, extension);
    
    FILE* file = fopen(bloque,"rb");

    fseek(file, 0, SEEK_END);
    int sizeFile = ftell(file);
    fseek(file, 0, SEEK_SET);
    /*if(feof(file)){ //ver que no rompa todo
        sizeFile--;
    }*/

    fclose(file);

    size += sizeFile;
    size --;

    char* sizeChar = malloc(10);
    strcpy(sizeChar, "");
    sprintf(sizeChar, "%d", size);

    config_set_value(configMetadataTallGrass, "SIZE", sizeChar);
    config_set_value(configMetadataTallGrass,"BLOCKS", bloquesConfig);

    pthread_mutex_lock(&metadata_create);
    config_save(configMetadataTallGrass);
    pthread_mutex_unlock(&metadata_create);
    log_info(obligatory_logger, "Se ha agregado un nuevo bloque al pokemon");
    config_destroy(configMetadataTallGrass);
    
    free(bloque);
    free(bloquesConfig);
    for(int i = 0; i<cantidadBloques; i++){
        free(bloques[i]);
    }
    free(bloques);
    free(sizeChar);
}