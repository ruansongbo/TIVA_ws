
//*********************************************************************
//* spi_nrf24l01.c
//*********************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "nrf24l01.h"

extern uint8_t RX_BUF[TX_PLOAD_WIDTH];

extern uint8_t TX_BUF[TX_PLOAD_WIDTH];

void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}
void main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    InitConsole();
    SPI_TM4C123_init();
    UARTprintf("nRF24L01霞編。。。。。。。。");
    NRF24L01_Check();
    RX_Mode();
    //TX_Mode(TX_BUF);
    while(1)
    {
        //NRF24L01_Receive();
        //NRF24L01_Send();
        nRF24L01_RxPacket(RX_BUF);
        //nRF24L01_TxPacket(TX_BUF);
        SysCtlDelay(31250000);

    }
}
