#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

void Clock_Init();

/*
 * Compare Interrupt
 */
void TIM4_IRQHandler(void);

#endif // CLOCK_H
