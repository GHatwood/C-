#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
//在I2C的模块上运行
//宏定义对于的寄存器地址
#define MPU_ADD   0xD0
#include "MPU_Reg.h"
//封装指定地址写和指定地址读的时序
void MPU_WriteReg(uint8_t RegAddress,uint8_t Data){
	MyI2C_Start();
	//发送从机地址，做应答
	MyI2C_SendByte(MPU_ADD);
	MyI2C_ReceiveAck();
	//发送对应的寄存器位置，做应答
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	//发送数据进对应寄存器，做应答
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	
	MyI2C_Stop();
}

uint8_t MPU_ReadingReg(uint8_t RegAddress){
	uint8_t Data;
	
	MyI2C_Start();
	//发送从机地址，做应答
	MyI2C_SendByte(MPU_ADD);
	MyI2C_ReceiveAck();
	//发送对应的寄存器位置，做应答
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	//读的时序，要先重新指定的读写位,所以要重新起始
	MyI2C_Start();
	//传入MPU写的时序
	MyI2C_SendByte(MPU_ADD|0x01);
	MyI2C_ReceiveAck();
	//此时控制权给到从机，由从机进行发送,读后，需要把应答发给从机
	Data=MyI2C_ReceiveByte();
	MyI2C_SendAck(1);//最后一次应答发送1
	
	MyI2C_Stop();
	
	return Data;
}



void MPU6050_Init(void){
	MyI2C_Init();
	MPU_WriteReg(MPU6050_PWR_MGMT_1,0x01);  //解除睡眠,选择陀螺仪时钟
	MPU_WriteReg(MPU6050_PWR_MGMT_2,0x00);	//6个轴均不待机
	MPU_WriteReg(MPU6050_SMPLRT_DIV,0x09);	//采样分频为10
	MPU_WriteReg(MPU6050_CONFIG,0x06);	//滤波参数最大
	MPU_WriteReg(MPU6050_GYRO_CONFIG,0x18);	//陀螺仪和加速度选择最大
	MPU_WriteReg(MPU6050_ACCEL_CONFIG,0x18);
	//此时的MPU就在进行大量的数据转换，数据存放在其他的寄存器里
}
//获取寄存器的数据
//6个返回值：XYZ的加速度值和陀螺仪值：多返回值的设计：1.外部定义6个全局变量；2.指针，进行变量的地址传递(实参传递)；3.用结构体，成员实现参数打包
void MPU_Getdata(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
      						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)  
{
	//读取加速度寄存器XYZ轴的高8位和低8位
	uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = MPU_ReadingReg(MPU6050_ACCEL_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_ACCEL_XOUT_L);		//读取加速度计X轴的低8位数据
	*AccX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU_ReadingReg(MPU6050_ACCEL_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_ACCEL_YOUT_L);		//读取加速度计Y轴的低8位数据
	*AccY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU_ReadingReg(MPU6050_ACCEL_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_ACCEL_ZOUT_L);		//读取加速度计Z轴的低8位数据
	*AccZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU_ReadingReg(MPU6050_GYRO_XOUT_H);		//读取陀螺仪X轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_GYRO_XOUT_L);		//读取陀螺仪X轴的低8位数据
	*GyroX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU_ReadingReg(MPU6050_GYRO_YOUT_H);		//读取陀螺仪Y轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_GYRO_YOUT_L);		//读取陀螺仪Y轴的低8位数据
	*GyroY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU_ReadingReg(MPU6050_GYRO_ZOUT_H);		//读取陀螺仪Z轴的高8位数据
	DataL = MPU_ReadingReg(MPU6050_GYRO_ZOUT_L);		//读取陀螺仪Z轴的低8位数据
	*GyroZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回

	
}
