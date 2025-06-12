#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "Key.h"

extern QueueHandle_t inputQueue;
extern TaskHandle_t xBuzzer_TaskHandler;

volatile uint8_t press_count = 0;
volatile bool is_pressing = false;


extern SemaphoreHandle_t xBuzzer_Semaphore;//闹钟的信号量


// 初始化TIM2为20ms中断（初始消抖周期）
void TIM2_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 3000 - 1;				//计数周期，即ARR的值 实现30ms定时中断
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);				//将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元	

	/*中断输出配置*/
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);						//清除定时器更新标志位
																//TIM_TimeBaseInit函数末尾，手动产生了更新事件
																//若不清除此标志位，则开启中断后，会立刻进入一次中断
																//如果不介意此问题，则不清除此标志位也可
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);		
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);				//配置NVIC为分组4 均为抢占优先级
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//选择配置NVIC的TIM2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	TIM_Cmd(TIM2, ENABLE);                          // 初始不启动定时器
}

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  //PA1引脚为按键引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA1引脚初始化为上拉输入
	
//	// 配置外部中断
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
//	
//	EXTI_InitTypeDef EXTI_InitStructure;
//	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
//	
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
}

/**
  * 函    数：旋转编码器初始化
  * 参    数：无
  * 返 回 值：无
  */
void Encoder_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//开启AFIO的时钟，外部中断必须开启AFIO的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//将P和PA7引脚初始化为上拉输入
	
	/*AFIO选择中断引脚*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);//将外部中断的0号线映射到GPIOA，即选择PA6为外部中断引脚
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	
	/*EXTI初始化*/
	EXTI_InitTypeDef EXTI_InitStructure;						//定义结构体变量
	EXTI_InitStructure.EXTI_Line = EXTI_Line6 ;//| EXTI_Line7;		//选择配置外部中断的6号线和7号线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//指定外部中断线使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//指定外部中断线为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//指定外部中断线为下降沿触发
	EXTI_Init(&EXTI_InitStructure);								//将结构体变量交给EXTI_Init，配置EXTI外设
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//选择配置NVIC的EXTI线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;	//指定NVIC线路的抢占优先级为6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//指定NVIC线路的响应优先级为0
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
}

// 按键处理任务 分长短按，适用于多个按键，这里是使用的这个，使用下面那个只识别短按也可以
void Key_Task(void *params) 
{
	InputData key_data;
  const TickType_t longPressTime = pdMS_TO_TICKS(2000);//长按的时间阈值（2000毫秒）
  TickType_t pressStart;//记录按键按下的开始时间
	static uint32_t lastpressTime = 0;
  BaseType_t longPressed = pdFALSE; // 用于标记是否发生了长按
	BaseType_t shortPressed = pdFALSE; // 用于标记是否发生了长按
  while(1) 
	{
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==RESET) {//检测为低电平  			
			pressStart = xTaskGetTickCount();
      while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==RESET) {//进行循环直到被拉高了
        vTaskDelay(pdMS_TO_TICKS(50));//每50毫秒检查一次按键状态
				if((pressStart-lastpressTime)>pdMS_TO_TICKS(500))//避免在长按的结束时刻触发短按
				shortPressed= pdTRUE; // 标记发生了短按
        //如果按键按下的时间超过了长按时间阈值，发送一个长按事件到队列，并跳出循环
        if((xTaskGetTickCount() - pressStart) > longPressTime) {
					key_data.id=Key_ID;
          key_data.count = BUTTON_LONG;
					longPressed = pdTRUE; // 标记发生了长按
					lastpressTime=xTaskGetTickCount();//获取长按时刻
					shortPressed = pdFALSE; // 把短按取消掉
          xQueueSend(inputQueue, &key_data, 0);
					xTaskNotify(xBuzzer_TaskHandler, 0x02, eSetBits);//轻量级事件组 00000010
          break;
        }
      }
      //如果按键被释放，发送一个短按事件到队列。然后，任务延迟20毫秒再次检查按键状态
      if((!longPressed) &&shortPressed) {
        key_data.id=1;
        key_data.count = BUTTON_SHORT;
        xQueueSend(inputQueue, &key_data, 0);
				xTaskNotify(xBuzzer_TaskHandler, SOUND_KEY, eSetBits);//轻量级事件组 00000010
      }
    }
		longPressed = pdFALSE; // 重置长按标记
		shortPressed = pdFALSE; // 重置短按标记
    vTaskDelay(pdMS_TO_TICKS(20));//延时20ms
  }
}



////按键触发中断
//void EXTI1_IRQHandler(void) {
//    if (EXTI_GetITStatus(EXTI_Line1) == SET) {
//			  taskENTER_CRITICAL();//临界区保护
//        EXTI->IMR &= ~EXTI_Line1;                    // 关闭中断防抖动
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == RESET) {
//            TIM_SetCounter(TIM2, 0);                 // 重置计数器
//					  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		 // 启动TIM2中断
//            is_pressing = true;
//        }
//        EXTI_ClearITPendingBit(EXTI_Line1);
//				taskEXIT_CRITICAL();
//    }
//}

////定时器2中断函数
//void TIM2_IRQHandler(void) {
//    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
//        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//        
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == RESET) {
//                InputData short_press = {Key_ID, BUTTON_SHORT};
//                xQueueSendFromISR(inputQueue, &short_press, NULL);
//								xTaskNotify(xBuzzer_TaskHandler, SOUND_KEY, eSetBits);//轻量级事件组 00000010
//						taskENTER_CRITICAL();//临界区保护
//						TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);  // 停止定时器中断
//            EXTI->IMR |= EXTI_Line1;                 // 重新开启按键中断
//						taskEXIT_CRITICAL();
//						
//        }
//    }
//}

