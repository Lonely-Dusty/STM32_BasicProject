#include "stm32f10x.h"

#include "Delay.h"
#include "IIC.h"

int main(void)
{
	IIC_Init();
	while(1)
	{
		IIC_SendByte(0x3c);
		Delay_s(4);
	}
}
