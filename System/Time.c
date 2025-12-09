#include "stm32f10x.h"                  // Device header
	//1.RCC开启时钟
  //2.选择时基单元的时钟源，我们选择内部时钟源
	//3.配置时基单元，通过结构体配置
	//4.配置输出中断控制，允许更新中断输出到NVIC
	//5.配置NVIC，并打开定时器中断的通道，并分配一个优先级
	//6.运行控制，对定时器进行使能，配置中断函数
void Time_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  //开启APB1的时钟函数，TIM2在APB1总线中
	//选择时基单元
	TIM_InternalClockConfig(TIM2);//系统默认是内部时钟，不写也可以
	//配置时基单元
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; //给输入的滤波器一个采样频率
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;//计数方式
	//因为公式里CK_PSC / (PSC + 1) / (ARR + 1)，所以两个值需要手动减1
	//设置1ms，
	TIM_TimeBaseInitStructure.TIM_Period=100-1;//ARR自动重装器的值
	TIM_TimeBaseInitStructure.TIM_Prescaler=720-1;//预分配器的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;//重复计数器的值，我们用的通用寄存器，所以直接写0就好了
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);//我们发现当我们按下复原键时，定时器是从1开始的，就说明复原的时候，自己走了一次，在此函数中发现它为了让值立刻起作用，所以它自己更新了中断’
	//想要从0开始，就需要把此时中断的标志值清空
	TIM_ClearFlag(TIM2,TIM_IT_Update);
	//使能更新中断
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);//开启了更新中断到NVIC的通路
	//配置NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//配置NVIC的分组通道
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;//匹配NVIC的TIM2通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	//启动定时器
	TIM_Cmd(TIM2, ENABLE);
}
////写中断函数
//void TIM2_IRQHandler(void){
//	//判断标志位是否存在
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
//		Num++;
//		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//	}
//}
//由于我们打算在主文件上使用这个中断函数来改写Num的值，第一个方法：extern Num到Time.c中；第二个方法，把中断函数放到主文件里