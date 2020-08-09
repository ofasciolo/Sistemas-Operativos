#include "common_utils.h"

new_pokemon* stream_to_new_pokemon(void* stream, uint32_t* id_message, bool is_broker){
    //reemplazar en el ternario por el metodo de alloc del broker
    new_pokemon* newPokemonMessage = malloc(sizeof(new_pokemon));

    memcpy(&(newPokemonMessage->sizePokemon), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    newPokemonMessage->pokemon = malloc(newPokemonMessage->sizePokemon);
    memcpy(newPokemonMessage->pokemon, stream, newPokemonMessage->sizePokemon);
    stream += newPokemonMessage->sizePokemon;
    memcpy(&((newPokemonMessage->position).posx), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&((newPokemonMessage->position).posy), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(newPokemonMessage->quantity), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_message != NULL) memcpy(id_message, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    return newPokemonMessage;
}

uint32_t size_of_new_pokemon(new_pokemon* newPokemonMessage){
    return sizeof(uint32_t) * 5 + newPokemonMessage->sizePokemon;
}

void* new_pokemon_to_stream(new_pokemon* newPokemonMessage, uint32_t* id_message){

    uint32_t size = size_of_new_pokemon(newPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(newPokemonMessage->sizePokemon), sizeof(uint32_t));
    forward += sizeof(uint32_t);    
    memcpy(stream + forward, newPokemonMessage->pokemon, newPokemonMessage->sizePokemon);
    forward += newPokemonMessage->sizePokemon;
    memcpy(stream + forward, &((newPokemonMessage->position).posx), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, &((newPokemonMessage->position).posy), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, &(newPokemonMessage->quantity), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

appeared_pokemon* stream_to_appeared_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker){

    appeared_pokemon* appearedPokemonMessage = malloc(sizeof(appeared_pokemon)); 

    memcpy(&(appearedPokemonMessage->sizePokemon), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    appearedPokemonMessage->pokemon = malloc(appearedPokemonMessage->sizePokemon );
    memcpy(appearedPokemonMessage->pokemon, stream, appearedPokemonMessage->sizePokemon);
    stream += appearedPokemonMessage->sizePokemon;
    memcpy(&((appearedPokemonMessage->position).posx), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&((appearedPokemonMessage->position).posy), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_message != NULL) memcpy(id_message, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_correlational != NULL) memcpy(id_correlational, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return appearedPokemonMessage;
}

uint32_t size_of_appeared_pokemon(appeared_pokemon* appearedPokemonMessage){
    return sizeof(uint32_t ) * 5 + appearedPokemonMessage->sizePokemon;
}

void* appeared_pokemon_to_stream(appeared_pokemon* appearedPokemonMessage, uint32_t* id_message, uint32_t* id_correlational){

    uint32_t size = size_of_appeared_pokemon(appearedPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(appearedPokemonMessage->sizePokemon), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, appearedPokemonMessage->pokemon, appearedPokemonMessage->sizePokemon);
    forward += appearedPokemonMessage->sizePokemon;
    memcpy(stream + forward, &((appearedPokemonMessage->position).posx), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, &((appearedPokemonMessage->position).posy), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_correlational, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

catch_pokemon* stream_to_catch_pokemon(void* stream, uint32_t* id_message, bool is_broker){

    catch_pokemon* catchPokemonMessage = malloc(sizeof(catch_pokemon)); 

    memcpy(&(catchPokemonMessage->sizePokemon), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    catchPokemonMessage->pokemon = malloc(catchPokemonMessage->sizePokemon);
    memcpy(catchPokemonMessage->pokemon, stream, catchPokemonMessage->sizePokemon);
    stream += catchPokemonMessage->sizePokemon;
    memcpy(&((catchPokemonMessage->position).posx), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&((catchPokemonMessage->position).posy), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_message != NULL) memcpy(id_message, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return catchPokemonMessage;
}

uint32_t size_of_catch_pokemon(catch_pokemon* catchPokemonMessage){
    return sizeof(uint32_t ) * 4 + catchPokemonMessage->sizePokemon;
}

void* catch_pokemon_to_stream(catch_pokemon* catchPokemonMessage, uint32_t* id_message){

    uint32_t size = size_of_catch_pokemon(catchPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(catchPokemonMessage->sizePokemon), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, catchPokemonMessage->pokemon, catchPokemonMessage->sizePokemon );
    forward += catchPokemonMessage->sizePokemon;
    memcpy(stream + forward, &((catchPokemonMessage->position).posx), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, &((catchPokemonMessage->position).posy), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

caught_pokemon* stream_to_caught_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker){

    caught_pokemon* caughtPokemonMessage = malloc(sizeof(caught_pokemon)); 

    memcpy(&(caughtPokemonMessage->success), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_message != NULL) memcpy(id_message, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    if (id_correlational != NULL) memcpy(id_correlational, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return caughtPokemonMessage;
}

uint32_t size_of_caught_pokemon(caught_pokemon* caughtPokemonMessage){
    return sizeof(uint32_t) * 3;
}

void* caught_pokemon_to_stream(caught_pokemon* caughtPokemonMessage, uint32_t* id_message, uint32_t* id_correlational){

    uint32_t size = size_of_caught_pokemon(caughtPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(caughtPokemonMessage->success), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_correlational, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

get_pokemon* stream_to_get_pokemon(void* stream, uint32_t* id_message, bool is_broker){

    get_pokemon* getPokemonMessage = malloc(sizeof(get_pokemon)); 

    memcpy(&(getPokemonMessage->sizePokemon), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    getPokemonMessage->pokemon = malloc(getPokemonMessage->sizePokemon);
    memcpy(getPokemonMessage->pokemon, stream, getPokemonMessage->sizePokemon);
    stream += getPokemonMessage->sizePokemon;
    if (id_message != NULL) memcpy(id_message, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return getPokemonMessage;
}

uint32_t size_of_get_pokemon(get_pokemon* getPokemonMessage){
    return sizeof(uint32_t ) * 2 + getPokemonMessage->sizePokemon;
}

void* get_pokemon_to_stream(get_pokemon* getPokemonMessage, uint32_t* id_message){

    uint32_t size = size_of_get_pokemon(getPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(getPokemonMessage->sizePokemon), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, getPokemonMessage->pokemon, getPokemonMessage->sizePokemon);
    forward += getPokemonMessage->sizePokemon;
    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

localized_pokemon* stream_to_localized_pokemon(void* stream, uint32_t* id_message, uint32_t* id_correlational, bool is_broker){

    localized_pokemon* localizedPokemonMessage = malloc(sizeof(localized_pokemon)); 
    uint32_t sizePositions = 0, forward = 0; 

    memcpy(&(localizedPokemonMessage->sizePokemon), stream + forward, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    localizedPokemonMessage->pokemon = malloc(localizedPokemonMessage->sizePokemon);
    memcpy(localizedPokemonMessage->pokemon, stream + forward, localizedPokemonMessage->sizePokemon);
    forward += localizedPokemonMessage->sizePokemon;
    memcpy(&(sizePositions), stream + forward, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    
    localizedPokemonMessage->positions = list_create();
    for(int i = 0; i < sizePositions; i++){
        t_position* position = malloc(sizeof(t_position));
        memcpy(&(position->posx), stream + forward, sizeof(uint32_t));
        forward += sizeof(uint32_t);
        memcpy(&(position->posy), stream + forward, sizeof(uint32_t));
        forward += sizeof(uint32_t);
        list_add(localizedPokemonMessage->positions, position);
    }
    if (id_message != NULL) memcpy(id_message, stream + forward, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    if (id_message != NULL) memcpy(id_correlational, stream + forward, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return localizedPokemonMessage;
}

uint32_t size_of_localized_pokemon(localized_pokemon* localizedPokemonMessage){
    return sizeof(uint32_t) * 4 + localizedPokemonMessage->sizePokemon + sizeof(uint32_t) * 2 * (*localizedPokemonMessage->positions).elements_count;
}

void* localized_pokemon_to_stream(localized_pokemon* localizedPokemonMessage, uint32_t* id_message, uint32_t* id_correlational){

    uint32_t size = size_of_localized_pokemon(localizedPokemonMessage);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(localizedPokemonMessage->sizePokemon), sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, localizedPokemonMessage->pokemon, localizedPokemonMessage->sizePokemon);
    forward += localizedPokemonMessage->sizePokemon;
    uint32_t amount_positions = list_size(localizedPokemonMessage->positions);
    memcpy(stream + forward, &amount_positions, sizeof(uint32_t));
    forward += sizeof(uint32_t);

    for(int i = 0; i < (*localizedPokemonMessage->positions).elements_count; i++){
        t_position* position = list_get(localizedPokemonMessage->positions, i);
        memcpy(stream + forward, &(position->posx), sizeof(uint32_t));
        forward += sizeof(uint32_t);
        memcpy(stream + forward, &(position->posy), sizeof(uint32_t)); 
        forward += sizeof(uint32_t);
    }

    memcpy(stream + forward, id_message, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    memcpy(stream + forward, id_correlational, sizeof(uint32_t));
    forward += sizeof(uint32_t);
    return stream;
}

subscribe* stream_to_subscribe(void* stream){

    subscribe* subscribeMessage = malloc(sizeof(subscribe)); 

    memcpy(&(subscribeMessage->colaMensajes), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return subscribeMessage;
}

void* subscribe_to_stream(subscribe* subscribeMessage){

    uint32_t size = sizeof(uint32_t);
    void* stream = malloc(size); 
    uint32_t forward = 0;

    memcpy(stream + forward, &(subscribeMessage->colaMensajes), sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

new_connection* stream_to_new_connection(void* stream){

    new_connection* newConnectionMessage = malloc(sizeof(new_connection));
    return newConnectionMessage;
}

void* new_connection_to_stream(new_connection* newConnectionMessage){
    void* stream = malloc(sizeof(new_connection)); 
    return stream;
}

reconnect* stream_to_reconnect(void* stream){

    reconnect* reconnectMessage = malloc(sizeof(reconnect)); 

    memcpy(&(reconnectMessage->id_connection), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return reconnectMessage;
}

void* reconnect_to_stream(reconnect* reconnectMessage){

    uint32_t size = sizeof(uint32_t);
    void* stream = malloc(size);
    uint32_t forward = 0;

    memcpy(stream + forward, &(reconnectMessage->id_connection), sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

connection* stream_to_connection(void* stream){

    connection* connectionMessage = malloc(sizeof(connection)); 

    memcpy(&(connectionMessage->id_connection), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return connectionMessage;
}

void* connection_to_stream(connection* connectionMessage){

    uint32_t size = sizeof(uint32_t);
    void* stream = malloc(size);
    uint32_t forward = 0;

    memcpy(stream + forward, &(connectionMessage->id_connection), sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

ack* stream_to_ack(void* stream){

    ack* acknowledgementMessage = malloc(sizeof(ack));

    memcpy(&(acknowledgementMessage->id_message), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return acknowledgementMessage;
}

void* ack_to_stream(ack* acknowledgementMessage){

    uint32_t size = sizeof(uint32_t);
    void* stream = malloc(size);
    uint32_t forward = 0;

    memcpy(stream + forward, &(acknowledgementMessage->id_message), sizeof(uint32_t));
    forward += sizeof(uint32_t);

    return stream;
}

new_pokemon* init_new_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t quantity){
    new_pokemon* pokemon = malloc(sizeof(new_pokemon));
    pokemon->pokemon = nombre;
    pokemon->sizePokemon = strlenNewLine(pokemon->pokemon);
    pokemon->position.posx = posx;
    pokemon->position.posy = posy;
    pokemon->quantity = quantity;
    return pokemon;
}

appeared_pokemon* init_appeared_pokemon(char* nombre, uint32_t posx, uint32_t posy){
    appeared_pokemon* pokemon = malloc(sizeof(appeared_pokemon));
    pokemon->pokemon = nombre;
    pokemon->sizePokemon = strlenNewLine(pokemon->pokemon);
    pokemon->position.posx = posx;
    pokemon->position.posy = posy;
    return pokemon;
}

catch_pokemon* init_catch_pokemon(char* nombre, uint32_t posx, uint32_t posy){
    catch_pokemon* pokemon = malloc(sizeof(catch_pokemon));
    pokemon->pokemon = nombre;
    pokemon->sizePokemon = strlenNewLine(pokemon->pokemon);
    pokemon->position.posx = posx;
    pokemon->position.posy = posy;
    return pokemon;
}

caught_pokemon* init_caught_pokemon(bool success){
    caught_pokemon* pokemon = malloc(sizeof(caught_pokemon));
    pokemon->success = success;
    return pokemon;
}

get_pokemon* init_get_pokemon(char* nombre){
    get_pokemon* pokemon = malloc(sizeof(get_pokemon));
    pokemon->pokemon = nombre;
    pokemon->sizePokemon = strlenNewLine(pokemon->pokemon);
    return pokemon; 
}

localized_pokemon* init_localized_pokemon(char* nombre, t_list* positions){
    localized_pokemon* pokemon = malloc(sizeof(localized_pokemon));
    pokemon->pokemon = nombre;
    pokemon->sizePokemon = strlenNewLine(pokemon->pokemon);
    pokemon->positions = positions;
    return pokemon;
}

new_connection* init_new_connection(){
    new_connection* con = malloc(sizeof(new_connection));
    return con;
}

reconnect* init_reconnect(uint32_t id_connection){
    reconnect* recon = malloc(sizeof(reconnect));
    recon->id_connection = id_connection;
    return recon;
}

connection* init_connection(uint32_t id_connection){
    connection* con = malloc(sizeof(connection));
    con->id_connection = id_connection;
    return con;
}

ack* init_ack(uint32_t id_message){
    ack* acknowledgement = malloc(sizeof(ack));
    acknowledgement->id_message = id_message;
    return acknowledgement;
}

subscribe* init_subscribe(uint32_t id_queue){
    subscribe* subs = malloc(sizeof(subscribe));
    subs->colaMensajes = id_queue;
    return subs;
}

void free_new_pokemon(new_pokemon* pokemon){
    //if(pokemon->pokemon) free(pokemon->pokemon);
    free(pokemon);
}

void free_appeared_pokemon(appeared_pokemon* pokemon){
    //if(pokemon->pokemon) free(pokemon->pokemon);
    free(pokemon);
}

void free_catch_pokemon(catch_pokemon* pokemon){
    //if(pokemon->pokemon) free(pokemon->pokemon);
    free(pokemon);
}

void free_caught_pokemon(caught_pokemon* pokemon){
    free(pokemon);
}

void free_get_pokemon(get_pokemon* pokemon){
    //if(pokemon->pokemon) free(pokemon->pokemon);
    free(pokemon);
}

void free_localized_pokemon(localized_pokemon* pokemon){
   // if(pokemon->pokemon) free(pokemon->pokemon);
    if(pokemon->positions) list_destroy_and_destroy_elements(pokemon->positions, free);
    free(pokemon);
}

void free_subscribe(subscribe* subscriber){
    free(subscriber);
}

void free_new_connection(new_connection* conn){
    free(conn);
}

void free_reconnect(reconnect* reconn){
    free(reconn);
}

void free_connection(connection* conn){
    free(conn);
}

void free_ack(ack* acknowledgement){
    free(acknowledgement);
}

void* list_find_with_args(t_list *self, bool(*condition)(void*, void*), void* args) {
	t_link_element *element = self->head;
	int position = 0;

	while (element != NULL && !condition(element->data, args)) {
		element = element->next;
		position++;
	}

	return element != NULL ? element->data : NULL;
}

bool has_socket_fd(void* data, void* socket){
    return data ? ((t_connection*) data)->socket == (uint32_t) socket && ((t_connection*) data)->is_connected: false;
}

int strlenNewLine (const char *str){
    const char *char_ptr;
    for (char_ptr = str; *char_ptr != '\n' && *char_ptr != '\0' && *char_ptr != 13 ;++char_ptr);
    return char_ptr - str;
}

uint64_t timestamp(void) {
    struct timeval valor;
    gettimeofday(&valor, NULL);
    unsigned long long result = (((unsigned long long )valor.tv_sec) * 1000 + ((unsigned long) valor.tv_usec));
    uint64_t tiempo = result;
    return tiempo;
}



