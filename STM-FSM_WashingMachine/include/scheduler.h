#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define TABLE_SIZE 16
#define QUEUE_SIZE 32
#define NONE 0xff

void Scheduler_Dispatch();
void Scheduler_AddTask(void (*task)(), uint32_t delay, uint32_t period);
void Scheduler_Step();

#endif