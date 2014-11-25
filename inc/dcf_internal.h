#ifndef DCF_INTERNAL_H
#define DCF_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__ ((__packed__)) DCF77_Bits_t {
    uint8_t M			    :1	;
    uint8_t O1			:1	;
    uint8_t O2			:1	;
    uint8_t O3			:1	;
    uint8_t O4			:1	;
    uint8_t O5			:1	;
    uint8_t O6			:1	;
    uint8_t O7			:1	;
    uint8_t O8			:1	;
    uint8_t O9			:1	;
    uint8_t O10			:1	;
    uint8_t O11			:1	;
    uint8_t O12			:1	;
    uint8_t O13			:1	;
    uint8_t O14			:1	;
    uint8_t R			:1	;
    uint8_t A1			:1	;
    uint8_t Z1			:1	;
    uint8_t Z2			:1	;
    uint8_t A2			:1	;
    uint8_t S			:1	;
    uint8_t Min			:7	;//7 Bits für die Minuten
    uint8_t P1			:1	;//Parity Minuten
    uint8_t Hour		:6	;//6 Bits für die Stunden
    uint8_t P2			:1	;//Parity Stunden
    uint8_t Day			:6	;//6 Bits für den Tag
    uint8_t Weekday		:3	;//3 Bits für den Wochentag
    uint8_t Month		:5	;//3 Bits für den Monat
    uint8_t Year		:8	;//8 Bits für das Jahr **eine 5 für das Jahr 2005**
    uint8_t P3			:1	;//Parity von P2
};


struct __attribute__ ((__packed__)) DCF_Flags_t {
    volatile uint8_t parity_err			:1	;//Hilfs Parity
    volatile uint8_t parity_P1			:1	;//Berechnetes Parity P1
    volatile uint8_t parity_P2			:1	;//Berechnetes Parity P2
    volatile uint8_t parity_P3			:1	;//Berechnetes Parity P3
    volatile bool dcf_rx				:1	;//Es wurde ein Impuls empfangen
    volatile bool dcf_sync				:1	;//In der letzten Minuten wurde die Uhr syncronisiert
};


#ifdef __cplusplus
}
#endif

#endif // DCF_INTERNAL_H
