#include "stm32f10x.h"                  // Device header
void AD_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//ADCCLK的配置
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //72MHz/6=12MHz
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //AIN模式中，GPIO口是无效的。防止对模拟电压进行干扰，相当于ADC的专属引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//选择规则组的输入通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);//在列表中的通道1的位置写入通道0,采样周期是55.5，
	//初始化ADC
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;//单次还是多次，ENABLE：连续，反之
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None; //选择外部配置，本实验不用，用软件进行
	ADC_InitStructure.ADC_Mode =ADC_Mode_Independent;// 选择工作模式
	ADC_InitStructure.ADC_NbrOfChannel=1;//在扫描模式下，会用到几个通道
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;//扫描还是不扫，ENABLE：扫描
	ADC_Init(ADC1, &ADC_InitStructure);
	//中断和模拟看门狗配置，本实验不用，跳过
	
	//开启ADC的电源
	ADC_Cmd(ADC1,ENABLE);
	//校准
  ADC_ResetCalibration(ADC1);  //复位校准
	while(ADC_GetResetCalibrationStatus(ADC1)== SET);//返回复位校准状态，所以如果要等待复位完成的话，需要一个循环
		/*取标志位和是否校准完成的对应关系：软件置该位为1，硬件开始复位校准，校准完后，会由硬件自动清0*/
	ADC_StartCalibration(ADC1); //启动校准
	while(ADC_GetCalibrationStatus(ADC1)== SET);//同理
	
	
}
//启动ADC，获取结果
uint16_t ADC_Get(void){
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//用软件转换
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)== RESET);//获取标志位状态，同理需要一个循环，来进行等待：等于1时，转换完成
	//转换周期是固定的12.5，则总周期=转换周期+采样周期=68个周期；ADCCLK是6分频，就是12MHz，最终时间为：1/12Mx68=5.6us
	//等待5.6us后，取结果
	return ADC_GetConversionValue(ADC1);  //EOC在读取完后会自动清0
	
	
}