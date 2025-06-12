#ifndef __ADXL345__H
#define __ADXL345__H

#define	ADXL345_ADDRESS   0xA6	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改         
//ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A


#define DEVICE_ID           0X00        //获取器件ID,0XE5
#define THRESH_TAP          0X1D        //敲击阀值
#define OFSX                0X1E		//x轴调整偏移值
#define OFSY                0X1F
#define OFSZ                0X20
#define DUR                 0X21
#define Latent              0X22
#define Window              0X23
#define THRESH_ACK          0X24
#define THRESH_INACT        0X25
#define TIME_INACT          0X26
#define ACT_INACT_CTL       0X27
#define THRESH_FF           0X28
#define TIME_FF             0X29
#define TAP_AXES            0X2A
#define ACT_TAP_STATUS      0X2B
#define BW_RATE             0X2C
#define POWER_CTL           0X2D
#define INT_ENABLE          0X2E
#define INT_MAP             0X2F
#define INT_SOURCE          0X30
#define DATA_FORMAT        0X31
#define DATA_X0            0X32
#define DATA_X1            0X33
#define DATA_Y0            0X34
#define DATA_Y1            0X35
#define DATA_Z0            0X36
#define DATA_Z1            0X37
#define FIFO_CTL            0X38
#define FIFO_STATUS         0X39
 
#define I_M_DEVID      ((uint8_t)0xE5) //器件ID=0XE5
 
#define TIMES 4
#define MAX(a,b) ((a) > (b) ? (a) : (b)) 
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define SAMPLE_SIZE         50
#define ABS(a) (0 - (a)) > 0 ? (-(a)) : (a)
#define DYNAMIC_PRECISION     			30   	 /*动态精度*/
#define MOST_ACTIVE_NULL      			0      	 /*未找到最活跃轴*/
#define MOST_ACTIVE_X					1		 /*最活跃轴X*/	
#define MOST_ACTIVE_Y					2        /*最活跃轴Y*/	
#define MOST_ACTIVE_Z					3        /*最活跃轴Z*/	
 
//加速度ad值取平均后数据
typedef struct {
  float x;
  float y;
  float z;
}Aver_Acc;		
 
//获得样本集中最大和最小值及差值
typedef struct {
	Aver_Acc newmax;
	Aver_Acc newmin;
	Aver_Acc D_Value;
}Peak_Value;

/*一个线性移位寄存器，通过监测变化是否大于动态精度，用于过滤高频噪声*/
typedef struct slid_reg{
	Aver_Acc new_sample;
	Aver_Acc old_sample;
}Slid_Reg;

void ADXL345_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t ADXL345_ReadReg(uint8_t RegAddress);
void ADXL345_WriteRegS(uint8_t RegAddress, uint8_t* Data,uint8_t count);
void ADXL345_ReadRegS(uint8_t RegAddress,uint8_t *Data,uint8_t Count);
void ADXL345_Init(void);
void ADXL345_GetData(int16_t *x, int16_t *y, int16_t *z);
void ADXL345_Average(Aver_Acc* aver_data);
void peak_update(Peak_Value *peak, Aver_Acc* aver_data,uint8_t *Active_Axis);//更新最大值和最小值用于更新动态阈值和最活跃轴
void slid_update(Slid_Reg *slid, Aver_Acc *cur_sample,uint8_t Active_Axis);//滤除高频噪声
int8_t Detect_Step(Peak_Value *peak, Slid_Reg *slid, Aver_Acc *cur_sample,uint8_t *Active_Axis);//判断是否走步
#endif

