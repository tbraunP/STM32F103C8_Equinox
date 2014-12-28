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
 * Perform a resynchronisation between DCF77Clock and local clock,
 * triggered by the minute overflow
 * @param dcfTime - current time
 * @param failed - number of consecutive failed syncs (0 if syncs are consecutive, i.e. every minute)
 */
void Clock_Sync(volatile struct DCF77_Time_t* dcfTime, uint8_t failed);

#ifdef __cplusplus
 }
#endif

#endif // CLOCK_H
