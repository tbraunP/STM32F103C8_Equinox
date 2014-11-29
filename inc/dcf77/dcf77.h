#ifndef DCF77_H
#define DCF77_H
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

struct DCF77_Time_t{
 volatile uint8_t ss;	//Globale Variable für Sekunden
 volatile uint8_t mm;	//Globale Variable für Minuten
 volatile uint8_t hh;	//Globale Variable für Stunden
 volatile uint8_t day;	//Globale Variable für den Tag
 volatile uint8_t mon;	//Globale Variable für den Monat
 volatile uint16_t year;	//Globale Variable für den Jahr
};

extern volatile struct DCF77_Time_t dcf;


/**
 * @brief DFC77_init
 * Start DCF77 Clock
 */
void DFC77_init();


// EXTI-Handler PA0 for DCF77 Module
void EXTI0_IRQHandler(void);

// DCF Timer Handler
void TIM2_IRQHandler(void);


/**
 * @brief DCF77_decrementTime
 * @param time - time to decrement
 * Decrement given time
 */
void DCF77_decrementTime(volatile struct DCF77_Time_t* time);


/**
 * @brief DCF77_incrementTime
 * @param time - time to increment
 * Increment given time by one second
 */
void DCF77_incrementTime(volatile struct DCF77_Time_t* time);


/**
 * @brief clone a time from src to dest
 * @param dest
 * @param src
 */
void DFC77_cloneDCF(volatile struct DCF77_Time_t*dest, volatile struct DCF77_Time_t* src);

#ifdef __cplusplus
 }
#endif

#endif // DCF77_H
