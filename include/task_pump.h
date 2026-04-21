#ifndef __TASK_PUMP_H__
#define __TASK_PUMP_H__

#include "global.h"

bool pump_on(int sec);
bool pump_off(int sec);
void task_pump (void *pvParameter);

#endif