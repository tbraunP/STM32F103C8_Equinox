#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "stm32f10x_conf.h"
#include "system_stm32f10x.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/uart.h"

static GPIO_InitTypeDef GPIO_InitStructure;

int main(void)
{

      /* GPIOD Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure pins in output pushpull mode */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    while(1)
    {
      GPIO_SetBits(GPIOA, GPIO_Pin_0);
      for (int i = 0; i< 4000;i++);
      GPIO_ResetBits(GPIOA, GPIO_Pin_0);
      for (int i = 0; i< 4000;i++);
    }

    UART_init();
}
