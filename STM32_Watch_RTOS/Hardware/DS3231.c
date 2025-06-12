#include "stm32f10x.h"                  // Device header
#include "DS3231.h"



/**
  * 函    数：DS3231写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考DS3231手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void DS3231_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据，最高位为1
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}

/**
  * 函    数：DS3231读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考DS3231手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t DS3231_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(DS3231_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	Data = MyI2C_ReceiveByte();			//接收指定寄存器的数据
	MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}
void DS3231_WriteRegS(uint8_t RegAddress,uint8_t *Data,uint8_t Count)
{
	uint8_t i;
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	for(i=0;i<Count;i++)
	{
		MyI2C_SendByte(Data[i]);				//发送要写入寄存器的数据，最高位为1
	  MyI2C_ReceiveAck();					//接收应答
	}
	MyI2C_Stop();						//I2C终止
}

void DS3231_ReadRegS(uint8_t RegAddress,uint8_t *Data,uint8_t Count)
{
	uint8_t i;
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(DS3231_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	for(i=0;i<Count;i++)
	{
		if(i==Count-1)
		{
			Data[i] = MyI2C_ReceiveByte();			//接收指定寄存器的数据
			MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
		}
		else
		{
			Data[i] = MyI2C_ReceiveByte();			//接收指定寄存器的数据
			MyI2C_SendAck(0);					//发送应答，给从机应答
		}
	}	
	MyI2C_Stop();						//I2C终止
}

/**
  * 函    数：蜂鸣器初始化
  * 参    数：无
  * 返 回 值：无
  */
void Buzzer_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIO的时钟
															//使用各个外设前必须开启时钟，否则对外设的操作无效
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					//定义结构体变量	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				//GPIO引脚，赋值为第4号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		//GPIO速度，赋值为10MHz	
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将赋值后的构体变量传递给GPIO_Init函数
															//函数内部会自动根据结构体的参数配置相应寄存器
															//实现GPIOA的初始化
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA4引脚设置为低电平，蜂鸣器不会鸣叫
}

//闹铃初始化，主要针对ds3231触发外部中断的PA4引脚
void Alarm_Init(void)
{
	Buzzer_Init();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//开启AFIO的时钟，外部中断必须开启AFIO的时钟
	//初始化GPIO引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*AFIO选择中断引脚*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);//将外部中断的2号线映射到GPIOA，即选择PA8为外部中断引脚
	
  /*EXTI初始化*/
	EXTI_InitTypeDef EXTI_InitStructure;														//定义结构体变量
	EXTI_InitStructure.EXTI_Line=EXTI_Line8;												//选择配置外部中断的8号线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;												//指定外部中断线使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;							//指定外部中断线为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;					//指定外部中断线为下降沿触发，按键按下出现下降沿
	EXTI_Init(&EXTI_InitStructure);																	//将结构体变量交给EXTI_Init，配置EXTI外设 
	
}


/**
  * 函    数：DS3231初始化
  * 参    数：无
  * 返 回 值：无
  */
void DS3231_Init(void)
{
	MyI2C_Init();
	uint8_t ctrl =DS3231_ReadReg(0x0E);//获取中断控制数据  // 读取控制寄存器（不需要 BCD 转换）
	DS3231_WriteReg(0x0E, ctrl | 0x04); // INTCN=1 引脚在闹钟匹配时输出低电平（报警模式）,而不是输出方波
	//需要清除AF1,状态寄存器中的A1F或A2F位未被清除，则新的闹钟触发时无法置位该标志，从而无法触发中断。
	 uint8_t status = DS3231_ReadReg(0x0F);   
    if (status & 0x01) {                    // 检查 A1F 位（BIT0）也就是检查闹钟触发，触发会置1需要软件清零
        DS3231_WriteReg(0x0F, status & ~0x01); // 清除 A1F 标志
    }
}

/**
 * @brief     8位8421BCD码转为十进制码
 * @param     8421BCD码
 * @retval    十进制码  
 */
uint8_t Bcd_Dec(uint8_t Bcd_Num)
{
	return ((Bcd_Num>>4)*10+(Bcd_Num & 0x0f));
}


/**
 * @brief     十进制码转为8421BCD码
 * @param     十进制码
 * @retval    8421BCD码  
 */
uint8_t Dec_Bcd(uint8_t Dec_Num)
{
	return (((Dec_Num/10)<<4) | (Dec_Num % 10));
}


/**
 * @brief     设置时间
 * @param     时间结构体
 * @retval      
 */
void DS3231_Write_Time(DS3231_TimeType Time)
{
	uint8_t bcd_data[7];
	bcd_data[6]=Dec_Bcd(Time.year);
	bcd_data[5]=Dec_Bcd(Time.month);
	bcd_data[4]=Dec_Bcd(Time.date);
	bcd_data[3]=Dec_Bcd(Time.day);
	bcd_data[2]=Dec_Bcd(Time.hour);
	bcd_data[1]=Dec_Bcd(Time.min);
	bcd_data[0]=Dec_Bcd(Time.sec);
	
	DS3231_WriteRegS(SECONDS, bcd_data,7);
}


/**
 * @brief     读取时间
 * @param     时间结构体指针
 * @retval      
 */
void DS3231_Read_Time(DS3231_TimeType *DS3231_Time)
{
	uint8_t bcd_data[7];
	DS3231_ReadRegS(SECONDS,bcd_data,sizeof(bcd_data));
	
	DS3231_Time->sec  = Bcd_Dec(bcd_data[0]);               // 秒
	DS3231_Time->min  = Bcd_Dec(bcd_data[1]);               // 分钟
	DS3231_Time->hour = Bcd_Dec(bcd_data[2] & 0x3F);        // 小时（24小时制，去掉最高两位）
	DS3231_Time->day  = Bcd_Dec(bcd_data[3]);               // 星期几
	DS3231_Time->date = Bcd_Dec(bcd_data[4]);               // 日期
	DS3231_Time->month= Bcd_Dec(bcd_data[5] & 0x1F);        // 月份（去掉世纪位）
	DS3231_Time->year = Bcd_Dec(bcd_data[6]);               // 年份

}


/**
 * @brief     设置闹钟
 * @param     时间结构体
 * @retval      
 */
void DS3231_Write_Alarm(DS3231_TimeType Time)
{
	uint8_t bcd_data[3];
	// 转换并设置A1M1=0（秒匹配）
	bcd_data[0] = Dec_Bcd(Time.sec) & 0x7F;  // 秒寄存器，A1M1=0
	bcd_data[1] = Dec_Bcd(Time.min) & 0x7F;  // 分钟寄存器，A1M2=0
	bcd_data[2] = Dec_Bcd(Time.hour) & 0x7F; // 小时寄存器，A1M3=0
	// 秒(00), A1M1=0(参与匹配)
	DS3231_WriteRegS(0x07, bcd_data,3);//写入闹钟的时分秒数据
	DS3231_WriteReg(0x0A, 0x80);// 日(任意), A1M4=1(不参与匹配)
	
	// 启用Alarm1中断
	uint8_t ctrl =DS3231_ReadReg(0x0E);//获取中断控制数据  // 读取控制寄存器（不需要 BCD 转换）
	ctrl &= 0xFC;   // 保留BBSQW、EOSC等位
	ctrl |= 0x05;   // INTCN=1（中断模式）, A1IE=1（启用Alarm1中断）
	DS3231_WriteReg(0x0E, 05);
}

/**
 * @brief     读取闹钟
 * @param     时间结构体
 * @retval      
 */
void DS3231_Read_Alarm(DS3231_TimeType *DS3231_Time)
{
	uint8_t bcd_data[3];
	DS3231_ReadRegS(0x07,bcd_data,sizeof(bcd_data));
	
	DS3231_Time->sec  = Bcd_Dec(bcd_data[0]);               // 秒
	DS3231_Time->min  = Bcd_Dec(bcd_data[1]);               // 分钟
	DS3231_Time->hour = Bcd_Dec(bcd_data[2] & 0x3F);        // 小时（24小时制，去掉最高两位）
}


uint8_t DS3231_GetAlarm(DS3231_TimeType DS3231_Time)
{
	uint8_t bcd_data[3];
	DS3231_ReadRegS(0x07,bcd_data,sizeof(bcd_data));
	if((DS3231_Time.sec == Bcd_Dec(bcd_data[0]))&& (DS3231_Time.min == Bcd_Dec(bcd_data[1])) && (DS3231_Time.hour == Bcd_Dec(bcd_data[2] & 0x3F)))
	{
		return 8;
	}
	else return 0;

}


extern QueueHandle_t inputQueue;
extern TaskHandle_t xMenuShow_TaskHandler; 
extern TaskHandle_t xMainShow_TaskHandler; 
extern TaskHandle_t xBuzzer_TaskHandler; 

//  //初始化ds3231的时间数据
//  DS3231_TimeType currentTime;
//  currentTime.year = 25;   
//  currentTime.month = 4;    
//  currentTime.day = 24;
//  currentTime.date = 21;   	
//  currentTime.hour = 10;   
//  currentTime.min = 50;     
//  currentTime.sec = 00;      
//  DS3231_Write_Time(currentTime);

//  //初始化ds3231的时间数据
  	
//  currentTime.hour = 10;   
//  currentTime.min = 17;     
//  currentTime.sec = 00;      
//  DS3231_Write_Time(AlarmTime);
#define MAX_Clock_Object 		5
#define MAX_Alarm_Object		2

static uint8_t Clock_Object=0;
static uint8_t Alarm_Object=0;
static DS3231_TimeType SetClock_Value;
static DS3231_TimeType SetAlarm_Value;  
extern SemaphoreHandle_t xSet_Init_Semaphore;

void SetClock_Task(void *params )
{
	InputData settime_data;
	while(1)
	{
		// 尝试获取信号量（非阻塞方式）
		if (xSemaphoreTake(xSet_Init_Semaphore, 0) == pdTRUE) {// 仅执行一次的代码
			taskENTER_CRITICAL();
			DS3231_Read_Time(&SetClock_Value);
			taskEXIT_CRITICAL();
		}
		// 检查是否有命令输入
		if (xQueueReceive(inputQueue, &settime_data, 5) == pdPASS) {
				if ((settime_data.id == 1) && (settime_data.count == 1)&&(Clock_Object==MAX_Clock_Object)) {	
					  taskENTER_CRITICAL();
					  DS3231_Write_Time(SetClock_Value);//把设置的时间写入ds3231时钟芯片
					  taskEXIT_CRITICAL();
						Clock_Object=0;//时间选择项清零
//					  xSemaphoreGive(xSet_Init_Semaphore);//释放初始化信号量，用于其他被悬挂的任务恢复时，某些代码进行必要的单次运行
					  vTaskResume(xMenuShow_TaskHandler);
						vTaskResume(xMainShow_TaskHandler);
						vTaskSuspend(NULL);
						continue;//跳出当前循环
				}
				else if ((settime_data.id == 1) && (settime_data.count == 1)) {
						Clock_Object++;//时间选择项加一	
				}
				else if ((settime_data.id == 2) && (settime_data.count != 0)) {
					switch (Clock_Object)
						{
							case 0:
								SetClock_Value.year+=settime_data.count;break;
							case 1:
								SetClock_Value.month+=settime_data.count;break;
							case 2:
								SetClock_Value.date+=settime_data.count; break;
//							case 4:
//								SetClock_Value.day+=settime_data.count;
//								break;
							case 3:
								SetClock_Value.hour+=settime_data.count; break;
							case 4:
								SetClock_Value.min+=settime_data.count;break;
							case 5:
								SetClock_Value.sec+=settime_data.count;break;
						}
				}
				else {
				}
		    
		}
		taskENTER_CRITICAL();	
		OLED_Clear();	
		OLED_ShowString(10,2,"Clock Setting",OLED_8X16);
		OLED_DrawLine(0,20,128,20);//画线表示选择对象
		OLED_Printf(32, 22,OLED_8X16, "%02d-%02d-%02d", SetClock_Value.year, SetClock_Value.month, SetClock_Value.date); // 显示要修改的日期
    OLED_Printf(32, 46,OLED_8X16, "%02d:%02d:%02d", SetClock_Value.hour, SetClock_Value.min, SetClock_Value.sec); // 显示要修改的时间
    OLED_DrawLine(32+(Clock_Object%3)*24,40+(Clock_Object/3)*22,48+(Clock_Object%3)*24,40+(Clock_Object/3)*22);//画线表示选择对象
		OLED_Update();
		taskEXIT_CRITICAL(); 
		settime_data.id=0;
		settime_data.count=0;
		vTaskDelay(100);
	}
}


void SetAlarm_Task(void *params)
{
	InputData setalarm_data;
	while(1)
	{
		  //先释放了信号量，再创建的任务，所以不等待信号量,直接创建任务
			if (xSemaphoreTake(xSet_Init_Semaphore, 0) == pdTRUE) {// 仅执行一次的代码
				taskENTER_CRITICAL();
				DS3231_Read_Alarm(&SetAlarm_Value);
				taskEXIT_CRITICAL();
			}
			// 检查是否有命令输入
			if (xQueueReceive(inputQueue, &setalarm_data, 10) == pdPASS) {
				if ((setalarm_data.id == 1) && (setalarm_data.count == 1)&&(Alarm_Object==MAX_Alarm_Object)) {
					  taskENTER_CRITICAL();
					  DS3231_Write_Alarm(SetAlarm_Value);//把设置的闹钟写入ds3231时钟芯片
					  taskEXIT_CRITICAL();
						Alarm_Object=0;//闹钟选择项清零
					  vTaskResume(xMenuShow_TaskHandler);
						vTaskResume(xMainShow_TaskHandler);
						vTaskSuspend(NULL);
						continue;//跳出当前循环
				}
				else if ((setalarm_data.id == 1) && (setalarm_data.count == 1)) {
						Alarm_Object++;//闹钟选择项加一				
				}
				else if ((setalarm_data.id == 2) && (setalarm_data.count != 0)) {//旋转
					switch (Alarm_Object)
						{
							case 0:
								SetAlarm_Value.hour+=setalarm_data.count;break;
							case 1:
								SetAlarm_Value.min+=setalarm_data.count;break;
							case 2:
								SetAlarm_Value.sec+=setalarm_data.count;break;
						}
				}
				else {}
		     
		}
		taskENTER_CRITICAL();	
		OLED_Clear();	
		OLED_ShowString(10,10,"Alarm Setting",OLED_8X16);
		OLED_DrawLine(0,28,128,28);//画线
    OLED_Printf(32, 32,OLED_8X16, "%02d:%02d:%02d", SetAlarm_Value.hour, SetAlarm_Value.min, SetAlarm_Value.sec); // 显示要修改的时间
    OLED_DrawLine(32+Alarm_Object*24,50,48+Alarm_Object*24,50);//画线表示选择对象
		OLED_Update();
		taskEXIT_CRITICAL(); 
		vTaskDelay(100);
	}
	
}



void Buzzer_Task(void *params)
{
	uint32_t pre_time;
	uint32_t Buzzer_NotificationValue;
	while(1)
	{
		xTaskNotifyWait(
    ~0,             // ulBitsToClearOnEntry = 0xFFFFFFFF（所有位清零）
    0,              // ulBitsToClearOnExit = 0（退出时不修改任何位）
    &Buzzer_NotificationValue, // 获取通知值
    portMAX_DELAY    // 一直等待
    );		
		if(Buzzer_NotificationValue & SOUND_KEY)//按键
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为高电平，蜂鸣器鸣叫
			vTaskDelay(15);							//延时10ms
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为低电平，蜂鸣器停止
			vTaskDelay(10);							//延时10ms
		}
		else if(Buzzer_NotificationValue & 0x04)//滚轮
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为高电平，蜂鸣器鸣叫
			vTaskDelay(5);							//延时10ms
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为低电平，蜂鸣器停止
//			vTaskDelay(10);							//延时10ms	
		}
		else if(Buzzer_NotificationValue & 0x08)
		{
			pre_time=xTaskGetTickCount();
			while(xTaskGetTickCount()-pre_time<5000)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为高电平，蜂鸣器鸣叫
				vTaskDelay(200);							//延时100ms
				GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为低电平，蜂鸣器停止
				vTaskDelay(200);							//延时100ms
			}
		}
	}
}
