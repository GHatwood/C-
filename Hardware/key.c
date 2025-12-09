#include "stm32f10x.h"                  // Device header
#include "Delay.h"
uint8_t Key_Num;
//配置按键功能：key1->PB1——上一项，key2->PA6——下一项，key3->PA4
void Key_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 选择上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 选择上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
// 读取按键的键值
uint8_t Key_GetNum(void) {
  uint8_t Temp;
  if(Key_Num){
    Temp=Key_Num;
    Key_Num=0;
    return Temp;
  }
  else{
    return 0;
  }
}
//bug:当我一直按着按键不放时，会一直触发按键事件，导致实时时钟不走
        //原因：按键按下后没有等待释放就继续执行，所以会一直检测到按键按下
        //Delay_ms(20);  // 消抖
        //所以，我们需要考虑使用其他方式来进行消抖
        //每20ms检测一次按键状态，如果按键状态持续为按下状态，则认为按键有效
        //用定时器来对每20ms进行检测
/*当Key3按下时，Pmos管接地连通，此时VBAT+和VBAT
连通，VBAT+是电池正极，VBAT是LDO稳压芯片的输入
端（输入3.3V给STM32供电），单片机开始工作，使CTL
(PB13)引脚拉高，npn管接通，此时Key3松开，电池也能
给单片机供电；当检测Key按键长按，；就把CTL拉低，
三极管断开，Pmos管电压会被R22上拉电阻拉至高电平，
Pmos管断开，单片机关机。再把CTL引脚置低电平，同
时把ADC对应BAT_ADC_EN (PB1)引脚置高电平。
*/
int press_time;
void Key3_Tick(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
		press_time++;
	}
	
	if((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 1))
	{
		press_time=0;
	}
};

uint8_t Key_GetState(void)
{
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
	{
		return 1;
	}
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0)
	{
		return 2;
	}
	
	else if ((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)&&press_time>1000)
	{
		return 4;
	}
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
		return 3;
	}
	
	else
	{
		return 0;
	}
	
}


void Key_Tick(void){
    static uint8_t Count;
    static uint8_t CurrentState,PreState;
    Count++;
    if(Count>=20){ //每20ms检测一次按键状态
        Count=0;
        PreState=CurrentState;
        CurrentState=Key_GetState();
        if(PreState!=0 && CurrentState==0){ //按键按下事件:如果上一次状态不为0且当前状态为0
            Key_Num=PreState;
        }
    }
}