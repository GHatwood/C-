 #include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "menu.h"
#include "Key.h"
#include "LED.h"
#include "Time.h"
#include "menu.h"
#include "dino.h"
/**
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */

int main(void)
{
	/*OLED初始化*/
	OLED_Init();
	OLED_Clear();
	Time_Init(); //初始化定时器2，用于按键扫描;
	Peripheral_Init(); //初始化外设
	
	int clkflag;
	extern int press_time;
	extern uint8_t Key_Num;
	while (1)
	{
		// OLED_ShowNum(64,0,press_time,4,OLED_6X8);
		// OLED_ShowNum(64,8,Key_Num,1,OLED_6X8);
		// OLED_Update();
		clkflag=First_Page_Clock();
		if(clkflag==1){
		    //进入菜单界面代码
			Menu();
		}
		else if(clkflag==2){
		    //进入设置界面代码
			SettingPage();
		}

		// Show_Clock_UI(); //显示时钟界面
		// OLED_Update(); //更新OLED显示
		// Delay_ms(500); //延时500ms
		// DinoGame_Animation();
	}
}

void TIM2_IRQHandler(void){
	//判断标志位是否存在
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
	Key3_Tick();
    Key_Tick();
	StopWatch_Tick();
	Dino_Tick();
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}