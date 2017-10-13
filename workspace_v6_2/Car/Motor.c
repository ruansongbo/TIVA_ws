/*******************************************************************************
* 电机驱动程序，内部包括PWM函数调用和管脚初始化函数。
* 函数调用说明：
void motorinit() 电机初始化函数,使用其它函数前须在主函数的调用
void move(unsigned char motor,unsigned char speed,unsigned char direction) 电机运动函数
//motor: 0代表电机A，1代表电机B
//peed： 0关闭电机，255最大速度，0~255
//direction： 0正转，1反转
void stop(unsigned char motor) 电机停止函数
//motor：0停止电机A，1停止电机B，2全部停止所有IO口清零
void rotating(unsigned char angle) 小车原地旋转函数
//angle： 0:原地不做操作， 1：原地旋转90度， 2：原地旋转180度， 3：原地旋转270度， 4：原地旋转360度
* 注意事项：
使用了定时器3的TimerA和TimerB，也即是由PIN_01和PIN_02产生的两路TimerPWM6,7。
频率10.12KHZ，80M晶振未进行预分频。
* 管脚定义：
PIN01(GPIO_10)——PWM1——电机A
PIN02(GPIO_11)——PWM2——电机B
PIN64(GPIO_09)——Stby——电机AB
PIN60(GPIO_05)——Ain1——电机AB
PIN61(GPIO_06)——Ain2——电机AB
PIN62(GPIO_07)——Bin1——电机AB
PIN63(GPIO_08)——Bin2——电机AB


Edit By 钟祥永
********************************************************************************/
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "hw_gpio.h"
#include "pin.h"
#include "timer.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "prcm.h"
#include "Motor.h"
#include "utils.h"
#include "encoder.h"

struct _pid{
	int SetSpeedA;
	int SetSpeedB;
	int ActualSpeedA;
	int ActualSpeedB;
	unsigned char SetpwmA;
	unsigned char SetpwmB;
	int errA;
	int errB;
	int err_nextA;
	int err_nextB;
	int err_lastA;
	int err_lastB;
	double KpA,KiA,KdA;
	double KpB,KiB,KdB;
}pid;

//电机初始化函数
void motorinit()
{
	PinMuxConfig();
	GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,0);
	GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,0);
	GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,0);
	GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,0);
	GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1,0);
	InitPWMModules();
}
//motor: 0代表电机A（右轮），1代表电机B（左轮）
//peed： 0关闭电机，255最大速度，0~255
//direction： 0正转，1反转
void move(unsigned char motor,int speed,unsigned char direction)
{
	//置位Stby
	GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1,GPIO_PIN_1);
	if (0==motor)
	{
	//驱动电机A
		if (0==direction)
		{
		//正转
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,0);
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,GPIO_PIN_6);
		}
		else if (1==direction)
		{
		//反转
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,GPIO_PIN_5);
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,0);
		}
		//调节速度
		if(speed<=255&&speed>=0)
		{
			UpdateDutyCycle(TIMERA3_BASE, TIMER_A, speed);
		}
	}
	else if (1==motor)
	{

	//驱动电机B
		if (0==direction)
		{
		//正转
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,0);
			GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,GPIO_PIN_0);
		}
		else if (1==direction)
		{
		//反转
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,GPIO_PIN_7);
			GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,0);
		}
		//调节速度
		if(speed<=255&&speed>=1)
		{
			UpdateDutyCycle(TIMERA3_BASE, TIMER_B, speed);
		}
	}

}
//motor：0停止电机A，1停止电机B，2全部停止所有IO口清零
void stop(unsigned char motor)
{
 switch (motor)
 {
 case 0:
	 GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,0);
	 GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,0);
	 break;
 case 1:
	 GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,0);
	 GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,0);
	 break;
 case 2:
	 GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1,0);
	 break;
 }
}
//原地旋转
//angle： 0:原地不做操作， 1：原地旋转90度， 2：原地旋转180度， 3：原地旋转270度， 4：原地旋转360度
void delayms(int x)
{
	int i,j;
	for (i=1;i<x;i++)
		for(j=1;j<1200;j++);
}
void rotating(unsigned char angle)
{
	switch (angle)
	{
	case 0:
		break;
	case 1:
		move(0,75,1);
		move(1,75,0);
		UtilsDelay(800000);
		delayms(2500);
		stop(2);
		break;
	case 2:
		move(0,50,1);
		move(1,50,0);
	//	delayms(8380);
		delayms(9680);
		stop(2);
		UtilsDelay(800000);
		break;
	case 3:
		break;
	case 4:
		break;
	}
}

void PID_init()
{
	pid.SetSpeedA=0;
	pid.ActualSpeedA=0;
	pid.SetpwmA=0;
	pid.errA=0;
	pid.err_nextA=0;
	pid.err_lastA=0;
	pid.KpA=0.2;
	pid.KiA=0.015;
	pid.KdA=0.2;


	pid.SetSpeedB=0;
	pid.ActualSpeedB=0;
	pid.SetpwmB=0;
	pid.errB=0;
	pid.err_nextB=0;
	pid.err_lastB=0;
	pid.KpB=0.2;
	pid.KiB=0.015;
	pid.KdB=0.2;

}
void PID_realize(unsigned char directiona,int setspeeda,unsigned char directionb,int setspeedb)
{
	double increment=0.0;
	pid.SetSpeedA=setspeeda;
	pid.SetSpeedB=setspeedb;
	pid.ActualSpeedA=0xff*(65535-g_ulSamples[0]);
	pid.ActualSpeedB=0xff*(65535-g_ulSamples[1]);
	pid.errA=pid.SetSpeedA-pid.ActualSpeedA;
	pid.errB=pid.SetSpeedB-pid.ActualSpeedB;
    increment=pid.KpA*(pid.errA-pid.err_nextA)+pid.KiA*pid.errA+pid.KdA*(pid.errA-2*pid.err_nextA+pid.err_lastA);
    pid.SetpwmA+=(unsigned char)increment;
    increment=pid.KpB*(pid.errB-pid.err_nextB)+pid.KiB*pid.errB+pid.KdB*(pid.errB-2*pid.err_nextB+pid.err_lastB);
    pid.SetpwmB+=(unsigned char)increment;
	move(0,pid.SetpwmA,directiona);
	move(1,pid.SetpwmB,directionb);
	pid.err_lastA=pid.err_nextA;
	pid.err_nextA=pid.errA;
	pid.err_lastB=pid.err_nextB;
	pid.err_nextB=pid.errB;
}

void
PinMuxConfig(void)
{
    //
    // Enable Peripheral Clocks
    //
    //MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);

    //Ain1电机1控制正反转 PIN60，GPIO_05
    MAP_PinTypeGPIO(PIN_60, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_5, GPIO_DIR_MODE_OUT);
    //Ain2电机1控制正反转 PIN61,GPIO_06
    MAP_PinTypeGPIO(PIN_61, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_6, GPIO_DIR_MODE_OUT);
    //Bin1电机2控制正反转 PIN62,GPIO_07
    MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_7, GPIO_DIR_MODE_OUT);
    //Bin2电机2控制正反转 PIN64,GPIO_08
    MAP_PinTypeGPIO(PIN_63, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, GPIO_PIN_0, GPIO_DIR_MODE_OUT);


    //管脚64，也即是GPIO09，为GPIO模式，STD（无上拉下拉无开漏）模式，输出模式
    //Stby 清零控制电机停止，置一方可控制正反转
    MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, GPIO_PIN_1, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_01 for TIMERPWM6 GT_PWM06
    //
    MAP_PinTypeTimer(PIN_01, PIN_MODE_3);

    //
    // Configure PIN_02 for TIMERPWM7 GT_PWM07
    //
    MAP_PinTypeTimer(PIN_02, PIN_MODE_3);
}


//! This function
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,(ucLevel*DUTYCYCLE_GRANULARITY));
}

//****************************************************************************
//
//! Setup the timer in PWM mode
//!
//! \param ulBase is the base address of the timer to be configured
//! \param ulTimer is the timer to be setup (TIMER_A or  TIMER_B)
//! \param ulConfig is the timer configuration setting
//! \param ucInvert is to select the inversion of the output
//!
//! This function
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void SetupTimerPWMMode(unsigned long ulBase, unsigned long ulTimer,
                       unsigned long ulConfig, unsigned char ucInvert)
{
    //
    // Set GPT - Configured Timer in PWM mode.
    //
    MAP_TimerConfigure(ulBase,ulConfig);
    MAP_TimerPrescaleSet(ulBase,ulTimer,0);

    //
    // Inverting the timer output if required
    //
    MAP_TimerControlLevel(ulBase,ulTimer,ucInvert);

    //
    // Load value set to ~0.5 ms time period
    //
    MAP_TimerLoadSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

    //
    // Match value set so as to output level 0
    //
    MAP_TimerMatchSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

}

//****************************************************************************
//
//! Sets up the identified timers as PWM to drive the peripherals
//!
//! \param none
//!
//! This function sets up the folowing
//!    1. TIMERA2 (TIMER B) as RED of RGB light
//!    2. TIMERA3 (TIMER B) as YELLOW of RGB light
//!    3. TIMERA3 (TIMER A) as GREEN of RGB light
//!
//! \return None.
//
//****************************************************************************
void InitPWMModules()
{
    //
    // Initialization of timers to generate PWM output
    //
    //MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);

    //
    // TIMERA2 (TIMER B) as RED of RGB light. GPIO 9 --> PWM_5
    //
    //SetupTimerPWMMode(TIMERA2_BASE, TIMER_B,
    //        (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER B) as YELLOW of RGB light. GPIO 10 --> PWM_6
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_A,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER A) as GREEN of RGB light. GPIO 11 --> PWM_7
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_B,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);

    //MAP_TimerEnable(TIMERA2_BASE,TIMER_B);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_A);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_B);
}

//****************************************************************************
//
//! Disables the timer PWMs
//!
//! \param none
//!
//! This function disables the timers used
//!
//! \return None.
//
//****************************************************************************
void DeInitPWMModules()
{
    //
    // Disable the peripherals
    //
    //MAP_TimerDisable(TIMERA2_BASE, TIMER_B);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_A);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_B);
    //MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);
}

