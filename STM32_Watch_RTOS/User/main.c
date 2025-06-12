#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "DS3231.h"
#include <string.h>
#include "ADXL345.h"
#include "Max30102.h"
#include "AD.h"
#include "Key.h"
#include "Encoder.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"  // 这个头文件包含了与信号量相关的所有函数和类型定义

//实时时间
DS3231_TimeType DS3231_R_Time;	//日历结构体
volatile uint8_t alarm_flag=0;

//步数
uint8_t Active_axis=1;  //1-X,2-Y,3-Z 默认初始为x轴
Peak_Value Peak = {{0}, {0}, {0}};//ADXL345样本集中最大和最小值及差值
Slid_Reg Slid = {{0}, {0}};//移位寄存器高频滤波
Aver_Acc Cur_Sample;//ADCL345当前样本
uint8_t Step_Count=0;//步数
//心率血氧
uint32_t buffer_length=500; //缓冲区长度为500，可存储以100sps运行的5秒样本; 数据长度（红外光数据与红光数据数据长度相等）
uint32_t ir_buffer[500]; 	 //IR LED   红外光数据，用于计算血氧
uint32_t red_buffer[500];  //Red LED	红光数据，用于计算心率曲线以及计算心率
int32_t n_sp02=0; 				//SPO2值
int8_t ch_spo2_valid;   //用于显示SP02计算是否有效的指示符
int32_t n_heart_rate=0;   //心率值
int8_t  ch_hr_valid;    //用于显示心率计算是否有效的指示符
uint16_t index_count;
uint32_t un_min, un_max;  //存储红光数据的最小值和最大值，用于归一化处理
uint8_t dis_hr=0,dis_spo2=0;//用于显示的心率和血氧饱和度值

uint8_t UART_Star=0;
static uint16_t BAT_Value;
static uint16_t Show_proportion;

QueueHandle_t inputQueue;//输入控制队列

TaskHandle_t xMainShow_TaskHandler;
TaskHandle_t xMenuShow_TaskHandler;
TaskHandle_t xSerial_TaskHandler;
TaskHandle_t xSetClock_TaskHandler;
TaskHandle_t xSetAlarm_TaskHandler;
TaskHandle_t xADXL_TaskHandler;
TaskHandle_t xMAX_TaskHandler;
TaskHandle_t xBuzzer_TaskHandler;
TaskHandle_t xKey_TaskHandler;

BaseType_t ret;											//动态内存分配返回结果
volatile SemaphoreHandle_t xSet_Init_Semaphore;//用于任务恢复时进行相关配置的信号量
volatile SemaphoreHandle_t xBuzzer_Semaphore;//闹钟的信号量
volatile SemaphoreHandle_t xMax30102_Semaphore;//闹钟的信号量
//EventGroupHandle_t xBuzzer_EventGroup;

void MainShow_Task(void *params);
void MenuShow_Task(void *params);
void Serial_Task(void *params);
void ADXL_Task(void *params);
void DS_Task(void *params);
void MAX_Task(void *params);

void Max30102_Get(void);

int main(void)
{
  /*模块初始化*/
  OLED_Init();			//OLED初始化
  Serial_Init();
  ADXL345_Init();
  MAX30102_Init();
	Alarm_Init();
  DS3231_Init();
  AD_Init();
//	TIM2_Init();
  Key_Init();
  Encoder_Init();
  OLED_Clear();
	OLED_ReverseArea(14,20,96,24);
  OLED_ShowString(18,24,"Smart Watch",OLED_8X16);
	OLED_DrawRectangle(16,22,92,20,OLED_UNFILLED);
  OLED_Update();
//	//读取前500个样本，并确定信号范围
//	for(index_count=0;index_count<buffer_length;index_count++)
//	{	
//		  while(MAX30102_INT()==1);
//  		MAX30102_FIFO_ReadWord(&red_buffer[index_count],&ir_buffer[index_count]);//获取红光和红外光的数据  
//	}
// 	//计算前500个样本（前5秒的样本）后的心率和血氧饱和度
//  maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
//	Delay_ms(1000);
	xSet_Init_Semaphore=xSemaphoreCreateBinary();
	xBuzzer_Semaphore=xSemaphoreCreateBinary();
  inputQueue= xQueueCreate(10, sizeof(InputData));// 输入事件队列 用于存储输入事件。队列的大小是10，意味着它可以同时存储10个事件
  // 创建任务
  ret=xTaskCreate(MainShow_Task,"MainShow", 128, NULL, 3, &xMainShow_TaskHandler);//主显示
  ret=xTaskCreate(MenuShow_Task,"MenuShow", 128, NULL, 7, &xMenuShow_TaskHandler);//菜单切换
  ret=xTaskCreate(Key_Task,     "Key_Task", 128, NULL, 8, &xKey_TaskHandler);			//按键
  ret=xTaskCreate(SetClock_Task,"SetClock", 128, NULL, 6, &xSetClock_TaskHandler);//设置时间
  ret=xTaskCreate(SetAlarm_Task,"SetAlarm", 128, NULL, 6, &xSetAlarm_TaskHandler);//设置闹钟
  ret=xTaskCreate(ADXL_Task,    "ADXL_Task",128, NULL, 5, &xADXL_TaskHandler);		//步数检测
  ret=xTaskCreate(MAX_Task,     "MAX_Task", 128, NULL, 6, &xMAX_TaskHandler);			//心率血氧检测
	ret=xTaskCreate(Buzzer_Task,  "Buzzer_Task", 128, NULL, 3, &xBuzzer_TaskHandler);	//闹钟响起
	ret=xTaskCreate(Serial_Task,  "Serial_Task", 128, NULL, 6, &xSerial_TaskHandler);  //串口(蓝牙）开关
  vTaskSuspend(xSetClock_TaskHandler);
  vTaskSuspend(xSetAlarm_TaskHandler);
  vTaskSuspend(xMAX_TaskHandler);
  vTaskSuspend(xSerial_TaskHandler);
  vTaskStartScheduler();          //开启任务调度
  while(1)
  {
    Serial_Printf( "Error\r\n");
  }
}


static int8_t Show_Page =0;

void MenuShow_Task(void *params)
{	
  while(1)
  {
		InputData menu_data;//获取到的队列数据
    //一直等待按键或者旋转编码器,否则不进行菜单切换
    xQueueReceive( inputQueue,&menu_data,portMAX_DELAY );
    if((menu_data.id==ENC_ID)&&(menu_data.count==1))//正转
    {
      Show_Page++;
      if(Show_Page>3) Show_Page=0;
			xTaskNotify(xBuzzer_TaskHandler, SOUND_SCROLL, eSetBits);//轻量级事件组 00000100
    }
    else if((menu_data.id==ENC_ID)&&(menu_data.count==-1))//反转
    {
      Show_Page--;
      if(Show_Page<0) Show_Page=3;
			xTaskNotify(xBuzzer_TaskHandler, SOUND_SCROLL, eSetBits);//轻量级事件组 00000100
    }
    else if((menu_data.id==Key_ID)&&(menu_data.count==1))//短按
    {
			xTaskNotify(xBuzzer_TaskHandler, SOUND_KEY, eSetBits);//轻量级事件组 00000010
      switch(Show_Page)
      {
        case 0:
					xSemaphoreGive(xSet_Init_Semaphore);//释放信号量，用于设置时间
          vTaskResume(xSetClock_TaskHandler);
          break;
        case 1:
					xSemaphoreGive(xSet_Init_Semaphore);//释放信号量，用于设置闹钟
          vTaskResume(xSetAlarm_TaskHandler); 
          break;
        case 2:
					xSemaphoreGive(xSet_Init_Semaphore);//释放信号量
          vTaskResume(xMAX_TaskHandler);
          break;
        case 3:
          vTaskResume(xSerial_TaskHandler);
          break;
      }
      vTaskSuspend(xMainShow_TaskHandler);
      vTaskSuspend(xMenuShow_TaskHandler);
    }  
    else if((menu_data.id==Key_ID)&&(menu_data.count==2))//长按
    {
			
    }
  }
}

//主显示任务
void MainShow_Task(void *params)
{
	uint16_t static show_steps=0;
	OLED_ShowNum(96,0,show_steps,5,OLED_6X8); //每十秒钟更新显示步数
	BAT_Value=AD_GetValue();//每十秒钟更新一次电量
	Show_proportion=(BAT_Value*3.3/4095)/3.3*100;//电量百分比
	Show_proportion/=25;
	TickType_t pre_stepshow=xTaskGetTickCount();
  while(1)
  {
    switch(Show_Page)
    {
      case 0:
        taskENTER_CRITICAL();
        DS3231_Read_Time(&DS3231_R_Time);
        OLED_Clear();
			  OLED_ShowString(54,0,"steps:",OLED_6X8);
			  OLED_ShowNum(92,0,show_steps,5,OLED_6X8);
			  if(xTaskGetTickCount()-pre_stepshow>10000)  {
					show_steps=Step_Count;//每十秒钟更新显示步数
					BAT_Value=AD_GetValue();//每十秒钟更新一次电量
					Show_proportion=(BAT_Value*3.3/4095)/3.3*100;//
//					OLED_ShowNum(18,0,Show_proportion,3,OLED_6X8);
					Show_proportion/=25;
					pre_stepshow=xTaskGetTickCount();}
				switch(Show_proportion)
					{
						case 4:
							OLED_ShowImage(2,1,16,8,Battery_Full); break;
						case 3:
							OLED_ShowImage(2,1,16,8,Battery_75); break;	
						case 2:
							OLED_ShowImage(2,1,16,8,Battery_50); break;
						case 1:
							OLED_ShowImage(2,1,16,8,Battery_25); break;	
						case 0:
							OLED_ShowImage(2,1,16,8,Battery_0); break;
						default: 
					    OLED_ShowImage(2,1,16,8,Battery_Full); break;}
        OLED_DrawLine(0, 12, 127, 12);
        OLED_Printf(32, 18,OLED_8X16, "%02d-%02d-%02d", DS3231_R_Time.year, DS3231_R_Time.month, DS3231_R_Time.date); // 显示日期
        OLED_Printf(32, 40,OLED_8X16, "%02d:%02d:%02d", DS3231_R_Time.hour, DS3231_R_Time.min, DS3231_R_Time.sec); // 显示时间				
        OLED_Update();
        taskEXIT_CRITICAL();
        break;
      case 1:
        taskENTER_CRITICAL();
        OLED_Clear();
        OLED_ShowString(10,24,"Alarm Setting",OLED_8X16);
			  OLED_DrawRectangle(8,22,108,20,OLED_UNFILLED);
        OLED_Update();
        taskEXIT_CRITICAL();
        break;
      case 2:
        taskENTER_CRITICAL();
        OLED_Clear();
        OLED_ShowString(26,15,"HR & SpO2",OLED_8X16);
			  OLED_ShowString(22,33,"Monitoring",OLED_8X16);
			  OLED_DrawRectangle(16,13,92,38,OLED_UNFILLED);
        OLED_Update();
        taskEXIT_CRITICAL();
        break;
      case 3:
        taskENTER_CRITICAL();
        OLED_Clear();
        OLED_ShowString(22,24,"Blue Tooth",OLED_8X16);
			  OLED_DrawRectangle(20,22,84,20,OLED_UNFILLED);
        OLED_Update();
        taskEXIT_CRITICAL();
        break;
    }
    vTaskDelay(200); // 延时 100ms
  }
}



void Serial_Task(void *params)
{
  InputData BT_data;
	static int8_t Open_flag=0;
  while (1)
  {
		// 检查是否有命令切换任务
		if (xQueueReceive(inputQueue, &BT_data, 10) == pdPASS) {
				if ((BT_data.id == 1) && (BT_data.count == 1)) {
					  if(Open_flag==1) {UART_Star = 1;}
						else UART_Star = 0;
						GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA4引脚设置为高电平，蜂鸣器鸣叫
						vTaskDelay(15);							//延时10ms
						GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA4引脚设置为低电平，蜂鸣器停止	
						vTaskResume(xMenuShow_TaskHandler);
						vTaskResume(xMainShow_TaskHandler);
						vTaskSuspend(NULL);
						continue;
				}
				else if (BT_data.id == 2) {
					Open_flag+=BT_data.count;
					if(Open_flag<0) Open_flag=1;//也就是关闭，只在开关之间切换
					Open_flag%=2;//只在开关之间切换
				}
		}
		taskENTER_CRITICAL();
    OLED_Clear();
		OLED_ShowString(0,14,"Bluetooth Close",OLED_8X16);
		OLED_ShowString(0,34,"Bluetooth Open",OLED_8X16);
    OLED_ReverseArea(0,14+20*Open_flag,128,18);
		OLED_Update();
    taskEXIT_CRITICAL();
    vTaskDelay(50);
  }
}


//后台任务  
void ADXL_Task(void *arg)
{
	bool Alarm_flag;
	TickType_t pre_sendtime=xTaskGetTickCount();
	TickType_t start_alarmtime=xTaskGetTickCount();
  while (1)
  {  
		//获取空闲栈
//		UBaseType_t freeNum;
//	  TaskHandle_t xTaskHandle;
//		xTaskHandle = xTaskGetCurrentTaskHandle();//任务句柄
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);	//获取空闲栈数量
//		Serial_Printf("FreeStack of Task %s : %d\n\r", pcTaskGetName(xTaskHandle), freeNum);	   
    ADXL345_Average(&Cur_Sample);
    Step_Count = Detect_Step(&Peak, &Slid, &Cur_Sample,&Active_axis); // 监测步数，显示步数
		
		if((xTaskGetTickCount()-pre_sendtime>1000)&&(UART_Star == 1))//1秒发送一次
		{
				//	串口发送数据
				taskENTER_CRITICAL();
				Serial_Printf( "HeartRate:%03d,Blood Oxygen:%02d%,Steps:%05d\r\n",n_heart_rate, n_sp02,Step_Count);
				taskEXIT_CRITICAL();
		    pre_sendtime=xTaskGetTickCount();
		}
		else if(UART_Star==2)  UART_Star = 0;//串口停止发送数据
		
		if (xSemaphoreTake(xBuzzer_Semaphore, 0) == pdTRUE) {// 仅执行一次的代码
		  Alarm_flag=true;
			start_alarmtime=xTaskGetTickCount();
		}
		if(Alarm_flag==true)
		{
			
			GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为高电平，蜂鸣器鸣叫
			vTaskDelay(100);							//延时100ms
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA2引脚设置为低电平，蜂鸣器停止
			vTaskDelay(80);							//延时80ms
			if(xTaskGetTickCount()-start_alarmtime>5000)
			{
				Alarm_flag=false;
			}
		}
		else {
			vTaskDelay(180); // 延时 180ms
     }
		
	}
    
}

//max30102心率血氧检测任务
void MAX_Task(void *arg)
{
  InputData HR_data;
  while(1)
  {
		//先释放了信号量，再创建的任务，所以不等待信号量,直接创建任务
			if (xSemaphoreTake(xSet_Init_Semaphore, 0) == pdTRUE) {// 仅执行一次的代码
				taskENTER_CRITICAL();
				MAX30102_Init();
				MAX30102_WriteReg(REG_INTR_ENABLE_1,0x80);	// INTR setting
				
				taskEXIT_CRITICAL();
				n_heart_rate=0;
				n_sp02=0;
				dis_hr=0;
			}
		// 检查是否有命令切换任务
		if (xQueueReceive(inputQueue, &HR_data, 10) == pdPASS) {
				if ((HR_data.id == 1) && (HR_data.count == 1)) {
					  index_count=0;
					  memset(red_buffer,0,500);
					  memset(ir_buffer,0,500);
					  GPIO_SetBits(GPIOA, GPIO_Pin_4);		//将PA4引脚设置为高电平，蜂鸣器鸣叫
						vTaskDelay(15);							//延时15ms
						GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//将PA4引脚设置为低电平，蜂鸣器停止
						vTaskResume(xMenuShow_TaskHandler);
						vTaskResume(xMainShow_TaskHandler);
						vTaskSuspend(NULL);
						continue;
				}
		}
    taskENTER_CRITICAL();//临界区保护
    OLED_Clear();
    //显示“心率”
    OLED_ShowChinese(0,14,"心率");
    OLED_ShowChar(34,14,':',OLED_8X16	);
    OLED_ShowString(70,14,"BMP",OLED_8X16	);
    //显示“血氧”
    OLED_ShowChinese(0,31,"血氧");
    OLED_ShowChar(34,31,':',OLED_8X16	);
    OLED_ShowChar(70,31,'%',OLED_8X16	);
		OLED_ShowNum(44, 14,n_heart_rate,3,OLED_8X16);
    OLED_ShowNum(44,31,n_sp02,2,OLED_8X16);
    OLED_Update();// 提交显示（I2C 操作）
    taskEXIT_CRITICAL();
		Max30102_Get();	
//    vTaskDelay(10);// 适当让出 CPU
  }
}

void Max30102_Get(void)
{
  for(index_count=100; index_count<buffer_length; index_count++)
  {
    red_buffer[index_count-100]=red_buffer[index_count];	//将100-500缓存数据移位到0-400
    ir_buffer[index_count-100] =ir_buffer[index_count];		//将100-500缓存数据移位到0-400
  }
  for(index_count=400; index_count<buffer_length; index_count+=20)
  {
//		xSemaphoreTake(xMax30102_Semaphore, 20);
		while(MAX30102_INT()==1) {vTaskDelay(5);}
		MAX30102_FIFO_ReadWordS(red_buffer, ir_buffer,index_count,20);
  }
  maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
  if(ch_hr_valid == 1 && n_heart_rate<120)
  {
    n_heart_rate-=20;	//做补偿
    dis_hr=n_heart_rate;
  }
  else if((n_heart_rate>120)&&(ch_hr_valid== 1)&&dis_hr>0)
  {
    n_heart_rate=dis_hr;
    dis_hr=n_heart_rate;
  }
  else
  {
    n_heart_rate=0;
    n_sp02=0;
    dis_hr=0;
  }
}


/**
  * 函    数：EXTI外部中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void EXTI9_5_IRQHandler(void) {
  static uint32_t lastEdgeTime = 0;
	InputData enc_data;
  uint32_t now = xTaskGetTickCountFromISR();//获取此刻滴答计数
  if (EXTI_GetITStatus(EXTI_Line6) == SET)		//判断是否是外部中断6号线触发的中断
  {
		if ((now - lastEdgeTime) > pdMS_TO_TICKS(20)) { // 消抖处理
            enc_data.id = ENC_ID;
            enc_data.count = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == RESET) ? 1 : -1; // 根据另一相的电平判断方向
            lastEdgeTime = now;
            xQueueSendFromISR(inputQueue, &enc_data, NULL);//发送队列中断函数
			      xTaskNotify(xBuzzer_TaskHandler, SOUND_SCROLL, eSetBits);//轻量级事件组 00000100
        }
        EXTI_ClearITPendingBit(EXTI_Line6); // 清除外部中断6号线的中断标志位
   }
//	else if (EXTI_GetITStatus(EXTI_Line5) == SET)		//判断是否是外部中断6号线触发的中断
//  {
//		    xSemaphoreGive(xMax30102_Semaphore);   //释放信号量，用于读取数据
//        EXTI_ClearITPendingBit(EXTI_Line6); // 清除外部中断6号线的中断标志位
//   }
//	else if (EXTI_GetITStatus(EXTI_Line7) == SET)		//判断是否是外部中断0号线触发的中断
//  {
//		if ((now - lastEdgeTime) > pdMS_TO_TICKS(10)) { // 消抖处理
////            enc_data.id = 2;
////            enc_data.count = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == RESET) ? 1 : -1; // 根据另一相的电平判断方向
////            lastEdgeTime = now;
////            xQueueSendFromISR(inputQueue, &enc_data, NULL);
//        }
//				taskENTER_CRITICAL();//临界区保护
//        EXTI_ClearITPendingBit(EXTI_Line7); // 清除外部中断7号线的中断标志位
//				taskEXIT_CRITICAL();
//   }
	else if (EXTI_GetITStatus(EXTI_Line8) == SET)		//判断是否是外部中断4号线触发的中断
	{
    uint8_t status = DS3231_ReadReg(0x0F);
    if (status & 0x01) {                       // 检查 A1F 位（BIT0）也就是检查闹钟触发，触发会置1需要软件清零
        DS3231_WriteReg(0x0F, status & ~0x01); // 清除 A1F 标志
    }
		xSemaphoreGive(xBuzzer_Semaphore);   //释放信号量，用于闹钟响铃
    EXTI_ClearITPendingBit(EXTI_Line8);	 // 清除外部中断8号线的中断标志位	
	}
}
