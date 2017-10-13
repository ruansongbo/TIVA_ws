/*
 * uartdma.h
 *
 *  Created on: 2016年5月23日
 *      Author: Z
 */

#ifndef UARTDMA_H_
#define UARTDMA_H_

static unsigned char ucTxBuff[50]={
0xff,0x12,0x23,0x34,0x45,0x56,0x67,0x78,0x89,
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
};

static unsigned char raTxBuff[50]={
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
};


static void UART0IntHandler();	//UART0中断函数，接收完成中断、发送完成中断
void Zigbeeuartdmainit();		//Zigbee Uartdma初始化函数，使用前必须调用
void uart0dmarx();		//Uart0 dma重新接收
void uart0dmatx(void *u0txbuff);		//Uart0 dma重新发送

static void UART1IntHandler();	//UART0中断函数，接收完成中断、发送完成中断
void Raspberryuartdmainit();		//Zigbee Uartdma初始化函数，使用前必须调用
void uart1dmarx();		//Uart1 dma重新接收
void uart1dmatx(void *u1txbuff);		//Uart1 dma重新发送


#endif /* UARTDMA_H_ */
