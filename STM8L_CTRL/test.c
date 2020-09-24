/* EXAMPLE OF DEEP SLEEP WITH STM8L050J3
// TESTED ON GAPPOC- B V2.1
// AUTHOR : M.GUERMANDI - GWT
 */

#include <stdio.h>
#include <rt/rt_api.h>
#include <stdint.h>
#include "stm8l.h"

#define GPIO_DATA 17
#define GPIO_CLK 21

//#define GPIO_DEBUG 25

//#define GPIO_LED 0



#define CLK_HALF_PERIOD 1


uint8_t read_byte(void)
{
    uint8_t rx_byte=0;
    for (int i=0; i<8; i++)
    {
        rt_gpio_set_pin_value(0, GPIO_CLK, 0);
        rt_time_wait_us(CLK_HALF_PERIOD);
        rt_gpio_set_pin_value(0, GPIO_CLK, 1);
        rt_time_wait_us(CLK_HALF_PERIOD);
        rx_byte=rx_byte<<1;
        rx_byte+=rt_gpio_get_pin_value(0, GPIO_DATA);
    }
    rt_gpio_set_pin_value(0, GPIO_CLK, 0);
    return(rx_byte);
}

void send_byte(uint8_t byte_to_send)
{
    for (int i=0; i<8; i++)
    {
        rt_gpio_set_pin_value(0, GPIO_DATA, (byte_to_send>>(7-i))&(0x01));
        rt_gpio_set_pin_value(0, GPIO_CLK, 0);
        rt_time_wait_us(CLK_HALF_PERIOD);
        rt_gpio_set_pin_value(0, GPIO_CLK, 1);
        rt_time_wait_us(CLK_HALF_PERIOD);
    }
    rt_gpio_set_pin_value(0, GPIO_CLK, 0);

}

void board_init()
{
   
    // GPIO initialization
    rt_gpio_init(0, GPIO_DATA);
    rt_gpio_init(0, GPIO_CLK);
    //rt_gpio_init(0, GPIO_DEBUG);
    //rt_gpio_init(0, GPIO_LED);

    rt_pad_set_function(23,1) ;     
    rt_pad_set_function(27,1) ;     
    //rt_pad_set_function(27,1) ;     

    // Configure GPIO as an outpout
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_OUT);
    rt_gpio_set_dir(0, 1<<GPIO_CLK, RT_GPIO_IS_OUT);
    //rt_gpio_set_dir(0, 1<<GPIO_DEBUG, RT_GPIO_IS_OUT);
    //rt_gpio_set_dir(0, 1<<GPIO_LED, RT_GPIO_IS_OUT);

    //rt_gpio_set_pin_value(0, GPIO_DEBUG, 1);
    //rt_gpio_set_pin_value(0, GPIO_LED, 1);


    rt_gpio_set_pin_value(0, GPIO_CLK, 0);

}

// SET STM8L RTC CALENDAR
void set_calendar(RTC_Calendar_TypeDef *RTC_Calendar)
{
    send_byte(0x00);        //SET CALENDAR
        send_byte(RTC_Calendar->RTC_Weekday);        //SET WEEKDAY
        send_byte(RTC_Calendar->day);           //SET DAY
        send_byte(RTC_Calendar->RTC_Month);          //SET MONTH
        send_byte(RTC_Calendar->year);          //SET YEAR
        send_byte(RTC_Calendar->hour);           //SET HOUR
        send_byte(RTC_Calendar->min);          //SET MIN
        send_byte(RTC_Calendar->sec);          //SET SEC
    send_byte(0xFF);        //END
}

// SET STM8L RTC ALARM (WAKE GAP8 WHEN CALENDAR MATCHES ALARM)
void set_alarm(RTC_Alarm_TypeDef *RTC_Alarm)
{
    send_byte(0x01);        //SET ALARM
        send_byte(RTC_Alarm->hour);        //SET HOUR
        send_byte(RTC_Alarm->min);           //SET MIN
        send_byte(RTC_Alarm->sec);          //SET SEC
        send_byte(RTC_Alarm->WeekDaySel);          //SET DATEWEEKDAYSEL
        send_byte(RTC_Alarm->dateweekday);           //SET DATEWEEKDAY
        send_byte(RTC_Alarm->AlarmMask);          //SET ALARM MASK
    send_byte(0xFF);        //END
}

// SET STM8L RTC WAKE UP AFTER [RTC_WakeUpClock_Value] PULSES OF THE 32.768 KHz RTC CLOCK DIVIDED BY [RTC_WakeUpClock_Divider]
// SINCE 0XFF IS TERMINATOR BYTE, SECOND AND THIRD WORD ARE MASKED TO HAVE BIT0 AT 0, THEIR BIT0 IS SENT WITH THE FIRST BYTE
void set_wakeup(RTC_WakeUpClock_TypeDef RTC_WakeUpClock_Divider, uint16_t RTC_WakeUpClock_Value)
{
    send_byte(0x02);
        send_byte((RTC_WakeUpClock_Divider<<2)+((RTC_WakeUpClock_Value>>7)&0x02)+((RTC_WakeUpClock_Value)&0x01));
        send_byte((RTC_WakeUpClock_Value>>8)&0XFE);
        send_byte((RTC_WakeUpClock_Value)&0XFE);        
    send_byte(0xFF);    
}

// SET STM8L TO GENERATE A PULSE ON THE DATA LINE OF [RTC_WakeUpClock_Value] PULSES OF THE 32.768 KHz RTC CLOCK DIVIDED BY [RTC_WakeUpClock_Divider]
// SINCE 0XFF IS TERMINATOR BYTE, SECOND AND THIRD WORD ARE MASKED TO HAVE BIT0 AT 0, THEIR BIT0 IS SENT WITH THE FIRST BYTE
void set_calibroutine(RTC_WakeUpClock_TypeDef RTC_WakeUpClock_Divider, uint16_t RTC_WakeUpClock_Value)
{
    send_byte(0x10);
        send_byte((RTC_WakeUpClock_Divider<<2)+((RTC_WakeUpClock_Value>>7)&0x02)+((RTC_WakeUpClock_Value)&0x01));
        send_byte((RTC_WakeUpClock_Value>>8)&0XFE);
        send_byte((RTC_WakeUpClock_Value)&0XFE);        
    send_byte(0xFF);    
}

uint32_t start_calibroutine(uint32_t no_of_cycles)
{
    *(uint32_t *)0x1A105104=0x00000001;
    *(uint32_t *)0x1A105100=0x00000000;
    *(uint32_t *)0x1A105000=0x00000002; //STOP
    *(uint32_t *)0x1A105004=0x00000914; //COUNT 32KHZ CYCLES IF GPIO20 IS LOW
    *(uint32_t *)0x1A105008=0xFFFF0000; //COUNT INDEFINITELY
    *(uint32_t *)0x1A10500C=0x0000FFFF; //COUNT INDEFINITELY
    *(uint32_t *)0x1A105000=0x00000008; //RESET
    *(uint32_t *)0x1A105000=0x00000004; //UPDATE
    *(uint32_t *)0x1A105000=0x00000001; //START
    
    send_byte(0x11);
    send_byte(0xFF); 
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_IN);

    while(rt_gpio_get_pin_value(0,GPIO_DATA)==1);
    while(rt_gpio_get_pin_value(0,GPIO_DATA)==0);
    return(no_of_cycles*32768/(*(uint32_t *)0x1A10502C)/2);
}

void send_calibdata(uint16_t calib_data)
{
    send_byte(0x12);
        send_byte(((calib_data>>7)&0x02)+((calib_data)&0x01));
        send_byte((calib_data>>8)&0XFE);
        send_byte((calib_data)&0XFE);        
    send_byte(0xFF);    
}

// WAKE GAP8 FROM RISING EDGE ON STM8L INTERRUPT PIN
void enable_IT_rising()
{
    send_byte(0x0D);        //ENABLE IT RISING EDGE
    send_byte(0xFF);        //END
}

// WAKE GAP8 FROM FALLING EDGE ON STM8L INTERRUPT PIN
void enable_IT_falling()
{
    send_byte(0x0E);        //ENABLE IT FALLING EDGE
    send_byte(0xFF);        //END
}

//SHUTDOWN GAP (CUT POWER)
void shutdown_gap8()
{
    send_byte(0x0C);        //SHUTDOWN
    send_byte(0xFF);        //END
}

// GET RTC CALENDAR
void get_calendar(RTC_Calendar_TypeDef *RTC_Calendar)
{
    send_byte(0x40);        //READ CALENDAR
    send_byte(0xFF);        //END

    //SWTICH DATA LINE TO INPUT
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_IN);
    rt_time_wait_us(10);
    
    RTC_Calendar->RTC_Weekday = read_byte();
    RTC_Calendar->day = read_byte();
    RTC_Calendar->RTC_Month = read_byte();
    RTC_Calendar->year = read_byte();
    RTC_Calendar->hour = read_byte();
    RTC_Calendar->min = read_byte();
    RTC_Calendar->sec = read_byte();

    rt_time_wait_us(10);
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_OUT);

}

// GET CURRENT ALARM
void get_alarm(RTC_Alarm_TypeDef *RTC_Alarm)
{
    send_byte(0x41);        //READ ALARM
    send_byte(0xFF);        //END

    //SWTICH DATA LINE TO INPUT
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_IN);
    rt_time_wait_us(10);

    RTC_Alarm->hour = read_byte();
    RTC_Alarm->min = read_byte();
    RTC_Alarm->sec = read_byte();
    RTC_Alarm->WeekDaySel = read_byte();
    RTC_Alarm->dateweekday = read_byte();
    RTC_Alarm->AlarmMask = read_byte();
    
    rt_time_wait_us(10);
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_OUT);
}

// RESET INTERFACE WITH STM8L IN CASE SOMETHING IS WRONG
void reset_Interface()
{
    for(int i=0;i<4;i++)
        send_byte(0xFF);
}


int main()
{
 
    RTC_Calendar_TypeDef RTC_Calendar;
    RTC_Alarm_TypeDef RTC_Alarm;

    uint8_t temp;

    board_init();

    reset_Interface();   // RESET INTERFACE WITH STM8L IN CASE SOMETHING IS WRONG (NOT REALLY NEEDED AT STARTUP)
    
    rt_time_wait_us(1000000);

    set_calibroutine(RTC_WakeUpClock_RTCCLK_Div16,512);   //WAKE ME UP AFTER 255 PULSES OF 32.768KHz CLOCK DIVIDED BY 16 (125 ms APPROX.)
    //enable_IT_falling();
    rt_time_wait_us(1000000);

    uint16_t cal;
    cal = start_calibroutine(512*16);
    printf("NO. OF CYCLES: %d\n", cal);
    //shutdown_gap8();          // SHUTDOWN GAP
    rt_time_wait_us(1000000);
    rt_gpio_set_dir(0, 1<<GPIO_DATA, RT_GPIO_IS_OUT);
    send_calibdata(cal);

    rt_time_wait_us(1000000);

    RTC_Calendar.RTC_Weekday = RTC_Weekday_Monday;
    RTC_Calendar.day = 1;
    RTC_Calendar.RTC_Month = RTC_Month_January;
    RTC_Calendar.year = 20;
    RTC_Calendar.hour = 0;
    RTC_Calendar.min = 0;
    RTC_Calendar.sec = 0;

    set_calendar(&RTC_Calendar); //TODAY IS MONDAY JAN 1st 2020, IT'S MIDNIGHT

    rt_time_wait_us(1000000);

    RTC_Alarm.hour = 0;
    RTC_Alarm.min = 0;
    RTC_Alarm.sec = 15;
    RTC_Alarm.WeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_Alarm.dateweekday = 1;
    RTC_Alarm.AlarmMask = RTC_AlarmMask_DateWeekDay;

   set_alarm(&RTC_Alarm);    //WAKE ME UP ON DAY 1st, 15 SECS AFTER MIDNIGHT

    //set_wakeup(RTC_WakeUpClock_RTCCLK_Div16,512);   //WAKE ME UP AFTER 255 PULSES OF 32.768KHz CLOCK DIVIDED BY 16 (125 ms APPROX.)

    //Srt_time_wait_us(1000000);
    shutdown_gap8();
  
    while(1);
    
//WONT GET BEYOND HERE IF YOU SHUT DOWN
    rt_time_wait_us(1000000);
    
    get_calendar(&RTC_Calendar);        // READ CALENDAR INFO

    printf("DAY CAL: %d\n",RTC_Calendar.day);   // TEST PRINTF SHOULD RETURN 1


    get_alarm(&RTC_Alarm);              // READ ALARM INFO

    printf("DAY AL: %d\n",RTC_Alarm.dateweekday);   // TEST PRINTF SHOULD RETURN 1
    

    return 0;


}

