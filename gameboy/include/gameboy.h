#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "common_utils.h"
#include "server_gameboy.h"
#include "tests.h"
#include <time.h>


void closeAll(t_log* optional_logger,t_log* obligatory_logger, t_config* config, int connection);
void countTime(void* timePassed);

#endif