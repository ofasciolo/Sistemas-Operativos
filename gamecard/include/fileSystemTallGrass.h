#ifndef FILESYSTEMTALLGRASS_H_
#define FILESYSTEMTALLGRASS_H_

#include "common_utils.h"
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <commons/string.h>

typedef struct{
    uint32_t blocks;
    uint32_t blockSize;
}configMetadata;

configMetadata configM;
t_bitarray* bitmap;
char* PUNTO_MONTAJE; 

char* blocksPath;
char* filesPath; 
char* metadataPath;
char* bitmapPath;

void obtenerConfig();
void crearBitmap();
void iniciarTallGrass();
void crearPuntoDeMontaje();
void metadataFiles();
void metadataMetadata();
void imprimirBITARRAY(t_bitarray* bitarray);

#endif