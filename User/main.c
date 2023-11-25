#include "stm32f10x.h"
#include "Test_Led.h"
#include "Delay.h"
#include "IIC.h"
#include "Serial.h"

uint8_t RxData;			//定义用于接收串口数据的变量

int main(void)
{
	Test_Led_Init();
	IIC_Init();
	Serial_Init();
	while(1)
	{
		//MyGPIO_ToggleBits(GPIOC, GPIO_Pin_13);
		//IIC_SendByte(0x3c);
		if (Serial_GetRxFlag() == 1)			//检查串口接收数据的标志位
		{
			RxData = Serial_GetRxData();		//获取串口接收的数据
			Serial_SendByte(RxData);			//串口将收到的数据回传回去，用于测试
		}
		//Delay_s(4);
	}
}
