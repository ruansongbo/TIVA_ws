#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "OLED.h"
#include "common.h"

/*pin_map***************
 *          AmoMcu                 TIVA(SPI0)
 *           D0         -         (SCLK)  PA4
 *           D1         -         (MOSI)  PA5
 *           RST        -           ��λ             PA6
 *           DC         -        ��������ѡ�����ݣ�H ���L�� PA7
 *           CS         -        Ƭѡ���ѽӵأ�
 */

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

void InitGPIO(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, 0xF0);
}
/*----------------------------------------------------------------------------*
 * �ڲ�����ԭ��                                                               *
 *----------------------------------------------------------------------------*/
VOID MAIN_Init(VOID); //��ʼ������
VOID MAIN_OLED_Display(VOID); //OLED��ʾ����

/*----------------------------------------------------------------------------*
 * �ⲿ����ԭ��                                                               *
 *----------------------------------------------------------------------------*/

VOID main(void)
{
    InitGPIO();
    InitConsole();
    MAIN_Init();
    MAIN_OLED_Display();
    while(1)
    {}
}

VOID MAIN_Init(VOID)
{
    LED_Init();

    return;
}

VOID MAIN_OLED_Display(VOID)
{
   USHORT16 i;

    LED_PXx16MixStr(20, 0, (UCHAR8*)"imodule�Ƽ�");
    LED_P8x16Str(0, 2, (UCHAR8*)"http://imodule168.taobao.com");
    SysCtlDelay(1560000);

    LED_Fill(0x00);
    LED_P14x16Str(30, 1, (UCHAR8*)"��Ļ����");

    for (i = 0; i <= 100; i++)
    {
        SysCtlDelay(312500);
        LED_PrintChar(45, 5, (CHAR8)i);
    }

    LED_Fill(0xFF); //����ȫ��
    SysCtlDelay(1560000);

    LED_Fill(0x00); //����
    SysCtlDelay(1560000);

    LED_P8x16Str(10, 2, (UCHAR8*)"Test Finished!");

}
