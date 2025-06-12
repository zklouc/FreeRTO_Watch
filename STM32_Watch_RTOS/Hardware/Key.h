#ifndef __KEY_H
#define __KEY_H
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"  // ���ͷ�ļ����������ź�����ص����к��������Ͷ���
#include "DS3231.h"

#define Key_ID  1
#define ENC_ID  2
#define BUTTON_SHORT  1
#define BUTTON_LONG   2


// ������״̬����
typedef struct {
    uint8_t id;//identification ��ʾ���ݱ�ʶ��Ϊ1��ʾ�������ݣ�Ϊ2��ʾ��ת����������
    int8_t count;//����ֵ
} InputData;

void TIM2_Init(void);
void Key_Init(void);
void Key_Task(void *params);
void Encoder_Init(void);



#endif
