#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "utils/cpu_usage.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#define s1sta GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1)
#define s2sta GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2)
#define s3sta GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3)

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
    }
}
#endif

void Initmotors(void)
{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);

	    GPIOPinConfigure(GPIO_PB6_M0PWM0);
	    GPIOPinConfigure(GPIO_PB4_M0PWM2);

	    SysCtlPWMClockSet(SYSCTL_PWMDIV_16);

	    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN|
	                                PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN|
	                        		PWM_GEN_MODE_NO_SYNC);

	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 10000);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 10000);

	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,0);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,0);

	    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT|PWM_OUT_2_BIT, true);

	    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

void systeminit(void)
{
	ROM_FPUEnable();
	ROM_FPULazyStackingEnable();
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
}

void
InitGPIO(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
}

uint16_t speed=2;
void main(void)
{
	systeminit();
	InitGPIO();
	Initmotors();
	while(1)
	{
		if(s1sta)
		{
			speed++;
			if(speed==3000)
			{
				speed=2999;
			}
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,speed);
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,speed);
		}
		if(s2sta)
		{
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,speed);
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,speed);
		    speed--;
		    if(speed==1)
		    {
		    	speed=2;
		    }
		}
		if(s3sta)
		{
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,speed);
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,speed);
		}
		SysCtlDelay(40000);
	}
}
