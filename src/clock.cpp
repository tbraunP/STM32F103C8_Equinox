#include "clock.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "hw/uart.h"

#define     UPDATE_RATE         (25)
#define     PRESCALER           (uint16_t) (0x2C1E)
#define     COUNTERVALUE40MS    (uint16_t) (0xFF)

static uint64_t totalDuration = 0;

void Clock_Init(){
    // Timer configuration
    static TIM_TimeBaseInitTypeDef timerConfig;
    static TIM_OCInitTypeDef TIM_OCInitStructure;

    // now we need a timer to decide if we received a bit or not
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&timerConfig);

    /* Compute the prescaler value for 10 KHz -> 10000 * 10 KHz = 1 s */
    uint16_t prescaler = (uint16_t) (SystemCoreClock / 10000) - 1;
    /* Time base configuration */
    timerConfig.TIM_Period = 0xFFFF;
    timerConfig.TIM_Prescaler = PRESCALER-1;
    timerConfig.TIM_ClockDivision = 0;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &timerConfig);
    TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

    /* Compare interrupt */
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    //TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    //TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);

    // set compare value
    TIM4->CCR1 = COUNTERVALUE40MS;

    totalDuration = COUNTERVALUE40MS * UPDATE_RATE;


    // Enable Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    NVIC_ClearPendingIRQ(TIM4_IRQn);
    NVIC_Init(&NVIC_InitStructure);

    // now start the timer
    TIM_Cmd(TIM4, ENABLE);
}


/*
 * Compare Interrupt
 */
void TIM4_IRQHandler(void){
    // clear IRQ Status
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    NVIC_ClearPendingIRQ(TIM2_IRQn);

}
