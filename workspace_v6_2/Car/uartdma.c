/*
 * uartdma.c
 *
 *  Created on: 2016年5月23日
 *      Author: Z
 */
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_uart.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "uart.h"
#include "udma.h"
#include "interrupt.h"
#include "utils.h"
#include "prcm.h"
#include "pin.h"
#include "Motor.h"
#include "string.h"

#include "uart_if.h"
#include "udma_if.h"
#include "uartdma.h"

unsigned char isintZigRx=0;
unsigned char isintRasRx=0;
char mark0=0;
char mark1=0;
unsigned char ucRxBuff[50]={

};
//ZigBee接收的data的Buffer

unsigned char raRxBuff[50]={

};
//Raspberry接收的data的Buffer
void Zigbeeuartdmainit()
{
	UDMAInit();
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
	MAP_PinTypeUART(PIN_03, PIN_MODE_7);   //GPIO_12,UART0_TX
	MAP_PinTypeUART(PIN_04, PIN_MODE_7);	//GPIO_13,UART0_RX
	MAP_UARTIntRegister(UARTA0_BASE,UART0IntHandler); //UART注册中断函数
    MAP_UARTConfigSetExpClk(CONSOLE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH),
	                            UART_BAUD_RATE,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                            UART_CONFIG_PAR_NONE));

    UARTFIFOLevelSet(UARTA0_BASE,UART_FIFO_TX1_8,UART_FIFO_RX1_8);

    UDMASetupTransfer(UDMA_CH9_UARTA0_TX,
                         UDMA_MODE_BASIC,
                         8,
                         UDMA_SIZE_8,
                         UDMA_ARB_2,
                         (void *)ucTxBuff,
                         UDMA_SRC_INC_8,
                         (void *)(UARTA0_BASE+UART_O_DR),
                         UDMA_DST_INC_NONE);
    MAP_UARTDMAEnable(UARTA0_BASE,UART_DMA_TX); //使能dma发送
    //MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMATX); //使能dma发送完成中断


    UDMASetupTransfer(UDMA_CH8_UARTA0_RX,
    				  UDMA_MODE_BASIC,
                      8,
                      UDMA_SIZE_8,
                      UDMA_ARB_2,
                      (void *)(UARTA0_BASE+UART_O_DR),
                      UDMA_SRC_INC_NONE,
                      (void *)ucRxBuff,
                      UDMA_DST_INC_8);
    MAP_UARTDMAEnable(UARTA0_BASE,UART_DMA_RX);	//使能dma接收
    MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMARX);	//使能dma接收完成中断
    isintZigRx=1;

}

void uart0dmarx()
{
    UDMASetupTransfer(UDMA_CH8_UARTA0_RX,
    				  UDMA_MODE_BASIC,
                      8,
                      UDMA_SIZE_8,
                      UDMA_ARB_2,
                      (void *)(UARTA0_BASE+UART_O_DR),
                      UDMA_SRC_INC_NONE,
                      (void *)ucRxBuff,
                      UDMA_DST_INC_8);
    MAP_UARTDMAEnable(UARTA0_BASE,UART_DMA_RX);	//使能dma接收
    MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMARX);	//使能dma接收完成中断
}

void uart0dmatx(void *u0txbuff)
{
	UDMASetupTransfer(UDMA_CH9_UARTA0_TX,
	                         UDMA_MODE_BASIC,
	                         8,
	                         UDMA_SIZE_8,
	                         UDMA_ARB_2,
	                         (void *)u0txbuff,
	                         UDMA_SRC_INC_8,
	                         (void *)(UARTA0_BASE+UART_O_DR),
	                         UDMA_DST_INC_NONE);
	    MAP_UARTDMAEnable(UARTA0_BASE,UART_DMA_TX); //使能dma发送
}

static void UART0IntHandler()	//UART中断函数，接收完成中断、发送完成中断
{

	// Disable UART RX DMA
	//
       // MAP_UARTDMADisable(UARTA0_BASE,UART_DMA_RX);

	//
	// Disable UART TX DMA
	//
    //MAP_UARTDMADisable(UARTA0_BASE,UART_DMA_TX);

    //
    // Clear the UART Interrupt
	MAP_UARTDMADisable(UARTA0_BASE,UART_DMA_RX);
	//UARTCharPut(CONSOLE,0x00);
    mark0=1;

    MAP_UARTIntClear(UARTA0_BASE,UART_INT_DMARX|UART_INT_DMATX);

}

//**********************************************************************************//
//***********************************我是淫荡的分割线************************************//
//******************************下面为树莓派UARTDMA通信代码********************************//
//**********************************************************************************//


void Raspberryuartdmainit()
{
	//UDMAInit();
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
	MAP_PinTypeUART(PIN_07, PIN_MODE_5);   //GPIO_16,UART1_TX
	MAP_PinTypeUART(PIN_08, PIN_MODE_5);	//GPIO_17,UART1_RX
	MAP_UARTIntRegister(UARTA1_BASE,UART1IntHandler); //UART注册中断函数
    MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1),
	                            UART_BAUD_RATE,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                            UART_CONFIG_PAR_NONE));

    UARTFIFOLevelSet(UARTA1_BASE,UART_FIFO_TX2_8,UART_FIFO_RX2_8);

    UDMASetupTransfer(UDMA_CH11_UARTA1_TX,
                         UDMA_MODE_BASIC,
                         8,
                         UDMA_SIZE_8,
                         UDMA_ARB_2,
                         (void *)raTxBuff,
                         UDMA_SRC_INC_8,
                         (void *)(UARTA1_BASE+UART_O_DR),
                         UDMA_DST_INC_NONE);
    MAP_UARTDMAEnable(UARTA1_BASE,UART_DMA_TX); //使能dma发送
    //MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMATX); //使能dma发送完成中断


    UDMASetupTransfer(UDMA_CH10_UARTA1_RX,
    				  UDMA_MODE_BASIC,
                      8,
                      UDMA_SIZE_8,
                      UDMA_ARB_2,
                      (void *)(UARTA1_BASE+UART_O_DR),
                      UDMA_SRC_INC_NONE,
                      (void *)raRxBuff,
                      UDMA_DST_INC_8);
    MAP_UARTDMAEnable(UARTA1_BASE,UART_DMA_RX);	//使能dma接收
    MAP_UARTIntEnable(UARTA1_BASE,UART_INT_DMARX);	//使能dma接收完成中断
    isintRasRx=1;

}

void uart1dmarx()
{
	UDMASetupTransfer(UDMA_CH10_UARTA1_RX,
	    				  UDMA_MODE_BASIC,
	                      8,
	                      UDMA_SIZE_8,
	                      UDMA_ARB_2,
	                      (void *)(UARTA1_BASE+UART_O_DR),
	                      UDMA_SRC_INC_NONE,
	                      (void *)raRxBuff,
	                      UDMA_DST_INC_8);
	MAP_UARTDMAEnable(UARTA1_BASE,UART_DMA_RX);	//使能dma接收
	MAP_UARTIntEnable(UARTA1_BASE,UART_INT_DMARX);	//使能dma接收完成中断
}

void uart1dmatx(void *u1txbuff)
{
    UDMASetupTransfer(UDMA_CH11_UARTA1_TX,
                         UDMA_MODE_BASIC,
                         8,
                         UDMA_SIZE_8,
                         UDMA_ARB_2,
                         (void *)u1txbuff,
                         UDMA_SRC_INC_8,
                         (void *)(UARTA1_BASE+UART_O_DR),
                         UDMA_DST_INC_NONE);
    MAP_UARTDMAEnable(UARTA1_BASE,UART_DMA_TX); //使能dma发送
}

static void UART1IntHandler()
{
	MAP_UARTDMADisable(UARTA1_BASE,UART_DMA_RX);
	//UARTCharPut(CONSOLE,0x01);
	mark1=1;
	MAP_UARTIntClear(UARTA1_BASE,UART_INT_DMARX);
}
