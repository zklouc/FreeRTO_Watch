#ifndef __MYI2C_H
#define __MYI2C_H


#define RCC_DS3231_GPIO RCC_APB2Periph_GPIOB
#define DS3231_GPIO GPIOB
#define DS3231_SCL_PIN GPIO_Pin_14
#define DS3231_SDA_PIN GPIO_Pin_15

void MyI2C_Init(void);
void MyI2C_Start(void);
void MyI2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);

#endif
