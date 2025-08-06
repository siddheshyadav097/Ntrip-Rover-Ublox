#ifndef __QTIMESPENT_H
#define __QTIMESPENT_H

#include "stm32g0xx_hal.h"

void InitQTimeSpent(void);
uint32_t GetStartTime(void);
uint8_t TimeSpent(uint32_t startTick, uint32_t totalTimeInMs);

#endif