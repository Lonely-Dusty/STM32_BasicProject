#include "stm32f10x.h"

#define led_port GPIOC  		//测试LED
#define led_pin GPIO_Pin_13

void Test_Led_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = led_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led_port,&GPIO_InitStructure);
	GPIO_ResetBits(led_port,led_pin);
}

