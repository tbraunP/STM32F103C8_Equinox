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

/**
 * @brief Clock_Sync
 * @param dcfTime - new time
 *
 * perform local clock synchronisation
 */
void Clock_Sync(volatile struct DCF77_Time_t* dcfTime);

#ifdef __cplusplus
 }
#endif

#endif // CLOCK_H