#include "stm32f10x.h"
#include "Delay.h"
#include "IIC.h"

#define IIC_Port GPIOB 

#define SCL_Pin GPIO_Pin_8
#define SDA_Pin GPIO_Pin_9

/*引脚配置*/

/**
  * 函    数：I2C写SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SCL的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SCL为低电平，当BitValue为1时，需要置SCL为高电平
  */
void IIC_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(IIC_Port,SCL_Pin,(BitAction)BitValue);
	Delay_ms(1);
}

/**
  * 函    数：I2C写SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SDA的电平，范围0~0xFF
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SDA为低电平，当BitValue非0时，需要置SDA为高电平
  */
void IIC_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(IIC_Port,SDA_Pin,(BitAction)BitValue);
	Delay_ms(1);
}

/**
  * 函    数：I2C读SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前SDA的电平，范围0~1
  * 注意事项：此函数需要用户实现内容，当前SDA为低电平时，返回0，当前SDA为高电平时，返回1
  */
uint8_t IIC_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(IIC_Port,SDA_Pin);
	Delay_ms(1);
	return BitValue;
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，实现SCL和SDA引脚的初始化
  */
void IIC_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//开漏
	GPIO_InitStructure.GPIO_Pin = SCL_Pin | SDA_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IIC_Port,&GPIO_InitStructure);
	
	GPIO_SetBits(IIC_Port,SCL_Pin | SDA_Pin);//默认高电平（释放总线）
	Delay_ms(800);
}

/*协议层*/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void IIC_Start(void)
{
	IIC_W_SCL(1);                                //释放SDA，确保SDA为高电平
	IIC_W_SDA(1);                                //释放SCL，确保SCL为高电平
	IIC_W_SDA(0);                                //在SCL高电平期间，拉低SDA，产生起始信号
	IIC_W_SCL(0);                                //起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void IIC_Stop(void)
{
	IIC_W_SCL(1);                                 //拉低SDA，确保SDA为低电平
	IIC_W_SDA(0);                                 //释放SCL，使SCL呈现高电平
	IIC_W_SDA(1);                                 //在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C等待ACK
  * 参    数：无
  * 返 回 值：无
  */
void IIC_WaitACK(void)
{
	IIC_W_SCL(1);
	IIC_W_SCL(0);
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void IIC_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0;i < 8;i ++)                         //循环8次，主机依次发送数据的每一位
	{                                              
		IIC_W_SDA(Byte & (0X80 >> i));             //使用掩码的方式取出Byte的指定一位数据并写入到SDA线
		IIC_W_SCL(1);                              //释放SCL，从机在SCL高电平期间读取SDA
		IIC_W_SCL(0);                              //拉低SCL，主机开始发送下一位数据
	}
}

/**
  * 函    数：I2C接收一个字节
  * 参    数：无
  * 返 回 值：接收到的一个字节数据，范围：0x00~0xFF
  */
uint8_t IIC_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;                         //定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
	IIC_W_SDA(1);                                   //接收前，主机先确保释放SDA，避免干扰从机的数据发送
	for(i = 0;i < 8;i ++)                           //循环8次，主机依次接收数据的每一位
	{
		IIC_W_SCL(1);                               //释放SCL，主机机在SCL高电平期间读取SDA
		if(IIC_R_SDA() == 1 )                       
		{                                           //读取SDA数据，并存储到Byte变量
			Byte |= (0x80 >> i);                    //当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
		}                                           
		IIC_W_SCL(0);                               //拉低SCL，从机在SCL低电平期间写入SDA
	}
	return Byte;
}

/**
  * 函    数：I2C发送应答位
  * 参    数：Byte 要发送的应答位，范围：0~1，0表示应答，1表示非应答
  * 返 回 值：无
  */
void IIC_SendACK(uint8_t ACKBit)
{
	IIC_W_SDA(ACKBit);                               //主机把应答位数据放到SDA线
	IIC_W_SCL(1);                                    //释放SCL，从机在SCL高电平期间，读取应答位
	IIC_W_SCL(0);                                    //拉低SCL，开始下一个时序模块
}

/**
  * 函    数：I2C接收应答位
  * 参    数：无
  * 返 回 值：接收到的应答位，范围：0~1，0表示应答，1表示非应答
  */
uint8_t IIC_ReceiveACK(void)
{
	uint8_t AckBit;                                   //定义应答位变量
	IIC_W_SDA(1);                                     //接收前，主机先确保释放SDA，避免干扰从机的数据发送
	IIC_W_SCL(1);                                     //释放SCL，主机机在SCL高电平期间读取SDA
	AckBit = IIC_R_SDA();                             //将应答位存储到变量里
	IIC_W_SCL(0);                                     //拉低SCL，开始下一个时序模块
	return AckBit;                                    //返回定义应答位变量
}
