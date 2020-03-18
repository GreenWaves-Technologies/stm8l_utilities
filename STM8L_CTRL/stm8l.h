#ifndef stm8l_h
#define stm8l_h

typedef enum
{
  RTC_Weekday_Monday      =  ((uint8_t)0x01), /*!< WeekDay is Monday */
  RTC_Weekday_Tuesday     =  ((uint8_t)0x02), /*!< WeekDay is Tuesday */
  RTC_Weekday_Wednesday   =  ((uint8_t)0x03), /*!< WeekDay is Wednesday */
  RTC_Weekday_Thursday    =  ((uint8_t)0x04), /*!< WeekDay is Thursday */
  RTC_Weekday_Friday      =  ((uint8_t)0x05), /*!< WeekDay is Friday */
  RTC_Weekday_Saturday    =  ((uint8_t)0x06), /*!< WeekDay is Saturday */
  RTC_Weekday_Sunday      =  ((uint8_t)0x07)  /*!< WeekDay is Sunday */
}
RTC_Weekday_TypeDef;

/**
  * @}
  */
  
/** @defgroup RTC_Months
  * @{
  */ 
typedef enum
{
  RTC_Month_January   =  ((uint8_t)0x01), /*!< Month is January */
  RTC_Month_February  =  ((uint8_t)0x02), /*!< Month is February */
  RTC_Month_March     =  ((uint8_t)0x03), /*!< Month is March */
  RTC_Month_April     =  ((uint8_t)0x04), /*!< Month is April */
  RTC_Month_May       =  ((uint8_t)0x05), /*!< Month is May */
  RTC_Month_June      =  ((uint8_t)0x06), /*!< Month is June */
  RTC_Month_July      =  ((uint8_t)0x07), /*!< Month is July */
  RTC_Month_August    =  ((uint8_t)0x08), /*!< Month is August */
  RTC_Month_September =  ((uint8_t)0x09), /*!< Month is September */
  RTC_Month_October   =  ((uint8_t)0x10), /*!< Month is October */
  RTC_Month_November  =  ((uint8_t)0x11), /*!< Month is November */
  RTC_Month_December  =  ((uint8_t)0x12)  /*!< Month is December */
}
RTC_Month_TypeDef;

/**
  * @}
  */
  
/** @defgroup RTC_Hour_Format
  * @{
  */ 
typedef enum
{
  RTC_HourFormat_24  = ((uint8_t)0x00), /*!< Hour Format is 24H */
  RTC_HourFormat_12  = ((uint8_t)0x40)  /*!< Hour Format is 12H (using AM/PM) */
}
RTC_HourFormat_TypeDef;

/**
  * @}
  */
  
/** @defgroup RTC_Time
  * @{
  */ 
typedef enum
{
  RTC_H12_AM     = ((uint8_t)0x00), /*!< AM/PM notation is AM or 24 hour format */
  RTC_H12_PM     = ((uint8_t)0x40)  /*!< AM/PM notation is PM  */
}
RTC_H12_TypeDef;

/**
  * @}
  */
  
/** @defgroup RTC_Alarm_WeekDay_Selection
  * @{
  */ 
typedef enum
{
  RTC_AlarmDateWeekDaySel_Date     = ((uint8_t)0x00), /*!< Date/WeekDay selection is Date */
  RTC_AlarmDateWeekDaySel_WeekDay  = ((uint8_t)0x40)  /*!< Date/WeekDay selection is WeekDay */
}
RTC_AlarmDateWeekDaySel_TypeDef;

/**
  * @}
  */
  
/** @defgroup RTC_Alarm_Mask
  * @{
  */ 
typedef enum
{
  RTC_AlarmMask_None         =  ((uint8_t)0x00), /*!< Alarm Masks disabled */
  RTC_AlarmMask_Seconds      =  ((uint8_t)0x80), /*!< Alarm Seconds Mask */
  RTC_AlarmMask_Minutes      =  ((uint8_t)0x40), /*!< Alarm Minutes Mask */
  RTC_AlarmMask_Hours        =  ((uint8_t)0x20), /*!< Alarm Hours Mask */
  RTC_AlarmMask_DateWeekDay  =  ((uint8_t)0x10), /*!< Alarm Date/WeekDay Mask */
  RTC_AlarmMask_All          =  ((uint8_t)0xF0)  /*!< Alarm All Mask are enabled */
}
RTC_AlarmMask_TypeDef;


typedef struct
{
  uint8_t RTC_Weekday;

  uint8_t day;

  uint8_t RTC_Month;

  uint8_t year;

  uint8_t hour;

  uint8_t min;

  uint8_t sec;

}
RTC_Calendar_TypeDef;


typedef struct
{
  uint8_t hour;

  uint8_t min;

  uint8_t sec;

  uint8_t WeekDaySel;                                        /*- If RTC Alarm Date/WeekDay selection is Date
                                                                and if If Binary format is selected, this
                                                                parameter can be any value from 1 to 31.
                                                               - If RTC Alarm Date/WeekDay selection is WeekDay,
                                                                 this parameter can be one of the
                                                                 @ref RTC_Weekday_TypeDef enumeration.*/

  uint8_t dateweekday;

  uint8_t AlarmMask;

}
RTC_Alarm_TypeDef;

/** @defgroup RTC_Wakeup_Clock
  * @{
  */ 
typedef enum
{
  RTC_WakeUpClock_RTCCLK_Div16    = ((uint8_t)0x00), /*!< (RTC clock) div 16*/
  RTC_WakeUpClock_RTCCLK_Div8     = ((uint8_t)0x01), /*!< (RTC clock) div 8*/
  RTC_WakeUpClock_RTCCLK_Div4     = ((uint8_t)0x02), /*!< (RTC clock) div 4*/
  RTC_WakeUpClock_RTCCLK_Div2     = ((uint8_t)0x03), /*!< (RTC clock) div 2*/
  RTC_WakeUpClock_CK_SPRE_16bits  = ((uint8_t)0x04), /*!< CK SPRE with a counter from 0x0000 to 0xFFFF */
  RTC_WakeUpClock_CK_SPRE_17bits  = ((uint8_t)0x06)  /*!< CK SPRE with a counter from 0x10000 to 0x1FFFF */
}
RTC_WakeUpClock_TypeDef;



#endif