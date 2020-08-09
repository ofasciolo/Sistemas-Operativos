#include "fileSystemTallGrass.h"

void obtenerConfig(){
	
	char* metadata = malloc(strlen(metadataPath) + strlen("/Metadata.bin") + 1);
	strcpy(metadata, "");
	strcat(metadata, metadataPath);
	strcat(metadata, "/Metadata.bin");

    t_config* configMetadata = config_create(metadata);

    configM.blockSize = config_get_int_value(configMetadata, "BLOCK_SIZE");
    configM.blocks = config_get_int_value(configMetadata, "BLOCKS");

    config_destroy(configMetadata);
	free(metadata);
}

void crearArchivoBitmap(){

	int blockNum = configM.blocks;
	while(blockNum%8!=0){
		blockNum++;
	}
	int bytes = BIT_CHAR(blockNum);
	char* bitsVacios = string_repeat(0,bytes);

	FILE *fp;
	t_bitarray* arrayCreador = bitarray_create_with_mode(bitsVacios,bytes,0);


	fp=fopen(bitmapPath,"w");
	fwrite(arrayCreador->bitarray,1,bytes,fp);
	fclose(fp);

	bitarray_destroy(arrayCreador); 

	free(bitsVacios);
}

void imprimirBITARRAY(t_bitarray* bitarray){

	int blockNum = configM.blocks;

	for (int i = 0; i<blockNum;i++){
		printf("%d",bitarray_test_bit(bitarray,i));
	}
	printf("\n");

}

void crearBitMap(){

	int blockNum = configM.blocks;
	while(blockNum%8!=0){
			blockNum++; 
	}
	int bytes = BIT_CHAR(blockNum);
	char* archivoBitmap;

	int fd = open(bitmapPath, O_RDWR , (mode_t)0600);
	struct stat mystat;
	if(fstat(fd,&mystat)<0){
		log_error(obligatory_logger, "No se pudo crear bitmap.");
		close (fd);
	}

	archivoBitmap=(char*) mmap(0,mystat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED ,fd,0);
	if(archivoBitmap==MAP_FAILED){
		log_error(obligatory_logger,"Error al mapear en memoria");
		close(fd);
	}

	bitmap=bitarray_create_with_mode(archivoBitmap , mystat.st_size ,0);

	msync(archivoBitmap,bytes,MS_SYNC);

   // imprimirBITARRAY(bitmap);

	close(fd);
}


void ActualizarBitmap(){

	int blockNum = configM.blocks;
	while(blockNum%8!=0){
			blockNum++; 
	}
	int bytes = BIT_CHAR(blockNum);

	FILE *fp;
	fp=fopen(bitmapPath,"w");
	fwrite(bitmap->bitarray,1,bytes,fp);

	fclose(fp);
}

int hayBitmap(){
	bitmapPath = malloc(strlen(PUNTO_MONTAJE) + strlen("/Metadata") + strlen("/Bitmap.bin") + 1); 
	strcpy(bitmapPath,""); 
	strcat(bitmapPath, metadataPath);
	strcat(bitmapPath, "/Bitmap.bin");

    return access( bitmapPath, F_OK ) != -1;

}

void crearPuntoDeMontaje(){
	mkdir(PUNTO_MONTAJE, ACCESSPERMS);

	filesPath = malloc(strlen(PUNTO_MONTAJE) + strlen("/Files") + 1); 
	strcpy(filesPath,""); 
	strcat(filesPath, PUNTO_MONTAJE);
	strcat(filesPath, "/Files");

	mkdir(filesPath, ACCESSPERMS);

	metadataFiles();

	blocksPath = malloc(strlen(PUNTO_MONTAJE) + strlen("/Blocks") + 1); 
	strcpy(blocksPath,""); 
	strcat(blocksPath, PUNTO_MONTAJE);
	strcat(blocksPath, "/Blocks");

	mkdir(blocksPath, ACCESSPERMS);

	metadataPath = malloc(strlen(PUNTO_MONTAJE) + strlen("/Metadata") + 1); 
	strcpy(metadataPath,""); 
	strcat(metadataPath, PUNTO_MONTAJE);
	strcat(metadataPath, "/Metadata");

	mkdir(metadataPath, ACCESSPERMS);

	metadataMetadata();

}

void metadataFiles(){
	t_config* configToWrite = config_create("./cfg/files_metadata.config");

	char* files = malloc(strlen(filesPath) + strlen("/Metadata.bin") + 1);
	strcpy(files, "");
	strcat(files, filesPath);
	strcat(files, "/Metadata.bin");

	config_save_in_file(configToWrite, files);
	config_destroy(configToWrite); 
	free(files);
}

void metadataMetadata(){
	t_config* configToWrite = config_create("./cfg/metadata_metadata.config");

	char* metadata = malloc(strlen(metadataPath) + strlen("/Metadata.bin") + 1);
	strcpy(metadata, "");
	strcat(metadata, metadataPath);
	strcat(metadata, "/Metadata.bin");


	if(access( metadata, F_OK ) == -1){
		config_save_in_file(configToWrite, metadata);
	}

	config_destroy(configToWrite); 
	free(metadata);
}

void iniciarTallGrass(){
	crearPuntoDeMontaje(); 
    obtenerConfig();
    
    if(!hayBitmap()){
        crearArchivoBitmap();
    }

    crearBitMap();
	log_info(obligatory_logger, "FileSystem iniciado correctamente.");
}
