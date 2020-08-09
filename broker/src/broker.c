#include "broker.h"


int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        broker_run_tests();
    else{  
        initialize();

        start_server(
            cfg_values.ip_broker, 
            cfg_values.puerto_broker,
            p_on_request    
        );
        finalize();
        return EXIT_SUCCESS;
    }
    
}