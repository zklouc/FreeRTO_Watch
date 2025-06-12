#ifndef __KEY_H
#define __KEY_H
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"  // 这个头文件包含了与信号量相关的所有函数和类型定义
#include "DS3231.h"

#define Key_ID  1
#define ENC_ID  2
#define BUTTON_SHORT  1
#define BUTTON_LONG   2


// 编码器状态跟踪
typedef struct {
    uint8_t id;//identification 表示数据标识，为1表示按键数据，为2表示旋转编码器数据
    int8_t count;//数据值
} InputData;

void TIM2_Init(void);
void Key_Init(void);
void Key_Task(void *params);
void Encoder_Init(void);



#endif
