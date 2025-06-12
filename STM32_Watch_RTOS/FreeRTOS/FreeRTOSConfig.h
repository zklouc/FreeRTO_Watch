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

// 是否启用抢占式调度机制
// 1：启用抢占式调度，高优先级任务可以中断低优先级任务的执行
// 0：不启用抢占式调度（不推荐，可能会导致实时性问题）
#define configUSE_PREEMPTION        1

// 是否启用空闲任务钩子（Idle Hook）
// 空闲任务钩子是一个回调函数，当系统处于空闲状态时会被调用
// 0：不启用空闲任务钩子
#define configUSE_IDLE_HOOK         0

// 是否启用时钟中断钩子（Tick Hook）
// 时钟中断钩子是一个回调函数，每次时钟中断时会被调用
// 0：不启用时钟中断钩子
#define configUSE_TICK_HOOK         0

// CPU 时钟频率，单位为 Hz
// 这里设置为 72MHz
#define configCPU_CLOCK_HZ          ((unsigned long) 72000000)

// 系统时钟中断频率，单位为 Hz
// 这里设置为 1000Hz，即每秒触发 1000 次时钟中断
#define configTICK_RATE_HZ          ((TickType_t) 1000)

// 系统支持的最大任务优先级数量
// 这里设置为 5，表示优先级范围为 0 到 4
#define configMAX_PRIORITIES        (5)

// 任务的最小堆栈大小，单位为字节
// 这里设置为 128 字节
#define configMINIMAL_STACK_SIZE    ((unsigned short) 128)

// 系统堆的总大小，单位为字节
// 这里设置为 3KB
#define configTOTAL_HEAP_SIZE       ((size_t) (3 * 1024))

// 任务名称的最大长度，单位为字符
// 这里设置为 16 个字符
#define configMAX_TASK_NAME_LEN     (16)

// 是否启用跟踪功能
// 0：不启用
#define configUSE_TRACE_FACILITY    0

// 是否使用 16 位计数器记录时钟中断次数
// 0：使用 32 位计数器
#define configUSE_16_BIT_TICKS      0

// 空闲任务是否应该让出 CPU
// 1：空闲任务会主动让出 CPU，允许其他同优先级任务运行
#define configIDLE_SHOULD_YIELD     1

// 是否启用互斥量（Mutex）功能
// 1：启用互斥量功能
#define configUSE_MUTEXES           1 // 启用互斥量功能

/* 是否包含以下 API 函数，设置为 1 表示包含，设置为 0 表示不包含 */

// 是否包含 vTaskPrioritySet API 函数，用于动态设置任务优先级
#define INCLUDE_vTaskPrioritySet    1

// 是否包含 uxTaskPriorityGet API 函数，用于获取任务优先级
#define INCLUDE_uxTaskPriorityGet   1

// 是否包含 vTaskDelete API 函数，用于删除任务
#define INCLUDE_vTaskDelete         1

// 是否包含 vTaskCleanUpResources API 函数，用于清理任务资源
#define INCLUDE_vTaskCleanUpResources 0

// 是否包含 vTaskSuspend API 函数，用于挂起任务
#define INCLUDE_vTaskSuspend        1

// 是否包含 vTaskDelayUntil API 函数，用于基于时间的延迟
#define INCLUDE_vTaskDelayUntil     1

// 是否包含 vTaskDelay API 函数，用于任务延迟
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

