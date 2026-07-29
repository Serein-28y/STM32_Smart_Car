#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "HC_SR04.h"

volatile uint16_t HC_SR04_EchoTime;
volatile uint8_t HC_SR04_Flag;

void HC_SR04_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP; //trig
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //echo TIM4_CH1
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);

	TIM_InternalClockConfig(TIM4);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=65536-1;			//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;			//PSC 1us
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter=0x03;
	TIM_ICInitStructure.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM_PWMIConfig(TIM4,&TIM_ICInitStructure);
	
	TIM_SelectInputTrigger(TIM4,TIM_TS_TI1FP1);
	TIM_SelectSlaveMode(TIM4,TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM4,TIM_MasterSlaveMode_Enable);
	TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(TIM4,TIM_IT_CC2);
	TIM_Cmd(TIM4,ENABLE);
}

void HC_SR04_Start(void)
{
	HC_SR04_Flag=0;
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	Delay_us(2);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	Delay_us(20);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

uint8_t HC_SR04_GetDistance(uint16_t *Distance)
{
	if(HC_SR04_Flag==0)
	{
		return 0;
	}
	
	*Distance=HC_SR04_EchoTime/58;
	HC_SR04_Flag=0;
	return 1;
}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_CC2)==SET)
	{
		HC_SR04_EchoTime=TIM_GetCapture2(TIM4);
		HC_SR04_Flag=1;
		TIM_ClearITPendingBit(TIM4,TIM_IT_CC2);
	}
}


