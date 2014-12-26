#ifndef COLORS_H_
#define COLORS_H_

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

extern uint8_t eightbit[766][3];


typedef struct RGB_T{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
} RGB_T;


typedef struct HSV_T{
    uint16_t h;     // color degree (0..359)
    uint8_t s;      // 0..100, saturation
    uint8_t v;      // 0..100, value
} HSV_T;


/**
 * @brief convertHSV2RGB
 * Convert HSV to RGB
 * @param hsv - HSV value
 * @return RGB value
 */
RGB_T convertHSV2RGB(const HSV_T* hsv);

#ifdef __cplusplus
 }
#endif

#endif /* COLORS_H_ */
