#ifndef MAPA_H
#define MAPA_H

#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include "common_utils.h"
#include "inicializacion.h"

typedef enum{
	P_FREE = 0,
    P_CHASING,
    P_CATCHED
} e_pokemon_catch_state;

typedef struct {
    char* pokemon;
    t_position position;
    e_pokemon_catch_state state;
    int id;
} t_pokemon_on_map;
// a medida que se reciban apariciones / localizaciones de pokemones se agregan a la lista de pokemon_on_map
t_list* pokemonsOnMap;
pthread_mutex_t pokemonsOnMap_mutex;


#endif