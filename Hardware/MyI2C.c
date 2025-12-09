#include "stm32f10x.h"                  // Device header
#include "Delay.h"

//对引脚的操作进行优化-用宏或函数都可以，只不过在某些大模块中需要去考虑延时
void MyI2C_W_SCL(uint8_t BitValue){
	GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)BitValue);
	Delay_us(10);
}

void MyI2C_W_SDA(uint8_t BitValue){
	GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)BitValue);
	Delay_us(10);
}

uint8_t MyI2C_R_SDA(void){
	uint8_t BitValue=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
	Delay_us(10);
	return BitValue;
}

	//初始化I2C
void MyI2C_Init(void){
	//软件调用：只需要调用GPIO的读写函数即可
	//1.把SCL和SDA都初始化为开漏输出模式(既可以输出也可输入)
	//2.把SCL和SDA置为高电平
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11);
}
//在通讯中，随着主机SCL的变化，从机也会使SDA的电平进行对应的更改
//时序单元-开始
void MyI2C_Start(void){
	//根据I2C通信的高低电平变化得知：先把SCL和SDA都释放，都输出1
	//之后先拉低SDA，再拉低SCL
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);//优先把SDA上去，兼容后续的重复的判断条件
	
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(0);
}
//终止条件
void MyI2C_Stop(void){
	MyI2C_W_SDA(0);//需要帮SDA先拉于低电平
	//而SCL，在接收应答后默认就是低电平，所以不用拉低
	//
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(1);
}
//除了开始和结束，其他的操作都是让SCL以低电平的方式结束，方便单元的拼接
//发送一个字节
void MyI2C_SendByte(uint8_t Byte){
	uint8_t i;
	for(i=0;i<8;i++){
		MyI2C_W_SDA(Byte&(0x80>>i)); //根据SDA传入的最高位来确定电平
		
		MyI2C_W_SCL(1);
		MyI2C_W_SCL(0);//先释放再读取，是给从机读取之前的在SDA的数据，一段脉冲时间
	}
}

//接收一个字节
//从机在SDA上，需要先把SDA释放，也就是设置为输入模式。当SCL低电平时，从机把数据放到SDA；SCL为高电平时，读取SDA；
//如果此时SDA出现乱动，则会被认为结束或开始的单元
uint8_t MyI2C_ReceiveByte(void){
	uint8_t i, Byte=0x00;
	MyI2C_W_SDA(1);
	//读
	for(i=0;i<8;i++){
		MyI2C_W_SCL(1);
		if(MyI2C_R_SDA()==1){
			Byte|=(0x80>>i);
		}
		MyI2C_W_SCL(0);
	}
	return Byte;
}

//发送应答和接收应答
//针对一个比特
void MyI2C_SendAck(uint8_t AckBit){
	MyI2C_W_SDA(AckBit);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);

}

uint8_t MyI2C_ReceiveAck(void){
	uint8_t AckBit=0x00;
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	AckBit=MyI2C_R_SDA();
	MyI2C_W_SCL(0);
	return AckBit;
}