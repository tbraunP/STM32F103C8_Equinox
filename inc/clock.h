#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

void Clock_Init();

/*
 * Compare Interrupt
 */
void TIM4_IRQHandler(void);


#ifdef __cplusplus
 }
#endif

#endif // CLOCK_H
