#include "stm32f10x.h"   // Device header
#include "stdio.h"
#include "Car.h"
#include "Servo.h"
uint8_t Serial_RxData;
uint8_t Serial_RxFlag;
void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;  //复用推挽
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;//TX
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;  //上拉输入
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;//RX
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);  //中断
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//设置中断优先级
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART1,ENABLE);							//开关控制
	//连接指令GPIO
	

}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);       //写入到TDR寄存器，之后等数据转移到TDR移位寄存器里面，看标志位是否置1
	while(RESET==USART_GetFlagStatus(USART1,USART_FLAG_TXE));//会被自动清零
}

void Serial_SendArray(uint8_t* Array,uint16_t Length)
{

	uint16_t i;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Array[i]);
	}

}

void Serial_SendString(char* String)
{
	uint8_t i;
	for(i=0;String[i]!='\0';i++)
	{
		Serial_SendByte(String[i]);
	}
}
uint32_t Pow(uint32_t x,uint32_t Y)
{
	uint32_t Result=1;
	while(Y--)
	{
		Result*=x;
	
	}
	return Result;

}
void Serial_SendNum(uint32_t Number,uint8_t Length)
{
	uint8_t i=0;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Number/Pow(10,Length-i-1)%10+'0');
	
	}
}
//串口重定向 
int fputc(int ch,FILE* f)
{
	Serial_SendByte(ch);
	return ch;
}

uint8_t Serial_GetRxFlag(void)
{
	if(Serial_RxFlag==1)
	{
		Serial_RxFlag=0;
		return 1;
	}
	return 0;
}

uint8_t Serial_GetRxData(void)
{
	return Serial_RxData;
}



void USART1_IRQHandler(void)   //中断函数
{
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
	{
		Serial_RxData=USART_ReceiveData(USART1);
		Serial_RxFlag=1;
		switch(Serial_RxData)  
			{
				case 'F': Car_Forward(SPEED_MEDIUM); break;
				case 'B': Car_Backward(SPEED_MEDIUM); break;
				case 'L': Car_TurnLeft(SPEED_MEDIUM); break;
				case 'R': Car_TurnRight(SPEED_MEDIUM); break;
				case 'A': Car_SpinLeft(SPEED_MEDIUM); break;
				case 'D': Car_SpinRight(SPEED_MEDIUM); break;
				case 'S': Car_Stop(); break;
				case '1': Car_SetSpeed(SPEED_MEDIUM, SPEED_MEDIUM); break;
				case 'E': Servo_SetAngle(0);   break;
				case 'T': Servo_SetAngle(45);  break;
				case 'G': Servo_SetAngle(90);  break;
				case 'Y': Servo_SetAngle(135); break;
				case 'H': Servo_SetAngle(180); break;
				default: Car_Stop(); break;
			}
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	
	}

}





