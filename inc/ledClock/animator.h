#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

 /**
 * @brief updateVisualization
 * @param hours - time in hours
 * @param minutes - time in minutes
 * @param seconds - time in seconds
 * @param posInSecond - position within second
 *
 * Visualize Clock using a ws2812 stripe with LED (see ws2812.h)
 */
void updateVisualization(uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t posInSecond);


#ifdef __cplusplus
}
#endif

#endif // ANIMATOR_H
