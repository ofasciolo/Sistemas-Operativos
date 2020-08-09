#include "initializer.h"

void initialize(){
    char* config_path = "cfg/broker.config";
    config = config_create(config_path);
    if (config == NULL) {
        error_show(
            "Configuration could not be loaded in %s. Aborting.\n", 
            config_path
        );
        exit(CONFIG_FAIL);
    }
    obligatory_logger = create_log_from_config("LOG_FILE", "BROKER", "LOG_SHOW");
    optional_logger = create_log_from_config(
        "OPTIONAL_LOG_FILE", "BROKER-D", "OPTIONAL_LOG_SHOW"
    );
    log_info(optional_logger, "Initializing Application...");
    fill_config_values();
    connections = list_create();
    id_connection = 0;
    id_message = 0;
    pthread_mutex_init(&m_id_connection, NULL);
    pthread_mutex_init(&m_id_message, NULL);
    pthread_mutex_init(&m_connections,NULL);
    pthread_mutex_init(&m_new_partition, NULL);
    init_queues();
    initializeMemory();
    set_sig_handler();
    p_on_request = &process_request;
    log_info(optional_logger, "Configuration and loggers loaded successfully.");
}

t_log* create_log_from_config(char* path_key, char* name, char* show_key){
    char* log_path = config_get_string_value(config, path_key);
    uint32_t log_show = config_get_int_value(config, show_key);
    t_log* logger =  log_create(log_path, name, log_show, LOG_LEVEL_INFO);
    if (logger==NULL){
        error_show("Log %s in path: %s could not be loaded. Aborting.\n", name, log_path);
        exit(LOG_FAIL);
    }
    return logger;
}

void fill_config_values(){
    cfg_values.tamano_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
    cfg_values.tamano_minimo_particion = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");
    cfg_values.algoritmo_memoria = config_get_string_value(config, "ALGORITMO_MEMORIA");
    cfg_values.algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    cfg_values.algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
    cfg_values.ip_broker = config_get_string_value(config, "IP_BROKER");
    cfg_values.puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
    cfg_values.frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
    cfg_values.dump_file = config_get_string_value(config, "DUMP_FILE");
    log_info(optional_logger, 
        "Configuration values: \n\n\tTAMANO_MEMORIA=%d\n\tTAMANO_MINIMO_PARTICION=%d\n\tALGORITMO_MEMORIA=%s\n\tALGORITMO_REEMPLAZO=%s\n\tALGORITMO_PARTICION_LIBRE=%s\n\tIP_BROKER=%s\n\tPUERTO_BROKER=%s\n\tFRECUENCIA_COMPACTACION=%d\n\tDUMP_FILE=%s\n\n",
        cfg_values.tamano_memoria, cfg_values.tamano_minimo_particion, cfg_values.algoritmo_memoria,
        cfg_values.algoritmo_reemplazo, cfg_values.algoritmo_particion_libre, cfg_values.ip_broker,
        cfg_values.puerto_broker, cfg_values.frecuencia_compactacion, cfg_values.dump_file
    );
}

void set_sig_handler(void)
{
    struct sigaction action;


    action.sa_flags = SA_SIGINFO; 
    action.sa_handler = dumpMemory;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGUSR1, &action, NULL) == -1) { 
        perror("sigusr: sigaction");
        _exit(1);
    }
}

void init_queues(){
    list_queues = list_create();
    for(int i=1; i<=6;i++){ //asumo que del 1 al 6 esta cada id de cola/mensaje
        t_message_queue* queue = malloc(sizeof(t_message_queue));
        queue->id_queue = i;
        queue->messages = queue_create();
        queue->subscribers = list_create();
        queue->sem_message = malloc(sizeof(sem_t));
        queue->m_queue_modify = malloc(sizeof(pthread_mutex_t));
        queue->m_subscribers_modify = malloc(sizeof(pthread_mutex_t));
        sem_init(queue->sem_message, 0, 0);
        pthread_mutex_init(queue->m_queue_modify,NULL);
        pthread_mutex_init(queue->m_subscribers_modify,NULL);
        list_add(list_queues,queue);
        pthread_create_and_detach(queue_message_sender,queue);
    }
}
