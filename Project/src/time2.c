#include "stm32f10x.h"
#include "stdio.h"
#include "time2.h"

uint32_t TK_Timecount = 0;

void time2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
											
//	TIM_DeInit(TIM2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	  /* Set the default configuration */
	TIM_TimeBaseInitStructure.TIM_Period = 100 -1;  //100ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;  //10 ms
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x0001;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	
	
	 /* Enable and set Button EXTI Interrupt to the lowest priority */
	 NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);	
	 
}

void time2_pro(void)
{
	TK_Timecount += 10;
	//printf("%d\r\n", count);
}
uint32_t GetTimeCount(void)
{
	return TK_Timecount;
}



