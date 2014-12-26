#ifndef WS2812_H_
#define WS2812_H_
#include <stdint.h>
#include "colors.h"


#ifdef __cplusplus
 extern "C" {
#endif

#define LED (60)

/**
* @brief WS2812_Init
* Initialize WS2812 Driver
*/
void WS2812_Init();

/**
 * This function sends data bytes out to a string of WS2812s
 * The first argument is a pointer to the first RGB triplet to be sent
 * The seconds argument is the number of LEDs in the chain
 *
 * This will result in the RGB triplet passed by argument 1 being sent to
 * the LED that is the furthest away from the controller (the point where
 * data is injected into the chain)
 * @param color - rgb map (index corresponds to led number)
 * @param leds  - number of leds specified in map, all others will be shutdown
 */
void WS2812_send(RGB_T* color, uint16_t leds);

/**
 * @brief DMA1_Channel6_IRQHandler
 * DMA Handler
 */
void DMA1_Channel6_IRQHandler();

/**
 * Shutdown all leds
 */
void WS2812_clear();

#ifdef __cplusplus
 }
#endif

#endif /* WS2812_H_ */
