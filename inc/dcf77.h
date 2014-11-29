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


void DFC77_init();


// EXTI-Handler PA0 for DCF77 Module
void EXTI0_IRQHandler(void);

// DCF Timer Handler
void TIM2_IRQHandler(void);

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
