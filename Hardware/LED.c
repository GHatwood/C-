#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PA1和PA2引脚初始化为推挽输出
	//默认就是低电平，所以不需要对输出电平进行设置就可以亮灯
	/*设置GPIO初始化后的默认电平*/
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);				
	GPIO_SetBits(GPIOB, GPIO_Pin_13);				
}

//问题：如何将LED1和LED2融合到一个函数里？？？
//void LED_ON(void){
//	GPIO_ResetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2);
//}
//void LED_OFF(void){
//	GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2);
//}

//打开LED1和关闭LED1的函数
void LED_ON(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
}
void LED_OFF(void){
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
}
