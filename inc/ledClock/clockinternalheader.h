#ifndef CLOCKINTERNALHEADER_H
#define CLOCKINTERNALHEADER_H
#include <stdint.h>


// CONSTANTS
#define     UPDATE_RATE_SEC         (25*2)
#define     RATE_MIN                (60*UPDATE_RATE_SEC)
#define     PRESCALER               (uint16_t) (0x2C1E/2)
#define     COUNTERVALUE40MS        (uint16_t) (0xFF)
#define     COUNTERVALUE40MS_2      (uint16_t) (COUNTERVALUE40MS/2)


// Visulization
// 0..100
#define SATURATION                  (100)
// 0..360
#define BASECOLOR_SECONDS           (0)
#define BASECOLOR_MINUTES           (120)
#define BASECOLOR_HOURS             (240)
// 0..100
#define MAX_BRIGHTNESS              (20)


#endif // CLOCKINTERNALHEADER_H

