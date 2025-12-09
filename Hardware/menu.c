#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MyRTC.h"                                                                                                                                                                                                     
#include "Key.h"
#include "LED.h"
#include "SetTime.h"
#include "MPU6050.h"
#include "Delay.h"
#include <math.h>
#include "dino.h"
#include "AD.h"

uint8_t KeyNum;

void Peripheral_Init(void)
{
	MyRTC_Init();
	Key_Init();
	LED_Init();
	MPU6050_Init();
	AD_Init();
}



/*----------------------------------首页时钟-------------------------------------*/

//显示电池电量的函数
uint16_t ADValue;
float VBAT;
int Battery_Capacity;
void Show_Battery(void){
	//稳定电压,用ADC来获取稳定电压
	int sum;
	for(int i=0;i<3000;i++){
		ADValue=ADC_Get();
		sum+=ADValue;
	}
	ADValue=sum/3000;	
	VBAT=(float)ADValue/4095*3.3;
	//当电压超过3.3V时，才能稳定输入3.3V的电压，所以以3.3V的电压为0%电量，锂电池302530充满4.1V（为100%）
	//超过3.3V的电压，在ADC检测电池电压中会串联两个4K(或者4.02K)电阻进行分压，对3.3V和4.1V进行分压乘对应分压比，所以ADC输入电压范围在2.64V~3.3V
	//3.3V对应的AD值是4095，所以2.64V对应AD值是2.64/3.3*4095=3276，所以对应的电池电量就会在（AD值-3276）/819*100%
	Battery_Capacity=(ADValue-3276)*100/819;
	//限制一下电量
	if(Battery_Capacity<0)
		Battery_Capacity=0;
	//OLED_ShowNum(64,0,ADValue,4,OLED_6X8);
	//OLED_Printf(64,8,OLED_6X8,"VBAT:%.2f",VBAT);
	OLED_ShowNum(85,4,Battery_Capacity,3,OLED_6X8);
	OLED_ShowChar(103,4,'%',OLED_6X8);
	if(Battery_Capacity>=95)
		OLED_ShowImage(110,0,16,16,Battery);
	else if(Battery_Capacity>=10&&Battery_Capacity<95){
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea((112+Battery_Capacity/10),5,(10-Battery_Capacity/10),6);
		OLED_ClearArea(85,4,6,8);
	}
	else{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea(112,5,10,6);
		OLED_ClearArea(85,4,12,8);
	}
}

//短按开机，长按关机

void Show_Clock_UI(void)
{
	Show_Battery();
	MyRTC_ReadTime();
	OLED_Printf(0,0,OLED_6X8,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
	OLED_Printf(16,16,OLED_12X24,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
	OLED_ShowString(0,48,"菜单",OLED_8X16);
	OLED_ShowString(96,48,"设置",OLED_8X16);
}

int clkflag=1;

int First_Page_Clock(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();

		if(KeyNum==1)//上一项
		{
			clkflag--;
			if(clkflag<=0)clkflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			clkflag++;
			if(clkflag>=3)clkflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			return clkflag;
		}
		else if(KeyNum==4){
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);				
			GPIO_SetBits(GPIOB, GPIO_Pin_12);	
		}
		
		switch(clkflag)
		{
			case 1:
				Show_Clock_UI();
				OLED_ReverseArea(0,48,32,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Clock_UI();
				OLED_ReverseArea(96,48,32,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------设置界面-------------------------------------*/

void Show_SettingPage_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"日期时间设置",OLED_8X16);
}

int setflag=1;
int SettingPage(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t setflag_temp=0;
		if(KeyNum==1)//上一项
		{
			setflag--;
			if(setflag<=0)setflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			setflag++;
			if(setflag>=3)setflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			setflag_temp=setflag;
		}
		
		if(setflag_temp==1){return 0;}
		else if(setflag_temp==2){SetTime();}
		
		switch(setflag)
		{
			case 1:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_SettingPage_UI();
				OLED_ReverseArea(0,16,96,16);
				OLED_Update();
				break;
		}
	}
}

/*----------------------------------滑动菜单界面-------------------------------------*/
//滑动原理：当按键按下时，改变menuflag的值，从而改变显示的图案，让三个图标一起向左或者向右进行移动
//			最终停留在选型框里的选项由上次选择的选项变成这次选择的选项		

//pre_selection和target_selection指向Menu_Graph数组中图像的索引值
uint8_t pre_selection;//上次选择的选项
uint8_t target_selection;//本次选择的选项

uint8_t x_pre=48;//上次选择的选项的X坐标,因为 OLED的屏幕大小长64，图像是32，总体一半是16，所以48是居中的位置
uint8_t Speed=4;//移动速度
uint8_t move_flag;//移动标志位，0表示不移动，1表示移动

//菜单滑动动画
void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage(42,10,44,44,Frame);
	if(pre_selection<target_selection){
		x_pre-=Speed;
		if(x_pre<=16){
			x_pre=48;
			pre_selection++;
			move_flag=0;
		}

	}
	if(pre_selection>target_selection){
		x_pre+=Speed;
		if(x_pre>=80){
			x_pre=48;
			pre_selection--;
			move_flag=0;
		}
	}
	//显示图像,需要考虑当pre_selection为0或者最大值时，避免数组越界
	if(pre_selection>=1){
		OLED_ShowImage(x_pre-48,16,32,32,Menu_Graph[pre_selection-1]);
	}
	//显示图像，需要考虑当图标向右移动时，同时移动图标时数据越界
	if(pre_selection>=2){
		OLED_ShowImage(x_pre-96,16,32,32,Menu_Graph[pre_selection-2]);
	}

	OLED_ShowImage(x_pre,16,32,32,Menu_Graph[pre_selection]);
	OLED_ShowImage(x_pre+48,16,32,32,Menu_Graph[pre_selection+1]);
	OLED_ShowImage(x_pre+96,16,32,32,Menu_Graph[pre_selection+2]);
	OLED_Update();
}
//用按键控制pre_selection和tar_selection的变化，从而实现滑动菜单
void Set_Selection(uint8_t move_flag,uint8_t Pre_selection,uint8_t Target_selection)
{
	if(move_flag==1){//向左滑动
		pre_selection=Pre_selection;
		target_selection=Target_selection;	
		
	}
	Menu_Animation();
}

//转场函数
void MenuToFunction(void)
{
	for(uint8_t i=0;i<=6;i++)
	{
		OLED_Clear();
			if(pre_selection>=1)
		{
			OLED_ShowImage(x_pre-48,16+8*i,32,32,Menu_Graph[pre_selection-1]);
		}
		
		
		OLED_ShowImage(x_pre,16+8*i,32,32,Menu_Graph[pre_selection]);
		OLED_ShowImage(x_pre+48,16+8*i,32,32,Menu_Graph[pre_selection+1]);
		
		OLED_Update();
	}
	
}
int menuflag=1;
int Menu(void)
{
	//来标识上一项和下一项的方向位
	uint8_t DirectFlag=2;//置1时，表示移动到上一项；置2时，表示移动到下一项
	move_flag=1;//初始化为移动状态，进入菜单界面时，显示第一个图标在中间位置
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t  menuflag_temp=0;
		if(KeyNum==1)//上一项
		{
			DirectFlag=1;
			move_flag=1;
			menuflag--;
			if(menuflag<=0)menuflag=7;
		}
		else if(KeyNum==2)//下一项
		{
			DirectFlag=2;
			move_flag=1;
			menuflag++;
			if(menuflag>=8)menuflag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			menuflag_temp=menuflag;
		}
		
		if(menuflag_temp==1){return 0;}
		else if(menuflag_temp==2){MenuToFunction();
			StopWatch();}
		else if(menuflag_temp==3){MenuToFunction();
			LED();}
		else if(menuflag_temp==4){MenuToFunction();
			MPU6050();}
		else if(menuflag_temp==5){MenuToFunction();
			Game();}
		else if(menuflag_temp==6){MenuToFunction();
			Emoji();}
		else if(menuflag_temp==7){MenuToFunction();
			Gradienter();}
		//滑动菜单中选择对应的功能：退回键、秒表、手电筒、MPU6050、游戏、小电视、水平仪
		// switch(menuflag)
		// {
			// case 1://退回键
			// 	if(DirectFlag==1){//向上移动
			// 		Set_Selection(move_flag,1,0);
			// 	}
			// 	else if(DirectFlag==2){//向下移动
			// 		Set_Selection(move_flag,0,0);
			// 	}
			// 	OLED_Update();
			// 	break;
			
			// case 2://秒表
			// 	if(DirectFlag==1){//向上移动
			// 		Set_Selection(move_flag,2,1);
			// 	}
			// 	else if(DirectFlag==2){//向下移动
			// 		Set_Selection(move_flag,0,1);
			// 	}
			// 	OLED_Update();
			// 	break;

			// case 3://手电筒
			// 	if(DirectFlag==1){//向上移动
			// 		Set_Selection(move_flag,3,2);
			// 	}
			// 	else if(DirectFlag==2){//向下移动
			// 		Set_Selection(move_flag,1,2);
			// 	}
			// 	OLED_Update();
			// 	break;

			// case 4://MPU6050
			// 		if(DirectFlag==1){//向上移动
			// 			Set_Selection(move_flag,1,0);
			// 		}
			// 		else if(DirectFlag==2){//向下移动
			// 			Set_Selection(move_flag,0,0);
			// 		}
			// 	OLED_Update();
			// 	break;

			// case 5:			//游戏
			// 	OLED_Update();
			// 	break;
		
			// case 6://小电视
				
			// 	OLED_Update();
			// 	break;
		
			// case 7://水平仪
				
			// 	OLED_Update();
			// 	break;	
		//}
		if (menuflag==1)
		{
			/* code */
			if(DirectFlag==1){//向上移动
					Set_Selection(move_flag,1,0);
				}
			else if(DirectFlag==2){//向下移动
				Set_Selection(move_flag,0,0);
			}

		}
		else {
			if(DirectFlag==1){//向上移动
				Set_Selection(move_flag,menuflag,menuflag-1);
			}
			else if(DirectFlag==2){//向下移动
				Set_Selection(move_flag,menuflag-2,menuflag-1);
			}
		}
	}
}

/*----------------------------------秒表-------------------------------------*/
//显示秒表UI
uint8_t hour,min,sec;
void Show_StopWatch_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(28,20,OLED_8X16,"%02d:%02d:%02d",hour,min,sec);
	OLED_ShowString(8,44,"开始",OLED_8X16);
	OLED_ShowString(48,44,"结束",OLED_8X16);
	OLED_ShowString(88,44,"清除",OLED_8X16);
}

//用Tim2作为秒表计时的基础，每秒中断一次，进行时间的累加,对tim2进行1000分频
uint8_t start_timing_flag;//开始计时标志位，0表示未开始计时，1表示正在计时
void StopWatch_Tick(void){
	static uint16_t Count;
	Count++;
	if (Count>=1000)
	{
		/* code */
		Count=0;
		if(start_timing_flag==1){
		sec++;
		if(sec>=60){
			sec=0;
			min++;
			if(min>=60){
				min=0;
				hour++;
				if(hour>=100){
					hour=0;
				}
			}
		}
		}
	}
	
	
}

uint8_t stopWatch_flag=1;
int StopWatch(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t stopWatch_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			stopWatch_flag--;
			if(stopWatch_flag<=0)stopWatch_flag=4;
		}
		else if(KeyNum==2)//下一项
		{
			stopWatch_flag++;
			if(stopWatch_flag>=5)stopWatch_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			stopWatch_flag_temp=stopWatch_flag;
		}
		
		//进入秒表页面，无下一个功能所以不要分条件，当确认时直接返回主菜单
		if(stopWatch_flag_temp==1){return 0;}
		
		switch(stopWatch_flag)
		{
			case 1://退回
				Show_StopWatch_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2://开始
				Show_StopWatch_UI();
				start_timing_flag=1;
				OLED_ReverseArea(8,44,32,16);
				OLED_Update();
				break;

			case 3://结束
				Show_StopWatch_UI();
				start_timing_flag=0;
				OLED_ReverseArea(48,44,32,16);
				OLED_Update();
				break;
			
			case 4://清除
				Show_StopWatch_UI();
				start_timing_flag=0;
				hour=0,min=0,sec=0;
				OLED_ReverseArea(88,44,32,16);
				OLED_Update();
				break;
		}
	}
}
/*----------------------------------手电筒-------------------------------------*/
void Show_LED_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(20,20,"OFF",OLED_12X24);
	OLED_ShowString(72,20,"ON",OLED_12X24);
}
uint8_t led_flag=1;
int LED(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t led_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			led_flag--;
			if(led_flag<=0)led_flag=3;
		}
		else if(KeyNum==2)//下一项
		{
			led_flag++;
			if(led_flag>=4)led_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			led_flag_temp=led_flag;
		}
		
		//进入秒表页面，无下一个功能所以不要分条件，当确认时直接返回主菜单
		if(led_flag_temp==1){return 0;}
		
		switch(led_flag)
		{
			case 1://退回
				Show_LED_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2://熄灯
				Show_LED_UI();
				LED_OFF();
				OLED_ReverseArea(20,20,36,24);
				OLED_Update();
				break;

			case 3://亮灯
				Show_LED_UI();
				LED_ON();
				OLED_ReverseArea(72,20,24,24);
				OLED_Update();
				break;
			
		}
	}
}

/*----------------------------------MPU6050-------------------------------------*/

//读取MPU6050数据进行姿态解算
int16_t ax,ay,az,gx,gy,gz;//MPU6050测得的三轴加速度和角速度
float roll_g,pitch_g,yaw_g;//陀螺仪解算的欧拉角
float roll_a,pitch_a;//加速度计解算的欧拉角
float Roll,Pitch,Yaw;//互补滤波后的欧拉角
float a=0.9;//互补滤波器系数
float Delta_t=0.005;//采样周期
double pi=3.1415927;

void MPU6050_Calculation(void)
{
	Delay_ms(5);
	MPU_Getdata(&ax,&ay,&az,&gx,&gy,&gz);
	
	//通过陀螺仪解算欧拉角
	roll_g=Roll+(float)gx*Delta_t;
	pitch_g=Pitch+(float)gy*Delta_t;
	yaw_g=Yaw+(float)gz*Delta_t;
	
	//通过加速度计解算欧拉角
	pitch_a=atan2((-1)*ax,az)*180/pi;
	roll_a=atan2(ay,az)*180/pi;
	
	//通过互补滤波器进行数据融合
	Roll=a*roll_g+(1-a)*roll_a;
	Pitch=a*pitch_g+(1-a)*pitch_a;
	Yaw=a*yaw_g;
	
}

//显示MPU6050 UI
void Show_MPU6050_UI(void){
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(0,16,OLED_8X16,"Roll:%.2f",Roll);
	OLED_Printf(0,32,OLED_8X16,"Pitch:%.2f",Pitch);
	OLED_Printf(0,48,OLED_8X16,"Yaw:%.2f",Yaw);
}

//主函数
int MPU6050(void){
	while(1){
		KeyNum=Key_GetNum();
		if(KeyNum==3){//确认键返回主菜单
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0,0,16,16);
		OLED_Update();
	}
}

/*----------------------------------游戏-------------------------------------*/
void Show_Game_UI(void){
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"小胡小恐龙",OLED_8X16);
}
uint8_t game_flag=1;
int Game(void){
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t game_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			game_flag_temp--;
			if(game_flag<=0)game_flag=2;
		}
		else if(KeyNum==2)//下一项
		{
			game_flag++;
			if(game_flag>=3)game_flag=1;
		}
		else if(KeyNum==3)//确认
		{
			OLED_Clear();
			OLED_Update();
			game_flag_temp=game_flag;
		}
		
		//进入秒表页面，无下一个功能所以不要分条件，当确认时直接返回主菜单
		if(game_flag_temp==1){return 0;}
		else if(game_flag_temp==2){
			DinoGame_Pos_Init();
			DinoGame_Animation();
		}

		switch(game_flag)
		{
			case 1:
				Show_Game_UI();
				OLED_ReverseArea(0,0,16,16);
				OLED_Update();
				break;
			
			case 2:
				Show_Game_UI();
				OLED_ReverseArea(0,16,80,16);
				OLED_Update();
				break;

			
			
		}
	}
}

/*----------------------------------小电视-------------------------------------*/
//睁眼闭眼是眼睛纵向长度的变化——圆变椭圆
void Show_Emoji_UI(void){
	//闭眼
	for(uint8_t i=0;i<3;i++){
		OLED_Clear();
		//眉毛
		OLED_ShowImage(30,10+i,16,16,Eyebrow[0]);
		OLED_ShowImage(82,10+i,16,16,Eyebrow[1]);
		//眼睛
		OLED_DrawEllipse(40,32,6,6-i,1);//左眼
		OLED_DrawEllipse(88,32,6,6-i,1);//右眼
		//嘴巴
		OLED_ShowImage(54,40,20,20,Month);
		OLED_Update();
		Delay_ms(100);
	}
	//睁眼
	for(uint8_t i=0;i<3;i++){
		OLED_Clear();
		//眉毛
		OLED_ShowImage(30,12-i,16,16,Eyebrow[0]);
		OLED_ShowImage(82,12-i,16,16,Eyebrow[1]);
		//眼睛
		OLED_DrawEllipse(40,32,6,4+i,1);//左眼
		OLED_DrawEllipse(88,32,6,4+i,1);//右眼
		//嘴巴
		OLED_ShowImage(54,40,20,20,Month);
		OLED_Update();
		Delay_ms(100);
	}
	Delay_ms(500);
}
int Emoji(void){
	while(1){
		KeyNum=Key_GetNum();
		if(KeyNum==3){//确认键返回主菜单
			OLED_Clear();
			OLED_Update();
			return 0;
		}

		Show_Emoji_UI();

	}
}

/*----------------------------------水平仪-------------------------------------*/
void Show_Gradienter_UI(void){
	MPU6050_Calculation();
	OLED_DrawCircle(64,32,30,0);
	OLED_DrawCircle(64-Roll,32+Pitch,4,1);
}
int Gradienter(void){
	while(1){
		KeyNum=Key_GetNum();
		if(KeyNum==3){//确认键返回主菜单
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		Show_Gradienter_UI();
		OLED_Update();
	}
}