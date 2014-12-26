#include "animator.h"

#include <stdio.h>
#include <stdbool.h>

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "hw/uart.h"
#include "ws2812.h"


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

// macrotick counter
static volatile uint16_t loops = 0;
static volatile uint8_t seconds = 0;
static volatile uint8_t minutes = 0;
static volatile uint8_t hours = 0;

/**
 * data structure for animation (2 leds + brightness)
 * led - which should be light up;
 * lightUp - 0 <= lightUp <= 1 (intensity);
 * ledNext - nextLed which should be light up;
 * lightUpNext - 0 <= lightUp <= 1 (intensity)
 */
typedef struct ANIM_LED_t{
    uint16_t led, ledNext;
    float lightUpLED, lightUpLEDNext;
} ANIM_LED_t;

// forward declarations
static void updateVisualization(uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t posInSecond);

// RGB structure to draw on
static RGB_T rgbStripe[LED];
static HSV_T hsvStripe[LED];


/**
 * Compare Interrupt
 */
void TIM4_IRQHandler(void){
    if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET){
        // clear IRQ Status
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
        NVIC_ClearPendingIRQ(TIM4_IRQn);

        // now increment local loop counter and modify local
        // time on second increment
        ++loops;


        if(loops % UPDATE_RATE_SEC == 0){
            // primitve watch
            seconds++;
            if(seconds == 60){
                seconds = 0;
                ++minutes;
                if(minutes == 60){
                    ++hours;
                    minutes = 0;
                    if(hours == 24){
                        hours = 0;
                    }
                }
            }


            seconds = seconds % 60;
            loops = 0;
            //UART_SendString("S\n\0");
        }

        // update timer compare register and update duration of current cycle /second in ticks
        TIM4->CCR1 += COUNTERVALUE40MS;

        // update visualization of clock
        updateVisualization(hours, minutes, seconds, (loops % UPDATE_RATE_SEC));
    }
}

/**
 * @brief calcLED
 * Calculate which LED in ring should be light up. Therefore, totalAnimationSteps is the number of executions of this method,
 * upon the LED should move arround the whole ring. 0<= relPosition < totalAnimationSteps encodes the current position within
 * the ring.
 *
 * @param totalAnimationSteps - number of animationsteps until LED should surround the ring
 * @param relPosition -  position within ring 0<= relPosition < totalAnimationSteps
 * @param animation - animation entry (led which should be light up; lightUp - 0 <= lightUp <= 1 (intensity); ledNext - nextLed which should be light up;lightUpNext - 0 <= lightUp <= 1 (intensity)
 */
static void calcLED(uint32_t totalAnimationSteps, uint32_t relPosition, ANIM_LED_t* animation){
    // calculate degree, if one roations consists of anisteps
    float deg = (360.0 * relPosition) / totalAnimationSteps;
    deg = ((deg >= 360) ? (deg - 360.0) : deg);

    // degree per LED
    // 1 LED at 0
    // 2 LED at 1*dst
    // 3 LED at 2*dest etc.
    float dst = 360.0 / LED;

    // calculate indize of leds
    animation->led = (uint32_t) deg/dst;
    animation->led = animation->led % LED;
    animation->ledNext = (animation->led + 1) % LED;

    // ligh up value
    animation->lightUpLED = 1-(deg/dst - animation->led);
    animation->lightUpLEDNext = 1 - animation->lightUpLED;
}


/**
 * @brief max
 * If criterion is fullfilled return max(first, second) else first
 * @param criterion - criterion
 * @param first - first element (standard if criterion not fullfilled)
 * @param second - second element
 * @return If criterion is fullfilled return max(first, second) else first
 */
static float max(bool criterion, float first, float second){
    if(criterion){
        return ((first > second) ? first : second);
    }
    return first;
}

/**
 * @brief mergeColorsForPixel
 * Perform merge for pixel index
 * @param index - pixel for merging
 * @param hours
 * @param minutes
 * @param seconds
 */
static void mergeColorsForPixel(uint16_t index, ANIM_LED_t* hours, ANIM_LED_t* minutes, ANIM_LED_t* seconds){
    // mix colors of elements by their brightness for the element
    float influenceSeconds = max(index == seconds->led, 0,seconds->lightUpLED) + max(index == seconds->ledNext, 0,seconds->lightUpLEDNext);
    float influenceMinutes = max(index == minutes->led, 0,minutes->lightUpLED) + max(index == minutes->ledNext, 0,minutes->lightUpLEDNext);
    float influencehours = max(index == hours->led, 0,hours->lightUpLED) + max(index == hours->ledNext, 0,hours->lightUpLEDNext);

    hsvStripe[index].h = influenceSeconds * BASECOLOR_SECONDS + influenceMinutes * BASECOLOR_MINUTES + influencehours * BASECOLOR_HOURS;
    hsvStripe[index].h = hsvStripe[index].h % 360;

    // brightness -> take maximum
    float brightness = 0;
    brightness = max(index == hours->led, brightness, hours->lightUpLED);
    brightness = max(index == hours->ledNext, brightness, hours->lightUpLEDNext);
    brightness = max(index == minutes->led, brightness, minutes->lightUpLED);
    brightness = max(index == minutes->ledNext, brightness, minutes->lightUpLEDNext);
    brightness = max(index == seconds->led, brightness, seconds->lightUpLED);
    brightness = max(index == seconds->ledNext, brightness, seconds->lightUpLEDNext);

    hsvStripe[index].v = MAX_BRIGHTNESS * brightness;

}

/**
 * @brief mergeColors
 * Merge the single animation colors together, necessary since a led may be adressed by more than one animation
 * at a given time
 * @param hours
 * @param minutes
 * @param seconds
 */
static void mergeColors(ANIM_LED_t* hours, ANIM_LED_t* minutes, ANIM_LED_t* seconds){
    // maybe multiple mixes for same index -> no problem
    mergeColorsForPixel(hours->led, hours, minutes, seconds);
    mergeColorsForPixel(hours->ledNext, hours, minutes, seconds);
    mergeColorsForPixel(minutes->led, hours, minutes, seconds);
    mergeColorsForPixel(minutes->ledNext, hours, minutes, seconds);
    mergeColorsForPixel(seconds->led, hours, minutes, seconds);
    mergeColorsForPixel(seconds->ledNext, hours, minutes, seconds);
}


static void updateVisualization(uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t posInSecond){
    uint32_t aniSteps = RATE_MIN;
    uint32_t clk = seconds * UPDATE_RATE_SEC + posInSecond;


    // Sekundenanimation
    ANIM_LED_t secondAnimation;
    calcLED(aniSteps, clk, &secondAnimation);

    // Minuten-Darstellung
    ANIM_LED_t minutesAnimation;
    aniSteps = RATE_MIN * 60;
    //clk = minutes * RATE_MIN + seconds * UPDATE_RATE_SEC + posInSecond;
    clk = minutes * RATE_MIN + clk;
    calcLED(aniSteps, clk, &minutesAnimation);

    // Stundenanimation
    ANIM_LED_t hoursAnimation;
    aniSteps = RATE_MIN * 60 * 12; // 12 hours display
    clk = (hours % 12) * RATE_MIN * 60 * 12 + clk;
    calcLED(aniSteps, clk, &hoursAnimation);


    // now set output to nothing ;)
    for(uint32_t i = 0; i < LED; i++){
        hsvStripe[i].s = SATURATION;
        hsvStripe[i].h = 0;
        hsvStripe[i].v = 0;
    }

    // merge colors
    mergeColors(&hoursAnimation, &minutesAnimation, &secondAnimation);

    // now convert to rgb value
    for(uint32_t i = 0; i < LED; i++){
        rgbStripe[i] = convertHSV2RGB(&hsvStripe[i]);
    }


    // no draw the fuck hahahahahaha
    WS2812_send(rgbStripe, LED);
}
