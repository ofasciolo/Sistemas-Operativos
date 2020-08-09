#ifndef TESTS_H
#define TESTS_H

#include "broker.h"
#include <CUnit/Basic.h>

void broker_run_tests();
void test_agregar_a_queue();
void manejo_memoria();

bool testSeekAlgorithms();
bool testAllocateDataBS();
bool testAllocateDataDP();
bool testDestroyPartitionFIFO();    
bool testDestroyPartitionLRU();
void internalStringData(uint32_t id, char* text);
bool testCompactDP();
bool testCompactBS();
bool testDump();
void baseBroker();
void consolidacionBroker();
void compactacionBroker();
void buddySystemBroker();

#endif