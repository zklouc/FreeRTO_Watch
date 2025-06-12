#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "Key.h"

extern QueueHandle_t inputQueue;
extern TaskHandle_t xBuzzer_TaskHandler;

volatile uint8_t press_count = 0;
volatile bool is_pressing = false;


extern SemaphoreHandle_t xBuzzer_Semaphore;//���ӵ��ź���


// ��ʼ��TIM2Ϊ20ms�жϣ���ʼ�������ڣ�
void TIM2_Init(void)
{
	/*����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//����TIM2��ʱ��
	
	/*ʱ����Ԫ��ʼ��*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//����ṹ�����
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//ʱ�ӷ�Ƶ��ѡ�񲻷�Ƶ���˲������������˲���ʱ�ӣ���Ӱ��ʱ����Ԫ����
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//������ģʽ��ѡ�����ϼ���
	TIM_TimeBaseInitStructure.TIM_Period = 3000 - 1;				//�������ڣ���ARR��ֵ ʵ��30ms��ʱ�ж�
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//Ԥ��Ƶ������PSC��ֵ
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//�ظ����������߼���ʱ���Ż��õ�
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);				//���ṹ���������TIM_TimeBaseInit������TIM2��ʱ����Ԫ	

	/*�ж��������*/
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);						//�����ʱ�����±�־λ
																//TIM_TimeBaseInit����ĩβ���ֶ������˸����¼�
																//��������˱�־λ�������жϺ󣬻����̽���һ���ж�
																//�������������⣬������˱�־λҲ��
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);		
	/*NVIC�жϷ���*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);				//����NVICΪ����4 ��Ϊ��ռ���ȼ�
	
	/*NVIC����*/
	NVIC_InitTypeDef NVIC_InitStructure;						//����ṹ�����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//ѡ������NVIC��TIM2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ָ��NVIC��·ʹ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;	//ָ��NVIC��·����ռ���ȼ�Ϊ2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//ָ��NVIC��·����Ӧ���ȼ�Ϊ1
	NVIC_Init(&NVIC_InitStructure);								//���ṹ���������NVIC_Init������NVIC����
	
	TIM_Cmd(TIM2, ENABLE);                          // ��ʼ��������ʱ��
}

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//����GPIOA��ʱ��
	
	/*GPIO��ʼ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  //PA1����Ϊ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//��PA1���ų�ʼ��Ϊ��������
	
//	// �����ⲿ�ж�
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
//	
//	EXTI_InitTypeDef EXTI_InitStructure;
//	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // �½��ش���
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
  * ��    ������ת��������ʼ��
  * ��    ������
  * �� �� ֵ����
  */
void Encoder_Init(void)
{
	/*����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//����GPIOA��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//����AFIO��ʱ�ӣ��ⲿ�жϱ��뿪��AFIO��ʱ��
	
	/*GPIO��ʼ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//��P��PA7���ų�ʼ��Ϊ��������
	
	/*AFIOѡ���ж�����*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);//���ⲿ�жϵ�0����ӳ�䵽GPIOA����ѡ��PA6Ϊ�ⲿ�ж�����
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	
	/*EXTI��ʼ��*/
	EXTI_InitTypeDef EXTI_InitStructure;						//����ṹ�����
	EXTI_InitStructure.EXTI_Line = EXTI_Line6 ;//| EXTI_Line7;		//ѡ�������ⲿ�жϵ�6���ߺ�7����
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//ָ���ⲿ�ж���ʹ��
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//ָ���ⲿ�ж���Ϊ�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//ָ���ⲿ�ж���Ϊ�½��ش���
	EXTI_Init(&EXTI_InitStructure);								//���ṹ���������EXTI_Init������EXTI����
	
	/*NVIC�жϷ���*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);				//����NVICΪ����2
																//����ռ���ȼ���Χ��0~3����Ӧ���ȼ���Χ��0~3
																//�˷������������������н������һ��
																//���ж���жϣ����԰Ѵ˴������main�����ڣ�whileѭ��֮ǰ
																//�����ö�����÷���Ĵ��룬���ִ�е����ûḲ����ִ�е�����
	
	/*NVIC����*/
	NVIC_InitTypeDef NVIC_InitStructure;						//����ṹ�����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ѡ������NVIC��EXTI��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ָ��NVIC��·ʹ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;	//ָ��NVIC��·����ռ���ȼ�Ϊ6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//ָ��NVIC��·����Ӧ���ȼ�Ϊ0
	NVIC_Init(&NVIC_InitStructure);								//���ṹ���������NVIC_Init������NVIC����
}

// ������������ �ֳ��̰��������ڶ��������������ʹ�õ������ʹ�������Ǹ�ֻʶ��̰�Ҳ����
void Key_Task(void *params) 
{
	InputData key_data;
  const TickType_t longPressTime = pdMS_TO_TICKS(2000);//������ʱ����ֵ��2000���룩
  TickType_t pressStart;//��¼�������µĿ�ʼʱ��
	static uint32_t lastpressTime = 0;
  BaseType_t longPressed = pdFALSE; // ���ڱ���Ƿ����˳���
	BaseType_t shortPressed = pdFALSE; // ���ڱ���Ƿ����˳���
  while(1) 
	{
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==RESET) {//���Ϊ�͵�ƽ  			
			pressStart = xTaskGetTickCount();
      while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==RESET) {//����ѭ��ֱ����������
        vTaskDelay(pdMS_TO_TICKS(50));//ÿ50������һ�ΰ���״̬
				if((pressStart-lastpressTime)>pdMS_TO_TICKS(500))//�����ڳ����Ľ���ʱ�̴����̰�
				shortPressed= pdTRUE; // ��Ƿ����˶̰�
        //����������µ�ʱ�䳬���˳���ʱ����ֵ������һ�������¼������У�������ѭ��
        if((xTaskGetTickCount() - pressStart) > longPressTime) {
					key_data.id=Key_ID;
          key_data.count = BUTTON_LONG;
					longPressed = pdTRUE; // ��Ƿ����˳���
					lastpressTime=xTaskGetTickCount();//��ȡ����ʱ��
					shortPressed = pdFALSE; // �Ѷ̰�ȡ����
          xQueueSend(inputQueue, &key_data, 0);
					xTaskNotify(xBuzzer_TaskHandler, 0x02, eSetBits);//�������¼��� 00000010
          break;
        }
      }
      //����������ͷţ�����һ���̰��¼������С�Ȼ�������ӳ�20�����ٴμ�鰴��״̬
      if((!longPressed) &&shortPressed) {
        key_data.id=1;
        key_data.count = BUTTON_SHORT;
        xQueueSend(inputQueue, &key_data, 0);
				xTaskNotify(xBuzzer_TaskHandler, SOUND_KEY, eSetBits);//�������¼��� 00000010
      }
    }
		longPressed = pdFALSE; // ���ó������
		shortPressed = pdFALSE; // ���ö̰����
    vTaskDelay(pdMS_TO_TICKS(20));//��ʱ20ms
  }
}



////���������ж�
//void EXTI1_IRQHandler(void) {
//    if (EXTI_GetITStatus(EXTI_Line1) == SET) {
//			  taskENTER_CRITICAL();//�ٽ�������
//        EXTI->IMR &= ~EXTI_Line1;                    // �ر��жϷ�����
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == RESET) {
//            TIM_SetCounter(TIM2, 0);                 // ���ü�����
//					  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		 // ����TIM2�ж�
//            is_pressing = true;
//        }
//        EXTI_ClearITPendingBit(EXTI_Line1);
//				taskEXIT_CRITICAL();
//    }
//}

////��ʱ��2�жϺ���
//void TIM2_IRQHandler(void) {
//    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
//        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//        
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == RESET) {
//                InputData short_press = {Key_ID, BUTTON_SHORT};
//                xQueueSendFromISR(inputQueue, &short_press, NULL);
//								xTaskNotify(xBuzzer_TaskHandler, SOUND_KEY, eSetBits);//�������¼��� 00000010
//						taskENTER_CRITICAL();//�ٽ�������
//						TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);  // ֹͣ��ʱ���ж�
//            EXTI->IMR |= EXTI_Line1;                 // ���¿��������ж�
//						taskEXIT_CRITICAL();
//						
//        }
//    }
//}

