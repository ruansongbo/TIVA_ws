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
 *           RST        -           复位             PA6
 *           DC         -        命令数据选择（数据：H 命令：L） PA7
 *           CS         -        片选（已接地）
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
 * 内部函数原型                                                               *
 *----------------------------------------------------------------------------*/
VOID MAIN_Init(VOID); //初始化函数
VOID MAIN_OLED_Display(VOID); //OLED显示函数

/*----------------------------------------------------------------------------*
 * 外部函数原型                                                               *
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

    LED_PXx16MixStr(20, 0, (UCHAR8*)"imodule科技");
    LED_P8x16Str(0, 2, (UCHAR8*)"http://imodule168.taobao.com");
    SysCtlDelay(1560000);

    LED_Fill(0x00);
    LED_P14x16Str(30, 1, (UCHAR8*)"屏幕测试");

    for (i = 0; i <= 100; i++)
    {
        SysCtlDelay(312500);
        LED_PrintChar(45, 5, (CHAR8)i);
    }

    LED_Fill(0xFF); //点亮全屏
    SysCtlDelay(1560000);

    LED_Fill(0x00); //黑屏
    SysCtlDelay(1560000);

    LED_P8x16Str(10, 2, (UCHAR8*)"Test Finished!");

}
