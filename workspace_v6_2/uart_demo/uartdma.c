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
    //
    // Configure PIN_55 for UART0 UART0_TX
    //
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);
    MAP_UARTConfigSetExpClk(CONSOLE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH),
	                            UART_BAUD_RATE,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                            UART_CONFIG_PAR_NONE));

    UARTFIFOLevelSet(UARTA0_BASE,UART_FIFO_TX4_8,UART_FIFO_RX4_8);

}

void uart0dmatx(void *u0txbuff)
{
	UDMASetupTransfer(UDMA_CH9_UARTA0_TX,
	                         UDMA_MODE_BASIC,
	                         23,
	                         UDMA_SIZE_8,
	                         UDMA_ARB_4,
	                         (void *)u0txbuff,
	                         UDMA_SRC_INC_8,
	                         (void *)(UARTA0_BASE+UART_O_DR),
	                         UDMA_DST_INC_NONE);
	    MAP_UARTDMAEnable(UARTA0_BASE,UART_DMA_TX); //使能dma发送
}

