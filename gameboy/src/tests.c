#include "tests.h"

void test1(){
    
}

void run_tests_gameboy(){

    CU_initialize_registry();
    CU_pSuite prueba = CU_add_suite("GAMEBOY Suite",NULL,NULL);
    CU_add_test(prueba,"Test de prueba",test1);
    run_tests();
}