/* MAIN.C file
 * 
 * Copyright (c) 2002-2005 STMicroelectronics
 */

//PA3 SYSTEM SWITCH

#include "stm8l15x.h"
#include "stm8l15x_it.h"
#include "main.h"

//#define USE_LSE 1					//USE LSE FOR RTC CLOCK
#define HALT_STM8L 1

/* Private variables ---------------------------------------------------------*/
RTC_InitTypeDef   RTC_InitStr;
RTC_TimeTypeDef   RTC_TimeStr;
RTC_DateTypeDef   RTC_DateStr;
RTC_AlarmTypeDef  RTC_AlarmStr;

uint8_t wake_me_up = 0;
uint8_t CLK_Edge_detected = 0;
uint8_t byte_read = 0;
uint8_t command[16];
uint8_t send_buffer[16]={0x05,0x05,0x05,0x05,0x05,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t byte_count=0;
uint8_t bytes_to_read=0;
uint8_t writing = 1;
uint8_t BAT[2];
uint16_t Bat_Vltg;
uint16_t temp_long;
uint8_t temp_short;
uint8_t bit_count=0;
uint16_t ADCData=0;
uint8_t temp=0;
uint8_t glitch = 0;
uint8_t calib_on = 0;
uint16_t RTC_calib = 16384;
uint32_t calib_temp;


static void Delay_ms(uint8_t del_ms)
{
	int i,j,k;

	volatile int c=0;
	for (i=0; i<64;i++)
		for (j=0; j<14;j++)
			for (k=0; k<del_ms;k++)
				c++;
}

static void Delay_10ms(uint8_t del_ms)
{
	int i,j,k;

	volatile int c=0;
	for (i=0; i<64;i++)
		for (j=0; j<140;j++)
			for (k=0; k<del_ms;k++)
				c++;
}

/**
  * @brief  Calendar Configuration.
  * @param  None
  * @retval None
  */
void Calendar_Init(void)
{
  
	RTC_InitStr.RTC_HourFormat = RTC_HourFormat_24;
  RTC_InitStr.RTC_AsynchPrediv = 0x01;
  RTC_InitStr.RTC_SynchPrediv = RTC_calib;
  RTC_Init(&RTC_InitStr);

  RTC_DateStructInit(&RTC_DateStr);
  RTC_DateStr.RTC_WeekDay = command[1];
  RTC_DateStr.RTC_Date = command[2];
  RTC_DateStr.RTC_Month = command[3];
  RTC_DateStr.RTC_Year = command[4];
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStr);

  RTC_TimeStructInit(&RTC_TimeStr);
  RTC_TimeStr.RTC_Hours   = command[5];
  RTC_TimeStr.RTC_Minutes = command[6];
  RTC_TimeStr.RTC_Seconds = command[7];
  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStr);
}

void Calendar_Read(void)
{
	RTC_GetDate(RTC_Format_BIN,&RTC_DateStr);
	send_buffer[0]=RTC_DateStr.RTC_WeekDay;
	send_buffer[1]=RTC_DateStr.RTC_Date;
	send_buffer[2]=RTC_DateStr.RTC_Month;
	send_buffer[3]=RTC_DateStr.RTC_Year;
	
	RTC_GetTime(RTC_Format_BIN,&RTC_TimeStr);
	send_buffer[4]=RTC_TimeStr.RTC_Hours;
	send_buffer[5]=RTC_TimeStr.RTC_Minutes;
	send_buffer[6]=RTC_TimeStr.RTC_Seconds;
}

void Alarm_Init(void)
{
	RTC_AlarmCmd(DISABLE);
	
	RTC_AlarmStructInit(&RTC_AlarmStr);
  RTC_AlarmStr.RTC_AlarmTime.RTC_Hours   = command[1];
  RTC_AlarmStr.RTC_AlarmTime.RTC_Minutes = command[2];
  RTC_AlarmStr.RTC_AlarmTime.RTC_Seconds = command[3];
	RTC_AlarmStr.RTC_AlarmDateWeekDaySel = command[4]; 
	RTC_AlarmStr.RTC_AlarmDateWeekDay = command[5];
  RTC_AlarmStr.RTC_AlarmMask = command[6];
  RTC_SetAlarm(RTC_Format_BIN, &RTC_AlarmStr);

	RTC_ClearITPendingBit(RTC_IT_ALRA);
	RTC_ClearITPendingBit(RTC_IT_WUT);
	RTC_ClearITPendingBit(RTC_IT_TAMP);
	
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
  RTC_AlarmCmd(ENABLE);
	enableInterrupts();
	
}

void Alarm_Read(void)
{
	RTC_GetAlarm(RTC_Format_BIN,&RTC_AlarmStr);
	send_buffer[0]=RTC_AlarmStr.RTC_AlarmTime.RTC_Hours;
	send_buffer[1]=RTC_AlarmStr.RTC_AlarmTime.RTC_Minutes;
	send_buffer[2]=RTC_AlarmStr.RTC_AlarmTime.RTC_Seconds;
	send_buffer[3]=RTC_AlarmStr.RTC_AlarmDateWeekDaySel;
	send_buffer[4]=RTC_AlarmStr.RTC_AlarmDateWeekDay;
	send_buffer[5]=RTC_AlarmStr.RTC_AlarmMask;
}

static void RTC_Config(void)
{

	#ifdef USE_LSE
		/* Enable RTC clock */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
		/* Wait for LSI clock to be ready */
		while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET);
	
		/* Select LSI (32.768 KHz) as RTC clock source */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
	
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#else
		/* Enable RTC LSE clock */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
		/* Wait for LSI clock to be ready */
		while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
		/* wait for 1 second for the LSE Stabilisation */
	
		/* Select LSE (32.768 KHz) as RTC clock source */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
	
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#endif

}

static void RTC_Config_WakeUp(uint8_t RTC_WakeUpClock)
{
	assert_param(IS_RTC_WAKEUP_CLOCK(RTC_WakeUpClock));
  #ifdef USE_LSE
		/* Enable RTC clock */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
		/* Wait for LSI clock to be ready */
		while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET);
	
		/* Select LSI (32.768 KHz) as RTC clock source */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
	
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#else
		/* Enable RTC LSE clock */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
		/* Wait for LSI clock to be ready */
		while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
		/* wait for 1 second for the LSE Stabilisation */
	
		/* Select LSE (32.768 KHz) as RTC clock source */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
	
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#endif

  /* Configures the RTC wakeup timer_step*/
  RTC_WakeUpClockConfig(RTC_WakeUpClock);

  disableInterrupts();
  /* Enable wake up unit Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);

  /* Enable general Interrupt*/
  enableInterrupts();
}

uint16_t ADC_Read(void)
{
	uint16_t ADC_data;
	/* Enable ADC1 clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);

	 /* Initialize and configure ADC1 */
   ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_2);
	 /* ADC channel used for IDD measurement */
		ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_384Cycles);

		/* Enable ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Disable SchmittTrigger for ADC_Channel, to save power */
		ADC_SchmittTriggerConfig(ADC1, ADC_Channel_12, DISABLE);

		/* Enable ADC1 Channel used for measurement */
		ADC_ChannelCmd(ADC1, ADC_Channel_12, ENABLE);

		/* Start ADC1 Conversion using Software trigger*/
		ADC_SoftwareStartConv(ADC1);


		/* Wait until End-Of-Convertion */
		while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0)
		{}

		ADC_data = ADC_GetConversionValue(ADC1);
		
		ADC_DeInit(ADC1);
		
		CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
		
		/* Get conversion value */
		return(ADC_data);
}

void Power_SW_Toggle(uint8_t value)
{
	if(value==1)
		GPIO_SetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
	else
		GPIO_ResetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
}

void fetch_command(void)
{
	uint16_t WU_counter;
	switch(command[0])
	{
		//WRITE COMMANDS
		case Set_Calendar:
			Calendar_Init();
			break;
		case Set_Alarm:
			Alarm_Init();
			break;
		case Set_Wake_Up: 
			RTC_Config_WakeUp(command[1]>>2);
			calib_temp = ((uint16_t)command[2]<<8) + (uint16_t)command[3] + ((((uint16_t)command[1])&0x02)<<7) + (((uint16_t)command[1])&0x01);
			calib_temp = calib_temp*RTC_calib;
			WU_counter = calib_temp/16384;
			RTC_SetWakeUpCounter(WU_counter);		
			RTC_WakeUpCmd(ENABLE);
			break;
		case Set_Year: break;
		case Set_Month: break;
		case Set_Day: break;
		case Set_WD: break;
		case Set_Hour: break;
		case Set_Minute: break;
		case Set_Seconds: break;
		case Set_SubSeconds: break;
		case Read_Battery:
			Delay_10ms(2);
			Bat_Vltg=ADC_Read();
			//RESTORE CLK AS INPUT WITH IT
			disableInterrupts();
			GPIO_Init(GPIO_CLK_PORT, GPIO_CLK_PIN, GPIO_Mode_In_FL_IT);
			EXTI_SetPinSensitivity(EXTI_Pin_6,EXTI_Trigger_Rising);
			Delay_10ms(10);
			enableInterrupts();
			temp_long=Bat_Vltg&0xFF;
			BAT[1]=(uint8_t)(temp_long);
			temp_long=Bat_Vltg>>8;
			BAT[0]=temp_long;
			glitch=1;
			break;
		case Shutdown_Now: 
			// SHUTDOWN GAP
			GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_In_FL_No_IT);
			GPIO_ResetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
			#ifdef HALT_STM8L
				halt();
			#endif
			break;
		case WakeUp_Rising_Edge:
			disableInterrupts();
			GPIO_Init(GPIO_WAKEUP_PORT, GPIO_WAKEUP_PIN, GPIO_Mode_In_FL_IT);
			EXTI_SetPinSensitivity(EXTI_Pin_3,EXTI_Trigger_Rising);
			enableInterrupts();
			break;
		case WakeUp_Falling_Edge:
			disableInterrupts();
			GPIO_Init(GPIO_WAKEUP_PORT, GPIO_WAKEUP_PIN, GPIO_Mode_In_FL_IT);
			EXTI_SetPinSensitivity(EXTI_Pin_3,EXTI_Trigger_Falling);
			enableInterrupts();
		break;
		case Reset_Interface:
			disableInterrupts();
		  while (GPIO_ReadInputDataBit(GPIO_DATA_PORT, GPIO_DATA_PIN)==1);
			enableInterrupts();
		break;
		case Set_CalibRoutine:
			RTC_Config_WakeUp(command[1]>>2);
			WU_counter = ((uint16_t)command[2]<<8) + (uint16_t)command[3] + ((((uint16_t)command[1])&0x02)<<7) + (((uint16_t)command[1])&0x01);
			RTC_SetWakeUpCounter(WU_counter);		
		break;
		case Start_CalibRoutine:
			calib_on=1;
			GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_Out_PP_High_Fast);
			RTC_WakeUpCmd(ENABLE);
			GPIO_ResetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
		break;
		case Set_CalibData:
			RTC_calib = ((uint16_t)command[2]<<8) + (uint16_t)command[3] + ((((uint16_t)command[1])&0x02)<<7) + (((uint16_t)command[1])&0x01);
			RTC_InitStr.RTC_HourFormat = RTC_HourFormat_24;
			RTC_InitStr.RTC_AsynchPrediv = 0x01;
			RTC_InitStr.RTC_SynchPrediv = RTC_calib;
			RTC_Init(&RTC_InitStr);
		break;
		
		//READ COMMANDS
		case Get_Calendar:
			Calendar_Read();
			bytes_to_read=7;
			GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_Out_PP_High_Fast);
			writing=0;
			break;
		case Get_Alarm: 
			Alarm_Read();
			bytes_to_read=6;
			GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_Out_PP_High_Fast);
			writing=0;
			break;
		case Get_Wake_Up: break;
		case Get_Year: break;
		case Get_Month: break;
		case Get_Day: break;
		case Get_WD: break;
		case Get_Hour: break;
		case Get_Minute: break;
		case Get_Seconds: break;
		case Get_SubSeconds: break;
		case Get_Battery_Voltage:
			bytes_to_read=2;
			send_buffer[0]=BAT[0];
			send_buffer[1]=BAT[1];
			GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_Out_PP_High_Fast);
			writing=0;
		break;
	}
}

void fetch_byte(void)
{
	if((byte_read==0xFF)&&(byte_count!=0))
	{
		fetch_command();
		byte_count=0;
	}
	else
		command[byte_count++]=byte_read;	
}


/**
  * @brief  Configure System Clock 
  * @param  None
  * @retval None
  */
static void CLK_Config(void)
{
  /* Select HSE as system clock source */
  CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
  /* system clock prescaler: 1*/
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
  {}

}

static void RTC_CLK_Config(void)
{
  #ifdef USE_LSE
		/* Select LSI (32.768 KHz) as RTC clock source */
		CLK_RTCCLKSwitchOnLSEFailureEnable(); //SWITCH TO LSI IN CASE OF LSE FAILURE
		
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
		
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#else
			/* Select LSI (32.768 KHz) as RTC clock source */

		CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
		
		CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
	#endif
}




main()
{
	CLK_Config();
	 RTC_CLK_Config();
	 PWR_UltraLowPowerCmd(ENABLE);
	 
	 GPIO_Init(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN, GPIO_Mode_In_FL_No_IT);
  // CHECK IF SYS_SWITCH IS FORCED LOW AND WAIT THERE INDEFINITELY (SAFETY MEASURE IN CASE ONE MAKES A MESS WITH E.G. SLEEP)
	while (GPIO_ReadInputDataBit(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN==0));
	
	// INIT SWITCH WITH GAP ON
	GPIO_Init(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN, GPIO_Mode_Out_PP_High_Fast);
	GPIO_SetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
  
//FOR TESTING- TO BE REMOVED
  // SET INTERFACE PINS AS INPUTS WITH INTERRUPT ON CLOCK
	//GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_Out_PP_High_Fast);
  
	/*while(1){
		GPIO_SetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
		Delay_10ms(2);
		GPIO_ResetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
		Delay_10ms(2);			
		}*/
		
		
		
		
	Delay_10ms(200); //DELAY 2 SEC
	
	#ifdef USE_LSE
		//DISABLE SWIM AND ENABLE LSE
		GPIO_Init(GPIOA, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);
		CLK_LSEConfig(CLK_LSE_ON);
	#endif

	//INIT SWITCH WITH GAP ON
	GPIO_Init(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_SetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
	
	// SET INTERFACE PINS AS INPUTS WITH INTERRUPT ON CLOCK
	GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_In_FL_No_IT);
  //GPIO_SetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
	GPIO_Init(GPIO_CLK_PORT, GPIO_CLK_PIN, GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_Pin_6,EXTI_Trigger_Rising);
  //GPIO_SetBits(GPIO_CLK_PORT, GPIO_CLK_PIN);
	
	// INIT SWITCH WITH GAP ON
  GPIO_SetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
	
	// CONFIG RTC
	//RTC_Config();
	/* Enable general Interrupt*/
  enableInterrupts();

			
	while (1)
	{
		//RTC_GetTime(RTC_Format_BCD, &RTC_TimeStr);
		if(wake_me_up==1)
		{
			wake_me_up=0;
			GPIO_SetBits(GPIO_SYS_SW_PORT, GPIO_SYS_SW_PIN);
			if(calib_on==1){
				calib_on=0;
				GPIO_SetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
				Delay_ms(1);
				GPIO_ResetBits(GPIO_DATA_PORT, GPIO_DATA_PIN);
				GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_In_FL_No_IT);
			}
		}
		if(glitch==1)
		{
			CLK_Edge_detected=0;
			glitch = 0;
		}
			
		if(CLK_Edge_detected==1)
		{
			CLK_Edge_detected = 0;
			
			if(writing)
			{
				byte_read=(byte_read<<1)+(GPIO_ReadInputDataBit(GPIO_DATA_PORT, GPIO_DATA_PIN)>>7);
				bit_count++;
				if (bit_count==8)
				{
					bit_count = 0;
					fetch_byte();
				}
			}
			else
			{
				GPIO_WriteBit(GPIO_DATA_PORT, GPIO_DATA_PIN,((send_buffer[byte_count])>>(7-bit_count)&0x01));
				bit_count++;
				if (bit_count==8)
				{
					byte_count++;
					bit_count = 0;
				}
				if(byte_count==bytes_to_read)
				{
					byte_count = 0;
					writing=1;
					GPIO_Init(GPIO_DATA_PORT, GPIO_DATA_PIN, GPIO_Mode_In_FL_No_IT);
				}
			}
		}
				
	}
}