#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "dcf77/dcf77.h"

#ifdef __cplusplus
 extern "C" {
#endif

extern volatile struct DCF77_Time_t clockTime;

/**
 * @brief Clock_Init
 * Local clock
 */
void Clock_Init(volatile struct DCF77_Time_t* dcfTime);

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
