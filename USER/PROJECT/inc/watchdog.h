#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "stm32f4xx.h"

void watchdog_init(u16 time_ms);
void watchdog_reload(void);

#endif
