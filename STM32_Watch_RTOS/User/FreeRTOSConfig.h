/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

// �Ƿ�������ռʽ���Ȼ���
// 1��������ռʽ���ȣ������ȼ���������жϵ����ȼ������ִ��
// 0����������ռʽ���ȣ����Ƽ������ܻᵼ��ʵʱ�����⣩
#define configUSE_PREEMPTION        1

// �Ƿ����ÿ��������ӣ�Idle Hook��
// ������������һ���ص���������ϵͳ���ڿ���״̬ʱ�ᱻ����
// 0�������ÿ���������
#define configUSE_IDLE_HOOK         0

// �Ƿ�����ʱ���жϹ��ӣ�Tick Hook��
// ʱ���жϹ�����һ���ص�������ÿ��ʱ���ж�ʱ�ᱻ����
// 0��������ʱ���жϹ���
#define configUSE_TICK_HOOK         0

// CPU ʱ��Ƶ�ʣ���λΪ Hz
// ��������Ϊ 72MHz
#define configCPU_CLOCK_HZ          ((unsigned long) 72000000)

// ϵͳʱ���ж�Ƶ�ʣ���λΪ Hz
// ��������Ϊ 1000Hz����ÿ�봥�� 1000 ��ʱ���ж�
#define configTICK_RATE_HZ          ((TickType_t) 1000)

// ϵͳ֧�ֵ�����������ȼ�����
// ��������Ϊ 5����ʾ���ȼ���ΧΪ 0 �� 4
#define configMAX_PRIORITIES        (10)

// �������С��ջ��С����λΪ�ֽ�
// ��������Ϊ 128 �ֽ�
#define configMINIMAL_STACK_SIZE    ((unsigned short) 128)

// ϵͳ�ѵ��ܴ�С����λΪ�ֽ�
// ��������Ϊ 3KB
#define configTOTAL_HEAP_SIZE       ((size_t) (7 * 1024))

// �������Ƶ���󳤶ȣ���λΪ�ַ�
// ��������Ϊ 16 ���ַ�
#define configMAX_TASK_NAME_LEN     (16)

// �Ƿ����ø��ٹ���
// 0��������
#define configUSE_TRACE_FACILITY    0

// �Ƿ�ʹ�� 16 λ��������¼ʱ���жϴ���
// 0��ʹ�� 32 λ������
#define configUSE_16_BIT_TICKS      0

// ���������Ƿ�Ӧ���ó� CPU
// 1����������������ó� CPU����������ͬ���ȼ���������
#define configIDLE_SHOULD_YIELD     1

// �Ƿ����û�������Mutex������
// 1�����û���������
#define configUSE_MUTEXES           1 // ���û���������

/* �Ƿ�������� API ����������Ϊ 1 ��ʾ����������Ϊ 0 ��ʾ������ */

// �Ƿ���� vTaskPrioritySet API ���������ڶ�̬�����������ȼ�
#define INCLUDE_vTaskPrioritySet    1

// �Ƿ���� uxTaskPriorityGet API ���������ڻ�ȡ�������ȼ�
#define INCLUDE_uxTaskPriorityGet   1

// �Ƿ���� vTaskDelete API ����������ɾ������
#define INCLUDE_vTaskDelete         1

// �Ƿ���� vTaskCleanUpResources API ��������������������Դ
#define INCLUDE_vTaskCleanUpResources 0

// �Ƿ���� vTaskSuspend API ���������ڹ�������
#define INCLUDE_vTaskSuspend        1

// �Ƿ���� vTaskDelayUntil API ���������ڻ���ʱ����ӳ�
#define INCLUDE_vTaskDelayUntil     1

// �Ƿ���� vTaskDelay API ���������������ӳ�
#define INCLUDE_vTaskDelay          1

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY 		255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15

#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */

