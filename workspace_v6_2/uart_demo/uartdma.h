/*
 * uartdma.h
 *
 *  Created on: 2016年5月23日
 *      Author: Z
 */

#ifndef UARTDMA_H_
#define UARTDMA_H_


void Zigbeeuartdmainit();		//Zigbee Uartdma初始化函数，使用前必须调用
void uart0dmatx(void *u0txbuff);		//Uart0 dma重新发送

#endif /* UARTDMA_H_ */
