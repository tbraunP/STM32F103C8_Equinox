#include "dcf77.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "hw/uart.h"

volatile int seconds = 0;

static void EXTI0_Config()
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef timerConfig;

    /* Alternative functions */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure PA0 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // set gpio
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    EXTI_ClearITPendingBit(EXTI_Line0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    //NVIC_Init(&NVIC_InitStructure);


    // now we need a timer to decide if we received a bit or not
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&timerConfig);

    /* Compute the prescaler value for 10 KHz -> 10000 * 10 KHz = 1 s */
    uint16_t prescaler = (uint16_t) (SystemCoreClock / 10000) - 1;
    /* Time base configuration */
    timerConfig.TIM_Period = (10000 - 1);
    timerConfig.TIM_Prescaler = prescaler;
    timerConfig.TIM_ClockDivision = 0;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &timerConfig);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    // Enable Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_Init(&NVIC_InitStructure);

    // now start the timer
    TIM_Cmd(TIM2, ENABLE);
}


void DFC77_init(){
    EXTI0_Config();
}


void EXTI0_IRQHandler(void){
    static int counter = 0;
    ++counter;

    //rising edge
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
        UART_Send((const uint8_t*)"R_DCF\n\0", 6);
    }else{
        UART_Send((const uint8_t*)"F_DCF\n\0", 6);
    }

    EXTI_ClearITPendingBit(EXTI_Line0);
}

void TIM2_IRQHandler(void){
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_ClearPendingIRQ(TIM2_IRQn);


    UART_Send((const uint8_t*)"TIMER\n\0", 6);
}
