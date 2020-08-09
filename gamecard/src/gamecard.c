#include "gamecard.h"

/* 
Hay que implementar el filesystem: TALL_GRASS
Se comunica de 2 formas:
    1.	A través de la conexión con el Broker asociándose globalmente a la cola de mensajes NEW_POKEMON, CATCH_POKEMON  y GET_POKEMON.
    2.	A través de un socket de escucha en el cual podrá recibir mensajes de las colas de mensajes mencionadas en el punto anterior.
Si el Broker se encuentra sin funcionar o se cae durante la ejecución, el proceso Game Card debe seguir procesando sus funciones sin el mismo.
En caso que la conexión no llegue a realizarse o se caiga, el proceso Game Card deberá contar con un sistema de reintento de conexión cada X segundos configurado desde archivo de configuración. 

*/
/*--Filesystem: TALL_GRASS--
El árbol de directorios tomará su punto de partida del punto de montaje del archivo de configuración.
Durante las pruebas no se proveerán archivos que tengan estados inconsistentes respecto del trabajo práctico, por lo que no es necesario tomar en cuenta dichos casos.

-Metadata:
Este archivo tendrá la información correspondiente a la cantidad de bloques y al tamaño de los mismos dentro del File System.
-Dentro del archivo se encontrarán los siguiente campos:
●	Block_size: Indica el tamaño en bytes de cada bloque (BLOCK_SIZE=64)
●	Blocks: Indica la cantidad de bloques del File System (BLOCKS=5192)
●	Magic_Number: Un string fijo con el valor “TALL_GRASS” (MAGIC_NUMBER=TALL_GRASS)
Dicho  archivo deberá encontrarse en la ruta [Punto_Montaje]/Metadata/Metadata.bin

-Bitmap:
Este será un archivo de tipo binario donde solamente existirá un bitmap , el cual representará el estado de los bloques dentro del FileSystem, siendo un 1 que el bloque está ocupado y un 0 que el bloque está libre.
La ruta del archivo de bitmap es: [Punto_Montaje]/Metadata/Bitmap.bin

-Files Metadata:
Los archivos dentro del FS se encontrarán en un path compuesto de la siguiente manera:
[Punto_Montaje]/Files/[Nombre_Archivo]
Donde el path del archivo incluye el archivo Metadata.
Dentro del archivo Metadata.bin se encontrarán los siguientes campos:
●	Directory: indica si el archivo en cuestión es un directorio o no (Y/N).
●	Size: indica el tamaño real del archivo en bytes (en caso de no ser un directorio).
●	Blocks: es un array de números que contiene el orden de los bloques en donde se encuentran los datos propiamente dichos de ese archivo (en caso de no ser un directorio).
●	Open: indica si el archivo se encuentra abierto (Y/N).

-Datos:
Los datos estarán repartidos en archivos de texto nombrados con un número, el cual representará el número de bloque. (Por ej 1.bin, 2.bin, 3.bin), 
Dichos archivos se encontraran dentro de la ruta:
[Punto_Montaje]/Blocks/[nroBloque].bin
Ej: 
/mnt/TALL_GRASS/Blocks/1.bin
/mnt/TALL_GRASS/Blocks/2.bin

*/
/*--Lineamientos e implementacion:
Este proceso gestionará un Filesystem que será leído e interpretado como un árbol de directorios y sus archivos utilizando el Filesystem Tall Grass.
Al iniciar el proceso Game Card se intentara suscribir globalmente al Broker a las siguientes colas de mensajes:
●	NEW_POKEMON
●	CATCH_POKEMON
●	GET_POKEMON
Al suscribirse a cada una de las colas deberá quedarse a la espera de recibir un mensaje del Broker. Al recibir un mensaje de cualquier hilo se deberá:
1.	Informar al Broker la recepción del mismo (ACK).
2.	Crear un hilo que atienda dicha solicitud.
3.	Volver a estar a la escucha de nuevos mensajes de la cola de mensajes en cuestión.
Todo archivo dentro del file system tendrá un valor “OPEN” dentro de su metadata que indicará si actualmente hay algún proceso que se encuentra utilizando el mismo.
No permitir a dos procesos abrir el mismo archivo en simultáneo. En caso que suceda esto, informar el error pertinente por archivo de log o consola.

*/
int main(int argc, char ** argv){
    iniciarGameCard();
    return 0;
}
