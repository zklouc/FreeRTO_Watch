#include "stm32f10x.h"                  // Device header
#include "ADXL345.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"
/**
  * 函    数：ADXL345等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void ADXL345_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000;									//给定超时计数时间
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)	//循环等待指定事件
	{
		Timeout --;										//等待时，计数值自减
		if (Timeout == 0)								//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;										//跳出等待，不等了
		}
	}
}

/**
  * 函    数：ADXL345写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考ADXL345手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void ADXL345_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2C2, Data);												//硬件I2C发送数据
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2C2, ENABLE);											//硬件I2C生成终止条件
}
 
/**
  * 函    数：ADXL345读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考ADXL345手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t ADXL345_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成重复起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6
	
	I2C_AcknowledgeConfig(I2C2, DISABLE);									//在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2C2, ENABLE);											//在接收最后一个字节之前提前申请停止条件
	
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED);				//等待EV7
	Data = I2C_ReceiveData(I2C2);											//接收数据寄存器
	
	I2C_AcknowledgeConfig(I2C2, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作
	
	return Data;
}

/**
  * 函    数：ADXL345写多个寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考ADXL345手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void ADXL345_WriteRegS(uint8_t RegAddress, uint8_t* Data,uint8_t count)
{
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
 // 依次发送指定数量的字节
	for (uint8_t i = 0; i < count; i++) {
			I2C_SendData(I2C2, Data[i]);
			ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	}
	I2C_GenerateSTOP(I2C2, ENABLE);											//硬件I2C生成终止条件
}
 
/**
  * 函    数：ADXL345读多个寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考ADXL345手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
void ADXL345_ReadRegS(uint8_t RegAddress,uint8_t* Data,uint8_t count)
{
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成重复起始条件
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, ADXL345_ADDRESS, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6

  for (uint8_t i = 0; i < count; i++) {
        if (i == count - 1) {
            I2C_AcknowledgeConfig(I2C2, DISABLE); // 禁用ACK，准备接收最后一个字节
        }
        ADXL345_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED); // 等待EV7
        Data[i] = I2C_ReceiveData(I2C2); // 接收数据
    }

  I2C_GenerateSTOP(I2C2, ENABLE); // 生成停止条件
  I2C_AcknowledgeConfig(I2C2, ENABLE); // 恢复ACK使能
}


//传感器初始化
/**
  * 函    数：ADXL345初始化
  * 参    数：无
  * 返 回 值：无
  */
void ADXL345_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);		//开启I2C2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为复用开漏输出
	
	/*I2C初始化*/
	I2C_InitTypeDef I2C_InitStructure;						//定义结构体变量
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//模式，选择为I2C模式
	I2C_InitStructure.I2C_ClockSpeed = 40000;				//时钟速度，选择为50KHz
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		//时钟占空比，选择Tlow/Thigh = 2
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//应答，选择使能
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//应答地址，选择7位，从机模式下才有效
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;				//自身地址，从机模式下才有效
	I2C_Init(I2C2, &I2C_InitStructure);						//将结构体变量交给I2C_Init，配置I2C2
	
	/*I2C使能*/
	I2C_Cmd(I2C2, ENABLE);									//使能I2C2，开始运行
	ADXL345_WriteReg(DATA_FORMAT,0X0B);	//高电平中断输出,13位全分辨率,输出数据右对齐,16g量程, 最小分辨率3.9mg/LSB
	ADXL345_WriteReg(BW_RATE,0x09);		  //数据输出速度为100Hz
	ADXL345_WriteReg(POWER_CTL,0x28);	  //链接使能,测量模式
	ADXL345_WriteReg(INT_ENABLE,0x00);	//不使用中断		 
	ADXL345_WriteReg(OFSX,0x00);				//X 偏移量
	ADXL345_WriteReg(OFSY,0x00);				//Y 偏移量
	ADXL345_WriteReg(OFSZ,0x00);				//Z 偏移量
}


//读取三个方向的加速度
void ADXL345_GetData(int16_t *x, int16_t *y, int16_t *z)
{
  uint8_t BUF[6];
	ADXL345_ReadRegS(DATA_X0 ,BUF,6);
 
    *x = ((u16)BUF[1] << 8) + BUF[0];
    *y = ((u16)BUF[3] << 8) + BUF[2];
    *z = ((u16)BUF[5] << 8) + BUF[4];
}


/**
  * 函    数：连读读取几次取平均值函数
  * 参    数：times 取平均值的次数
  * 返 回 值：无
  */
void ADXL345_Average(Aver_Acc* aver_data)
{
	uint8_t i;
	short tx,ty,tz;
	aver_data->x=0;
	aver_data->y=0;
	aver_data->z=0;
	if(TIMES)//读取次数不为0
	{
		for(i=0;i<TIMES;i++)//连续读取times次
		{
			taskENTER_CRITICAL();		
			ADXL345_GetData(&tx,&ty,&tz);
			taskEXIT_CRITICAL();
			aver_data->x+=tx;
			aver_data->y+=ty;
			aver_data->z+=tz;
			vTaskDelay(5);
		}
		aver_data->x/=TIMES;
		aver_data->y/=TIMES;
		aver_data->z/=TIMES;
	}
}

/**
  * 函    数：更新最大值、最小值和最活跃轴  用于更新动态阈值
  * 参    数：times 取平均值的次数
  * 返 回 值：无
  */
void peak_update(Peak_Value *peak, Aver_Acc* aver_data,uint8_t *Active_Axis)
{
	static uint8_t i;
	short min_value = -32768;
	short max_value = 32767;
	if(i==0)
	{
		peak->newmax.x = min_value;//使理论最大值为最小值
		peak->newmax.y = min_value;
		peak->newmax.z = min_value;

		peak->newmin.x = max_value;
		peak->newmin.y = max_value;
		peak->newmin.z = max_value;
	}
	peak->newmax.x = MAX(peak->newmax.x, aver_data->x);//比较得出新的实际最大值
	peak->newmax.y = MAX(peak->newmax.y, aver_data->y);
	peak->newmax.z = MAX(peak->newmax.z, aver_data->z);

	peak->newmin.x = MIN(peak->newmin.x, aver_data->x);//比较得出新的最小值
	peak->newmin.y = MIN(peak->newmin.y, aver_data->y);
	peak->newmin.z = MIN(peak->newmin.z, aver_data->z);
	
	i+=1;
	if(i==30)//如果已经采样比较30次了
	{
		i=0;//重置i,开始下一次判断周期
		//得出差值
		peak->D_Value.x=(peak->newmax.x-peak->newmin.x);
		peak->D_Value.y=(peak->newmax.y-peak->newmin.y);
		peak->D_Value.z=(peak->newmax.z-peak->newmin.z);
		//得出最活跃轴 1-X,2-Y,3-Z 
		if(peak->D_Value.x > peak->D_Value.y)  
		{
			((peak->D_Value.x) > (peak->D_Value.z) ? (*Active_Axis=1) : (*Active_Axis=3));
		}
		else{
			((peak->D_Value.y) > (peak->D_Value.z) ? (*Active_Axis=2) : (*Active_Axis=3));
		}
	}
	
}

//
/**
  * 函    数：滤除高频噪声
  * 参    数：线性移位寄存器
  * 参    数：当前样本值
  * 参    数：最活跃轴
  * 返 回 值：无
  */
void slid_update(Slid_Reg *slid, Aver_Acc *cur_sample,uint8_t Active_Axis)
{
	switch(Active_Axis)
	{
		case 0: {break;}
		case 1: {if (ABS((cur_sample->x - slid->new_sample.x)) > DYNAMIC_PRECISION) {
							slid->old_sample.x = slid->new_sample.x;
							slid->new_sample.x = cur_sample->x;} 
						else {slid->old_sample.x = slid->new_sample.x;}
						break;}
		case 2:	{if (ABS((cur_sample->y - slid->new_sample.y)) > DYNAMIC_PRECISION) {
							slid->old_sample.y = slid->new_sample.y;
							slid->new_sample.y = cur_sample->y;}
					  else {slid->old_sample.y = slid->new_sample.y;}
						break;}
		case 3: {if (ABS((cur_sample->z - slid->new_sample.z)) > DYNAMIC_PRECISION) {
							slid->old_sample.z = slid->new_sample.z;
							slid->new_sample.z = cur_sample->z;} 
						else {slid->old_sample.z = slid->new_sample.z;}	
						break;}						
  }
}

/*判断是否走步*/
int8_t Detect_Step(Peak_Value *peak, Slid_Reg *slid, Aver_Acc *cur_sample,uint8_t *Active_Axis)
{
	static uint8_t step_cnt = 0;
//	ADXL345_Average(cur_sample);//获取取平均后的样本
	peak_update(peak, cur_sample,Active_Axis);//更新比较最大值和最小值，内部每50个样本更新一次最活跃轴
	slid_update(slid, cur_sample,*Active_Axis);//根据最活跃轴滤除高频噪声
	
	switch (*Active_Axis) {
		case MOST_ACTIVE_X: {
			float threshnew_x = (peak->newmax.x + peak->newmin.x) / 2;
			if (slid->old_sample.x > threshnew_x && slid->new_sample.x < threshnew_x)
			{
				step_cnt ++;
			}
			break;
		}
		case MOST_ACTIVE_Y: {
			float threshnew_y = (peak->newmax.y + peak->newmin.y) / 2;
			if (slid->old_sample.y > threshnew_y && slid->new_sample.y < threshnew_y) 
				{
					step_cnt ++;
				}
			break;
		}
		case MOST_ACTIVE_Z: {
			float threshnew_z = (peak->newmax.z + peak->newmin.z) / 2;
			if (slid->old_sample.z > threshnew_z && slid->new_sample.z < threshnew_z) 
				{
					step_cnt ++;
				}
			break;
		}
		default: 
			break;
	}
	return step_cnt;
}



