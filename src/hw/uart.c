#include "stm32f10x_conf.h"
#include "system_stm32f10x.h"
#include "hw/uart.h"

static USART_InitTypeDef USART_InitStruct;


/*
 * UART Config
 *
 * USART1_TX -> PB6
 * USART1_RX -> PB7
 */
void UART_init(){
    GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIOD Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Configure pins in output pushpull mode */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // remap
    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

    // configure UART
    USART_DeInit(USART1);
    USART_StructInit(&USART_InitStruct);
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_Mode = (USART_Mode_Tx);
    //USART_InitStruct.USART_Mode = (USART_Mode_Tx | USART_Mode_Rx);
    USART_Init(USART1, &USART_InitStruct);

    // enable USART
    USART_Cmd(USART1, ENABLE);
}


void UART_Send(const uint8_t* str, uint16_t len){
    for(uint16_t i = 0; i< len; i++){
        // wait until empty
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART1->DR = str[i];
    }
}


void UART_SendString(const char* str){
    while(*str != '\0'){
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART1->DR = *str;
        ++str;
    }
}
