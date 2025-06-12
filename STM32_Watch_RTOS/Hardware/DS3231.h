#ifndef __DS3231_H
#define __DS3231_H

#include "MyI2C.h"
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#define DS3231_ADDRESS		0xD0		//DS3231的I2C从机地址

#define SOUND_KEY    0x02  // 按键提示音  
#define SOUND_SCROLL 0x04  // 菜单滚动音  
#define SOUND_ALARM  0x08  // 闹钟提醒  

//读取数据的地址，不怎么变化，选择枚举
typedef enum
{
	SECONDS=0x00,
	MINUTES,
	HOURS,    
	DAY,			//星期 1-7
	DATE,			//日 1-31
	MONTH,
	YEAR,
}DS3231_Addr;

typedef struct
{
	uint8_t sec;  
	uint8_t min;
	uint8_t hour;  //时间，默认24小时制
	uint8_t day;  //星期
	uint8_t date; //日
	uint8_t month;
	uint8_t year;	
}DS3231_TimeType;



void Buzzer_Init(void);
void Alarm_Init(void);
void DS3231_Init(void);
void DS3231_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t DS3231_ReadReg(uint8_t RegAddress);
void DS3231_ReadRegS(uint8_t RegAddress,uint8_t *Data,uint8_t Count);
void DS3231_Write_Time(DS3231_TimeType Time);
void DS3231_Read_Time(DS3231_TimeType *DS3231_Time);
void DS3231_Write_Alarm(DS3231_TimeType Time);
void DS3231_Read_Alarm(DS3231_TimeType *DS3231_Time);
uint8_t DS3231_GetAlarm(DS3231_TimeType DS3231_Time);
void SetClock_Task(void *params );
void SetAlarm_Task(void *params);
void Buzzer_Task(void *params);
#endif
