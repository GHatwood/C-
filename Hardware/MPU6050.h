#ifndef __MPU6050_H
#define __MPU6050_H

void MPU_WriteReg(uint8_t RegAddress,uint8_t Data);

uint8_t MPU_ReadingReg(uint8_t RegAddress);

void MPU_Getdata(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
      						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);
void MPU6050_Init(void);

#endif
