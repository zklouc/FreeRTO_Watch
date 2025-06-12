#include "stm32f10x.h"                  // Device header
#include "MAX30102.h"

/**
  * 函    数：MAX30102等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void MAX30102_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
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
  * 函    数：MAX30102写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MAX30102手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MAX30102_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C1, RegAddress);											//硬件I2C发送寄存器地址
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2C1, Data);												//硬件I2C发送数据
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2C1, ENABLE);											//硬件I2C生成终止条件
}
 
/**
  * 函    数：MAX30102读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MAX30102手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t MAX30102_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C1, RegAddress);											//硬件I2C发送寄存器地址
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成重复起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6
	
	I2C_AcknowledgeConfig(I2C1, DISABLE);									//在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2C1, ENABLE);											//在接收最后一个字节之前提前申请停止条件
	
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);				//等待EV7
	Data = I2C_ReceiveData(I2C1);											//接收数据寄存器
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作
	
	return Data;
}

/**
  * 函    数：MAX30102写多个寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MAX30102手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MAX30102_WriteRegS(uint8_t RegAddress, uint8_t* Data,uint8_t count)
{
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C1, RegAddress);											//硬件I2C发送寄存器地址
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
 // 依次发送指定数量的字节
	for (uint8_t i = 0; i < count; i++) {
			I2C_SendData(I2C1, Data[i]);
			MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	}
	I2C_GenerateSTOP(I2C1, ENABLE);											//硬件I2C生成终止条件
}
 
/**
  * 函    数：MAX30102读多个寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MAX30102手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
void MAX30102_ReadRegS(uint8_t RegAddress,uint8_t* Data,uint8_t count)
{
	
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C1, RegAddress);											//硬件I2C发送寄存器地址
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成重复起始条件
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, I2C_WRITE_ADDR, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6

  for (uint8_t i = 0; i < count; i++) 
	{
        if (i == count - 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE); // 禁用ACK，准备接收最后一个字节
        }
        MAX30102_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED); // 等待EV7
        Data[i] = I2C_ReceiveData(I2C1); // 接收数据
   }
  I2C_GenerateSTOP(I2C1, ENABLE); // 生成停止条件
  I2C_AcknowledgeConfig(I2C1, ENABLE); // 恢复ACK使能
}


//传感器初始化
/**
  * 函    数：MAX30102初始化
  * 参    数：无
  * 返 回 值：无
  */
void MAX30102_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);		//开启I2C1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB6和PB7引脚初始化为复用开漏输出
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);  //配置INT中断引脚
	
	
//	//配置INT引脚为外部中断
//	/*AFIO选择中断引脚*/
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);//将外部中断的14号线映射到GPIOB，即选择PB14为外部中断引脚
//	
//	/*EXTI初始化*/
//	EXTI_InitTypeDef EXTI_InitStructure;						//定义结构体变量
//	EXTI_InitStructure.EXTI_Line = EXTI_Line5;					//选择配置外部中断的14号线
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//指定外部中断线使能
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//指定外部中断线为中断模式
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//指定外部中断线为下降沿触发
//	EXTI_Init(&EXTI_InitStructure);								//将结构体变量交给EXTI_Init，配置EXTI外设
	
//	/*NVIC中断分组*/
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
//																//即抢占优先级范围：0~3，响应优先级范围：0~3
//																//此分组配置在整个工程中仅需调用一次
//																//若有多个中断，可以把此代码放在main函数内，while循环之前
//																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
//	
//	/*NVIC配置*/
//	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;		//选择配置NVIC的EXTI15_10线
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//指定NVIC线路的抢占优先级为1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//指定NVIC线路的响应优先级为1
//	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
//	
	/*I2C初始化*/
	I2C_InitTypeDef I2C_InitStructure;						//定义结构体变量
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//模式，选择为I2C模式
	I2C_InitStructure.I2C_ClockSpeed = 200000;				//时钟速度，选择为50KHz
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		//时钟占空比，选择Tlow/Thigh = 2
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//应答，选择使能
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//应答地址，选择7位，从机模式下才有效
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;				//自身地址，从机模式下才有效
	I2C_Init(I2C1, &I2C_InitStructure);						//将结构体变量交给I2C_Init，配置I2C1
	GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_SET);
	
	/*I2C使能*/
	I2C_Cmd(I2C1, ENABLE);									//使能I2C1，开始运行
	
	MAX30102_Reset();//复位
	//确保设备在开始正常工作之前处于一个干净、一致的状态，避免因之前的状态或错误配置导致的问题
//	MAX30102_WriteReg(REG_INTR_ENABLE_1,0xc0);	// INTR setting
	MAX30102_WriteReg(REG_INTR_ENABLE_1,0x80);	// INTR setting
	MAX30102_WriteReg(REG_INTR_ENABLE_2,0x00);  //禁用所有中断
	MAX30102_WriteReg(REG_FIFO_WR_PTR,0x00);  	//FIFO_WR_PTR[4:0]，写指针寄存器 REG_FIFO_WR_PTR 清零，确保从头开始读写FIFO
	MAX30102_WriteReg(REG_OVF_COUNTER,0x00);  	//OVF_COUNTER[4:0]，溢出计数器寄存器清零
	MAX30102_WriteReg(REG_FIFO_RD_PTR,0x00);  	//FIFO_RD_PTR[4:0]，读指针寄存器清零
	
	MAX30102_WriteReg(REG_FIFO_CONFIG,0x4c);  	//sample avg = 4, fifo rollover=false, fifo almost full = 17
	MAX30102_WriteReg(REG_MODE_CONFIG,0x03);  	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED，使用红光和红外光测量血氧饱和度
	MAX30102_WriteReg(REG_SPO2_CONFIG,0x2f);  	// SPO2_ADC range = 4096nA, SPO2 sample rate (400 Hz),LED pulseWidth (400uS)  最后以100hz填充FIFO
	MAX30102_WriteReg(REG_LED1_PA,0x32);   	//Choose value for ~ 10mA for LED1
	MAX30102_WriteReg(REG_LED2_PA,0x32);   	// Choose value for ~ 10mA for LED2
	MAX30102_WriteReg(REG_PILOT_PA,0x7f);   	// Choose value for ~ 25mA for Pilot LED
}	

uint8_t MAX30102_INT(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5);
}

//确保设备在开始正常工作之前处于一个干净、一致的状态，避免因之前的状态或错误配置导致的问题
void MAX30102_Reset(void)
{
	MAX30102_WriteReg(REG_MODE_CONFIG,0x40);
}


/**
 * @brief     
 * @param     Data_Ir_Led  用于存储红外光 LED 的采样值
 * @param     Data_Red_Led 用于存储红光 LED 的采样值
 * @retval      
 */
void MAX30102_FIFO_ReadWord(uint32_t *Data_Red_Led, uint32_t *Data_Ir_Led)
{
	*Data_Red_Led=0;//两个变量清零，确保在读取数据之前它们的值是干净的
	*Data_Ir_Led=0; 
  uint8_t data[6];  // 用于存储从FIFO读取的6个字节

	// 读取中断状态寄存器，清除中断标志
	MAX30102_ReadReg(REG_INTR_STATUS_1);//MAX30102的中断状态寄存器是“读清”类型的，即当你读取这些寄存器时，会自动清除其中的中断标志。
	MAX30102_ReadReg(REG_INTR_STATUS_2);

	MAX30102_ReadRegS(REG_FIFO_DATA,data,6);

	// 解析红光和红外光数据
	*Data_Red_Led = ((uint32_t)(data[0]&0x03 ) << 16) | (uint32_t)(data[1] << 8) | data[2];  // 红光数据
	*Data_Ir_Led =  ((uint32_t)(data[3]&0x03 ) << 16) | (uint32_t)(data[4] << 8) | data[5];  // 红外光数据
	//阈值（10000）滤波
	if(*Data_Red_Led<=10000)
	{
		*Data_Red_Led=0;
	}
	if(*Data_Ir_Led<=10000)
	{
		*Data_Ir_Led=0;
	}
}

/**
 * @brief     
 * @param     Data_Ir_Led  用于存储红外光 LED 的采样值
 * @param     Data_Red_Led 用于存储红光 LED 的采样值
 * @retval      
 */
void MAX30102_FIFO_ReadWordS(uint32_t *Data_Red_Led, uint32_t *Data_Ir_Led,uint16_t index,uint8_t count )
{
  uint8_t data[count*6];  // 用于存储从FIFO读取的6个字节
  taskENTER_CRITICAL();//临界区保护
		
	// 读取中断状态寄存器，清除中断标志
	MAX30102_ReadReg(REG_INTR_STATUS_1);//MAX30102的中断状态寄存器是“读清”类型的，即当你读取这些寄存器时，会自动清除其中的中断标志。
	MAX30102_ReadReg(REG_INTR_STATUS_2);
	
	MAX30102_ReadRegS(REG_FIFO_DATA,data,count*6);
  taskEXIT_CRITICAL();
	for(uint8_t i=0;i<count;i++)
	{
		// 解析红光和红外光数据
		Data_Red_Led[index+i] = ((uint32_t)(data[0+i*6]&0x03 ) << 16) | (uint32_t)(data[1+i*6] << 8) | data[2+i*6];  // 红光数据
		Data_Ir_Led [index+i] =  ((uint32_t)(data[3+i*6]&0x03 ) << 16) | (uint32_t)(data[4+i*6] << 8) | data[5+i*6];  // 红外光数据
		//阈值（10000）滤波
		if(Data_Red_Led[index+i]<=10000)
		{
			Data_Red_Led[index+i]=0;
		}
		if(Data_Ir_Led [index+i] <=10000)
		{
			Data_Ir_Led [index+i] =0;
		}
	}
	
}



const uint16_t auw_hamm[31]={ 41,    276,    512,    276,     41 }; //Hamm=  long16(512* hamming(5)');
//uch_spo2_table is computed as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t uch_spo2_table[184]={ 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99, 
                            99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
                            100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 
                            97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 
                            90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 
                            80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 
                            66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
                            49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29, 
                            28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 
                            3, 2, 1 } ;
static  int32_t an_dx[ BUFFER_SIZE-MA4_SIZE]; // delta
static  int32_t an_x[ BUFFER_SIZE]; //ir
static  int32_t an_y[ BUFFER_SIZE]; //red

void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer,  int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid, 
                              int32_t *pn_heart_rate, int8_t  *pch_hr_valid)
/**
* \brief        Calculate the heart rate and SpO2 level
* \par          Details
*               By detecting  peaks of PPG cycle and corresponding AC/DC of red/infra-red signal, the ratio for the SPO2 is computed.
*               Since this algorithm is aiming for Arm M0/M3. formaula for SPO2 did not achieve the accuracy due to register overflow.
*               Thus, accurate SPO2 is precalculated and save longo uch_spo2_table[] per each ratio.
*
* \param[in]    *pun_ir_buffer           - IR sensor data buffer
* \param[in]    n_ir_buffer_length      - IR sensor data buffer length
* \param[in]    *pun_red_buffer          - Red sensor data buffer
* \param[out]    *pn_spo2                - Calculated SpO2 value
* \param[out]    *pch_spo2_valid         - 1 if the calculated SpO2 value is valid
* \param[out]    *pn_heart_rate          - Calculated heart rate value
* \param[out]    *pch_hr_valid           - 1 if the calculated heart rate value is valid
*
* \retval       None
*/
{
    uint32_t un_ir_mean ,un_only_once ;
    int32_t k ,n_i_ratio_count;
    int32_t i, s, m, n_exact_ir_valley_locs_count ,n_middle_idx;
    int32_t n_th1, n_npks,n_c_min;      
    int32_t an_ir_valley_locs[15] ;
    int32_t an_exact_ir_valley_locs[15] ;
    int32_t an_dx_peak_locs[15] ;
    int32_t n_peak_interval_sum;
    
    int32_t n_y_ac, n_x_ac;
    int32_t n_spo2_calc; 
    int32_t n_y_dc_max, n_x_dc_max; 
    int32_t n_y_dc_max_idx, n_x_dc_max_idx; 
    int32_t an_ratio[5],n_ratio_average; 
    int32_t n_nume,  n_denom ;
    // remove DC of ir signal  平均值，用于去直流 
    un_ir_mean =0; 
    for (k=0 ; k<n_ir_buffer_length ; k++ ) un_ir_mean += pun_ir_buffer[k] ;
    un_ir_mean =un_ir_mean/n_ir_buffer_length ;
    for (k=0 ; k<n_ir_buffer_length ; k++ )  an_x[k] =  pun_ir_buffer[k] - un_ir_mean ; 
    
    // 4 pt Moving Average 4点移动平均,用于滤波
    for(k=0; k< BUFFER_SIZE-MA4_SIZE; k++){
        n_denom= ( an_x[k]+an_x[k+1]+ an_x[k+2]+ an_x[k+3]);
        an_x[k]=  n_denom/(int32_t)4; 
    }

    // get difference of smoothed IR signal
    //一阶差分
    for( k=0; k<BUFFER_SIZE-MA4_SIZE-1;  k++)
        an_dx[k]= (an_x[k+1]- an_x[k]);

    // 2-pt Moving Average to an_dx
    for(k=0; k< BUFFER_SIZE-MA4_SIZE-2; k++){
        an_dx[k] =  ( an_dx[k]+an_dx[k+1])/2 ;
    }
    
    // hamming window
    // flip wave form so that we can detect valley with peak detector
    // hamming window 构造窗函数
    // 波形翻转找波峰波谷
		for ( i=0 ; i<BUFFER_SIZE-HAMMING_SIZE-MA4_SIZE-2 ;i++){
        s= 0;
        for( k=i; k<i+ HAMMING_SIZE ;k++){
            s -= an_dx[k] *auw_hamm[k-i] ; 
                     }
        an_dx[i]= s/ (int32_t)1146; // divide by sum of auw_hamm 
    }

 
    n_th1=0; // threshold calculation	波形提取特征点的阈值
    for ( k=0 ; k<BUFFER_SIZE-HAMMING_SIZE ;k++){
        n_th1 += ((an_dx[k]>0)? an_dx[k] : ((int32_t)0-an_dx[k])) ;
    }
    n_th1= n_th1/ ( BUFFER_SIZE-HAMMING_SIZE);
    // peak location is acutally index for sharpest location of raw signal since we flipped the signal         
    maxim_find_peaks( an_dx_peak_locs, &n_npks, an_dx, BUFFER_SIZE-HAMMING_SIZE, n_th1, 8, 5 );//找峰值高度,和相邻峰值距离  peak_height, peak_distance, max_num_peaks 

    n_peak_interval_sum =0;
    if (n_npks>=2){
        for (k=1; k<n_npks; k++)
            n_peak_interval_sum += (an_dx_peak_locs[k]-an_dx_peak_locs[k -1]);
        n_peak_interval_sum=n_peak_interval_sum/(n_npks-1);
        *pn_heart_rate=(int32_t)(6000/n_peak_interval_sum);// beats per minutes
        *pch_hr_valid  = 1;
    }
    else  {
        *pn_heart_rate = -999;
        *pch_hr_valid  = 0;
    }
    //初始数据波谷位置修正        
    for ( k=0 ; k<n_npks ;k++)
        an_ir_valley_locs[k]=an_dx_peak_locs[k]+HAMMING_SIZE/2; 


    // raw value : RED(=y) and IR(=X)
    // we need to assess DC and AC value of ir and red PPG. 
    for (k=0 ; k<n_ir_buffer_length ; k++ )  {
        an_x[k] =  pun_ir_buffer[k] ; 
        an_y[k] =  pun_red_buffer[k] ; 
    }

    // find precise min near an_ir_valley_locs
		//精确的查找位置减小spo2误差
    n_exact_ir_valley_locs_count =0; 
    for(k=0 ; k<n_npks ;k++){
        un_only_once =1;
        m=an_ir_valley_locs[k];
        n_c_min= 16777216;//2^24;
        if (m+5 <  BUFFER_SIZE-HAMMING_SIZE  && m-5 >0){
            for(i= m-5;i<m+5; i++)
                if (an_x[i]<n_c_min){
                    if (un_only_once >0){
                       un_only_once =0;
                   } 
                   n_c_min= an_x[i] ;
                   an_exact_ir_valley_locs[k]=i;
                }
            if (un_only_once ==0)
                n_exact_ir_valley_locs_count ++ ;
        }
    }
		//波谷小于2个无法计算spo2
    if (n_exact_ir_valley_locs_count <2 ){
       *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
       *pch_spo2_valid  = 0; 
       return;
    }
    // 4 pt MA
    for(k=0; k< BUFFER_SIZE-MA4_SIZE; k++){
        an_x[k]=( an_x[k]+an_x[k+1]+ an_x[k+2]+ an_x[k+3])/(int32_t)4;
        an_y[k]=( an_y[k]+an_y[k+1]+ an_y[k+2]+ an_y[k+3])/(int32_t)4;
    }

    //using an_exact_ir_valley_locs , find ir-red DC andir-red AC for SPO2 calibration ratio
    //finding AC/DC maximum of raw ir * red between two valley locations
    n_ratio_average =0; 
    n_i_ratio_count =0; 
    
    for(k=0; k< 5; k++) an_ratio[k]=0;
    for (k=0; k< n_exact_ir_valley_locs_count; k++){
        if (an_exact_ir_valley_locs[k] > BUFFER_SIZE ){             
            *pn_spo2 =  -999 ; // do not use SPO2 since valley loc is out of range
            *pch_spo2_valid  = 0; 
            return;
        }
    }
    // find max between two valley locations 
    // and use ratio betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2 

    for (k=0; k< n_exact_ir_valley_locs_count-1; k++){
        n_y_dc_max= -16777216 ; 
        n_x_dc_max= - 16777216; 
        if (an_exact_ir_valley_locs[k+1]-an_exact_ir_valley_locs[k] >10){
            for (i=an_exact_ir_valley_locs[k]; i< an_exact_ir_valley_locs[k+1]; i++){
                if (an_x[i]> n_x_dc_max) {n_x_dc_max =an_x[i];n_x_dc_max_idx =i; }
                if (an_y[i]> n_y_dc_max) {n_y_dc_max =an_y[i];n_y_dc_max_idx=i;}
            }
            n_y_ac= (an_y[an_exact_ir_valley_locs[k+1]] - an_y[an_exact_ir_valley_locs[k] ] )*(n_y_dc_max_idx -an_exact_ir_valley_locs[k]); //red
            n_y_ac=  an_y[an_exact_ir_valley_locs[k]] + n_y_ac/ (an_exact_ir_valley_locs[k+1] - an_exact_ir_valley_locs[k])  ; 
        
        
            n_y_ac=  an_y[n_y_dc_max_idx] - n_y_ac;    // subracting linear DC compoenents from raw 
            n_x_ac= (an_x[an_exact_ir_valley_locs[k+1]] - an_x[an_exact_ir_valley_locs[k] ] )*(n_x_dc_max_idx -an_exact_ir_valley_locs[k]); // ir
            n_x_ac=  an_x[an_exact_ir_valley_locs[k]] + n_x_ac/ (an_exact_ir_valley_locs[k+1] - an_exact_ir_valley_locs[k]); 
            n_x_ac=  an_x[n_y_dc_max_idx] - n_x_ac;      // subracting linear DC compoenents from raw 
            n_nume=( n_y_ac *n_x_dc_max)>>7 ; //prepare X100 to preserve floating value
            n_denom= ( n_x_ac *n_y_dc_max)>>7;
            if (n_denom>0  && n_i_ratio_count <5 &&  n_nume != 0)
            {   
                an_ratio[n_i_ratio_count]= (n_nume*20)/n_denom ; //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max) ;  ///*************************n_nume原来是*100************************//
                n_i_ratio_count++;
            }
        }
    }

    maxim_sort_ascend(an_ratio, n_i_ratio_count);
    n_middle_idx= n_i_ratio_count/2;

    if (n_middle_idx >1)
        n_ratio_average =( an_ratio[n_middle_idx-1] +an_ratio[n_middle_idx])/2; // use median
    else
        n_ratio_average = an_ratio[n_middle_idx ];

    if( n_ratio_average>2 && n_ratio_average <184){
        n_spo2_calc= uch_spo2_table[n_ratio_average] ;
        *pn_spo2 = n_spo2_calc ;
        *pch_spo2_valid  = 1;//  float_SPO2 =  -45.060*n_ratio_average* n_ratio_average/10000 + 30.354 *n_ratio_average/100 + 94.845 ;  // for comparison with table
    }
    else{
        *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
        *pch_spo2_valid  = 0; 
    }
}


void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num)
/**
* \brief        Find peaks，在信号中查找峰值，这些峰值对应于脉搏波
* \par          Details
*               Find at most MAX_NUM peaks above MIN_HEIGHT separated by at least MIN_DISTANCE
*								信号数组、最小高度、最小距离和最大峰值数量作为参数
								调整峰值的位置和数量
* \retval       None
*/
{
    maxim_peaks_above_min_height( pn_locs, pn_npks, pn_x, n_size, n_min_height );
    maxim_remove_close_peaks( pn_locs, pn_npks, pn_x, n_min_distance );
    *pn_npks = min( *pn_npks, n_max_num );
}

void maxim_peaks_above_min_height(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x, int32_t n_size, int32_t n_min_height)
/**
* \brief        Find peaks above n_min_height 筛选出高于最小高度的峰值，只有足够高的峰值被考虑，以减少噪声的影响
* \par          Details 
*               Find all peaks above MIN_HEIGHT
*
* \retval       None
*/
{
    int32_t i = 1, n_width;
    *pn_npks = 0;
    
    while (i < n_size-1){
        if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i-1]){            // find left edge of potential peaks
            n_width = 1;
            while (i+n_width < n_size && pn_x[i] == pn_x[i+n_width])    // find flat peaks
                n_width++;
            if (pn_x[i] > pn_x[i+n_width] && (*pn_npks) < 15 ){                            // find right edge of peaks
                pn_locs[(*pn_npks)++] = i;        
                // for flat peaks, peak location is left edge
                i += n_width+1;
            }
            else
                i += n_width;
        }
        else
            i++;
    }
}


void maxim_remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_min_distance)
/**
* \brief        Remove peaks 移除距离过近的峰值，以确保每个峰值代表一个独立的脉搏波
* \par          Details
*               Remove peaks separated by less than MIN_DISTANCE
*
* \retval       None
*/
{
    
    int32_t i, j, n_old_npks, n_dist;
    
    /* Order peaks from large to small */
    maxim_sort_indices_descend( pn_x, pn_locs, *pn_npks );

    for ( i = -1; i < *pn_npks; i++ ){
        n_old_npks = *pn_npks;
        *pn_npks = i+1;
        for ( j = i+1; j < n_old_npks; j++ ){
            n_dist =  pn_locs[j] - ( i == -1 ? -1 : pn_locs[i] ); // lag-zero peak of autocorr is at index -1
            if ( n_dist > n_min_distance || n_dist < -n_min_distance )
                pn_locs[(*pn_npks)++] = pn_locs[j];
        }
    }

    // Resort indices longo ascending order
    maxim_sort_ascend( pn_locs, *pn_npks );
}

void maxim_sort_ascend(int32_t *pn_x,int32_t n_size) 
/**
* \brief        Sort array 该函数将峰值按升序排序，排序后的峰值有助于更准确地计算心率
* \par          Details
*               Sort array in ascending order (insertion sort algorithm)
*
* \retval       None
*/
{
    int32_t i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_x[i];
        for (j = i; j > 0 && n_temp < pn_x[j-1]; j--)
            pn_x[j] = pn_x[j-1];
        pn_x[j] = n_temp;
    }
}

void maxim_sort_indices_descend(int32_t *pn_x, int32_t *pn_indx, int32_t n_size)
/**
* \brief        Sort indices 该函数根据某个标准（如峰值高度）对峰值索引进行降序排序，有助于选择最重要的峰值进行心率和血氧饱和度的计算
* \par          Details
*               Sort indices according to descending order (insertion sort algorithm)
*
* \retval       None
*/ 
{
    int32_t i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_indx[i];
        for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j-1]]; j--)
            pn_indx[j] = pn_indx[j-1];
        pn_indx[j] = n_temp;
    }
}



