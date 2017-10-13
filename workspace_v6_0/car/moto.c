/*
 * moto.c
 *
 *  Created on: 2015年7月18日
 *      Author: rsb
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "moto.h"
/***********************************************
函数名：MotorPwmFlash(int16_t MOTO1_PWM,int16_t MOTO2_PWM,int16_t MOTO3_PWM,int16_t MOTO4_PWM)
功能：更新四路PWM值
输入参数：MOTO1_PWM,MOTO2_PWM,MOTO3_PWM,MOTO4_PWM
输出:无
描述：四路PWM由定时器2输出，输入范围0-999
备注：
***********************************************/
void MotorPwmFlash(int16_t MOTO1_PWM,int16_t MOTO2_PWM)
{
     if(MOTO1_PWM>=Moto_PwmMax) MOTO1_PWM = Moto_PwmMax;
     if(MOTO2_PWM>=Moto_PwmMax) MOTO2_PWM = Moto_PwmMax;
     if(MOTO1_PWM<=0)   MOTO1_PWM = 0;
     if(MOTO2_PWM<=0)   MOTO2_PWM = 0;//限定输入不能小于0，大于999
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,MOTO1_PWM);
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,MOTO2_PWM);
}

void MotorInit(void)
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

        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,5000);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,5000);

        PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT|PWM_OUT_2_BIT, true);

        PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

