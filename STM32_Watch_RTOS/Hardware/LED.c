#include "stm32f10x.h"                  // Device header

#define BAT_Alarm_Value 20

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_12 );
	GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	}
}

void LED2_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}

void LED2_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}

void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
	}
}

void Bat_LED_Show(uint8_t val)
{
	if(val>=BAT_Alarm_Value)
	{
		LED1_ON();
		LED2_OFF();
	}
	else {
		LED2_ON();
		LED1_OFF();
	}
}

