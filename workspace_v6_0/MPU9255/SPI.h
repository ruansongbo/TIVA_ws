/*
 * SPI.h
 *
 *  Created on: 2016Äê11ÔÂ19ÈÕ
 *      Author: bb
 */

#ifndef SPI_H_
#define SPI_H_
#include <stdint.h>
#include <stdbool.h>

#define SSI_BitRate 1000000

void SPI_TM4C123_init(void);
uint8_t SPI_RW(uint8_t value);
uint8_t SPI_Write_register(uint8_t WriteReg,uint8_t Value);
uint8_t SPI_Write_Buf(uint8_t WriteReg, uint8_t *pBuf, uint8_t bytes);
uint8_t SPI_Read_register(uint8_t reg);
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars);

#endif /* SPI_H_ */
