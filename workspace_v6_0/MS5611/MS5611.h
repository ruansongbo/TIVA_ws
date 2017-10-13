/*
 * MS5611.h
 *
 *  Created on: 2015年6月11日
 *      Author: ideapad
 */

#ifndef MS5611_H_
#define MS5611_H_
#include <stdbool.h>
#include <stdint.h>

#define  MS561101BA_SlaveAddress 0x77  //定义器件在IIC总线中的从地址

#define  MS561101BA_D1 0x40
#define  MS561101BA_D2 0x50
#define  MS561101BA_RST 0x1E

//#define  MS561101BA_D1_OSR_256 0x40
//#define  MS561101BA_D1_OSR_512 0x42
//#define  MS561101BA_D1_OSR_1024 0x44
//#define  MS561101BA_D1_OSR_2048 0x46
#define  MS561101BA_D1_OSR_4096 0x48

//#define  MS561101BA_D2_OSR_256 0x50
//#define  MS561101BA_D2_OSR_512 0x52
//#define  MS561101BA_D2_OSR_1024 0x54
//#define  MS561101BA_D2_OSR_2048 0x56
#define  MS561101BA_D2_OSR_4096 0x58

#define  MS561101BA_ADC_RD 0x00
#define  MS561101BA_PROM_RD 0xA0
#define  MS561101BA_PROM_CRC 0xAE

void MS561101BA_RESET(void);
void MS561101BA_PROM_READ(void);
uint32_t MS561101BA_DO_CONVERSION(uint8_t command);
void MS561101BA_GetTemperature(uint8_t OSR_Temp);
void MS561101BA_GetPressure(uint8_t OSR_Pres);
void MS561101BA_Init(void);






#endif /* MS5611_H_ */
