#include "stm32f10x.h"   // Device header
#include "stdio.h"
#include "Car.h"
#define SERIAL_RX_BUFFER_SIZE 16

/* 中断与主循环共享的变量必须使用volatile，防止编译器缓存旧值。 */
volatile uint8_t Serial_RxData;
volatile uint8_t Serial_RxFlag;
static volatile uint8_t Serial_ExitRequested;
static volatile uint8_t Serial_RxBuffer[SERIAL_RX_BUFFER_SIZE];
static volatile uint8_t Serial_RxWriteIndex;
static volatile uint8_t Serial_RxReadIndex;

/**
  * @brief  初始化USART1及接收中断，通信格式为9600-8-N-1。
  * @param  无
  * @retval 无
  */
void Serial_Init(void)
{
	Serial_RxData = 0;
	Serial_RxFlag = 0;
	Serial_ExitRequested = 0;
	Serial_RxWriteIndex = 0;
	Serial_RxReadIndex = 0;

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

/**
  * @brief  通过USART1发送一个字节。
  * @param  Byte 待发送的8位数据。
  * @retval 无
  */
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);       //写入到TDR寄存器，之后等数据转移到TDR移位寄存器里面，看标志位是否置1
	while(RESET==USART_GetFlagStatus(USART1,USART_FLAG_TXE));//会被自动清零
}

/**
  * @brief  通过USART1连续发送指定长度的字节数组。
  * @param  Array 字节数组首地址。
  * @param  Length 需要发送的字节数量。
  * @retval 无
  */
void Serial_SendArray(uint8_t* Array,uint16_t Length)
{

	uint16_t i;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Array[i]);
	}

}

/**
  * @brief  通过USART1发送以'\0'结尾的字符串。
  * @param  String 待发送字符串首地址。
  * @retval 无
  */
void Serial_SendString(char* String)
{
	uint8_t i;
	for(i=0;String[i]!='\0';i++)
	{
		Serial_SendByte(String[i]);
	}
}

/**
  * @brief  计算无符号整数x的Y次方。
  * @param  x 底数。
  * @param  Y 指数。
  * @retval x的Y次方。
  */
uint32_t Pow(uint32_t x,uint32_t Y)
{
	uint32_t Result=1;
	while(Y--)
	{
		Result*=x;
	
	}
	return Result;

}

/**
  * @brief  按固定十进制位数发送无符号整数。
  * @param  Number 待发送的无符号整数。
  * @param  Length 输出位数，不足时在高位补0。
  * @retval 无
  */
void Serial_SendNum(uint32_t Number,uint8_t Length)
{
	uint8_t i=0;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Number/Pow(10,Length-i-1)%10+'0');
	
	}
}

/**
  * @brief  将printf的字符输出重定向到USART1。
  * @param  ch printf输出的字符。
  * @param  f 标准库文件指针，本函数不使用。
  * @retval 已发送的字符。
  */
int fputc(int ch,FILE* f)
{
	Serial_SendByte(ch);
	return ch;
}

/**
  * @brief  查询接收缓冲区中是否还有未处理字节。
  * @param  无
  * @retval 1表示有数据，0表示缓冲区为空。
  */
uint8_t Serial_GetRxFlag(void)
{
	return (Serial_RxReadIndex != Serial_RxWriteIndex);
}

/**
  * @brief  获取USART1最近一次接收到的原始字节。
  * @param  无
  * @retval 最近一次接收到的字节。
  */
uint8_t Serial_GetRxData(void)
{
	return Serial_RxData;
}

/**
  * @brief  从环形接收缓冲区中取出一个待处理字节。
  * @param  data 用于保存取出字节的地址。
  * @retval 1表示成功读取，0表示缓冲区为空。
  */
uint8_t Serial_ReadByte(uint8_t *data)
{
	uint8_t readIndex;

	if(Serial_RxReadIndex == Serial_RxWriteIndex)
	{
		return 0;
	}

	/* 先读取数据再推进读索引；写索引只由中断修改。 */
	readIndex = Serial_RxReadIndex;
	*data = Serial_RxBuffer[readIndex];
	Serial_RxReadIndex = (readIndex + 1) % SERIAL_RX_BUFFER_SIZE;
	Serial_RxFlag = (Serial_RxReadIndex != Serial_RxWriteIndex);
	return 1;
}

/**
  * @brief  丢弃接收缓冲区内尚未处理的所有普通指令。
  * @param  无
  * @retval 无
  */
void Serial_ClearRxBuffer(void)
{
	/* 极短临界区避免清空时USART中断恰好修改写索引。 */
	__disable_irq();
	Serial_RxReadIndex = Serial_RxWriteIndex;
	Serial_RxFlag = 0;
	__enable_irq();
}

/**
  * @brief  查询是否收到全局退出指令X。
  * @param  无
  * @retval 1表示请求退出，0表示继续当前模式。
  */
uint8_t Serial_IsExitRequested(void)
{
	return Serial_ExitRequested;
}

/**
  * @brief  清除全局退出请求，供完成模式退出或进入新模式时调用。
  * @param  无
  * @retval 无
  */
void Serial_ClearExitRequest(void)
{
	__disable_irq();
	Serial_ExitRequested = 0;
	__enable_irq();
}

/**
  * @brief  USART1接收中断：处理全局急停并缓存普通蓝牙指令。
  * @param  无
  * @retval 无
  */
void USART1_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
	{
		uint8_t nextWriteIndex;

		Serial_RxData = (uint8_t)USART_ReceiveData(USART1);
		if(Serial_RxData == 'X' || Serial_RxData == 'x')
		{
			/* X不进入普通缓冲区，即使缓冲区已满也必须立即锁止电机。 */
			Serial_ExitRequested = 1;
			Car_EmergencyStop();
		}
		else
		{
			/* 环形缓冲区预留一个空位置，用读写索引相等表示“空”。 */
			nextWriteIndex = (Serial_RxWriteIndex + 1) % SERIAL_RX_BUFFER_SIZE;
			if(nextWriteIndex != Serial_RxReadIndex)
			{
				Serial_RxBuffer[Serial_RxWriteIndex] = Serial_RxData;
				Serial_RxWriteIndex = nextWriteIndex;
				Serial_RxFlag = 1;
			}
		}
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	
	}

}





