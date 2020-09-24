#ifndef MAIN_H
	#define MAIN_H

	#define GPIO_SYS_SW_PORT GPIOA
	#define GPIO_SYS_SW_PIN GPIO_Pin_3
	#define GPIO_CLK_PORT GPIOB
	#define GPIO_CLK_PIN GPIO_Pin_6
	#define GPIO_DATA_PORT GPIOB
	#define GPIO_DATA_PIN GPIO_Pin_7
	#define GPIO_WAKEUP_PORT GPIOB
	#define GPIO_WAKEUP_PIN GPIO_Pin_3
	
	
	typedef enum
	{
		Set_Calendar = (uint8_t)0x00,
		Set_Alarm = (uint8_t)0x01,
		Set_Wake_Up = (uint8_t)0x02,
		Set_Year = (uint8_t)0x03,
		Set_Month = (uint8_t)0x04,
		Set_Day = (uint8_t)0x05,
		Set_WD = (uint8_t)0x06,
		Set_Hour = (uint8_t)0x07,
		Set_Minute = (uint8_t)0x08,
		Set_Seconds = (uint8_t)0x09,
		Set_SubSeconds = (uint8_t)0x0A,
		Read_Battery = (uint8_t)0x0B,
		Shutdown_Now = (uint8_t)0x0C,
		WakeUp_Rising_Edge = (uint8_t)0x0D,
		WakeUp_Falling_Edge = (uint8_t)0x0E,
		Calibrate_1sec = (uint8_t)0x0F,
		Set_CalibRoutine = (uint8_t)0x10,
		Start_CalibRoutine = (uint8_t)0x11,
		Set_CalibData = (uint8_t)0x12,
		Reset_Interface = (uint8_t)0xFF, 
		//READ COMMANDS
		Get_Calendar = (uint8_t)0x40,
		Get_Alarm = (uint8_t)0x41,
		Get_Wake_Up = (uint8_t)0x42,
		Get_Year = (uint8_t)0x43,
		Get_Month = (uint8_t)0x44,
		Get_Day = (uint8_t)0x45,
		Get_WD = (uint8_t)0x46,
		Get_Hour = (uint8_t)0x47,
		Get_Minute = (uint8_t)0x48,
		Get_Seconds = (uint8_t)0x49,
		Get_SubSeconds = (uint8_t)0x4A,
		Get_Battery_Voltage = (uint8_t)0x4B
} 	STM8L_Functions;
	
#endif