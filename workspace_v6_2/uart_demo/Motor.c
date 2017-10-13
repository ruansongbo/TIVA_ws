/*******************************************************************************
* ������������ڲ�����PWM�������ú͹ܽų�ʼ��������
* ��������˵����
void motorinit() �����ʼ������,ʹ����������ǰ�����������ĵ���
void move(unsigned char motor,unsigned char speed,unsigned char direction) ����˶�����
//motor: 0��������1��������
//peed�� 0�رյ����255����ٶȣ�0~255
//direction�� 0��ת��1��ת
void stop(unsigned char motor) ���ֹͣ����
//motor��0ֹͣ���A��1ֹͣ���B��2ȫ��ֹͣ����IO������
void rotating(unsigned char angle) С��ԭ����ת����
//angle�� 0:ԭ�ز��������� 1��ԭ����ת90�ȣ� 2��ԭ����ת180�ȣ� 3��ԭ����ת270�ȣ� 4��ԭ����ת360��
* ע�����
ʹ���˶�ʱ��3��TimerA��TimerB��Ҳ������PIN_01��PIN_02��������·TimerPWM6,7��
Ƶ��10.12KHZ��80M����δ����Ԥ��Ƶ��
* �ܽŶ��壺
PIN01(GPIO_10)����PWM1�������A
PIN02(GPIO_11)����PWM2�������B
PIN64(GPIO_09)����Stby�������AB
PIN60(GPIO_05)����Ain1�������AB
PIN61(GPIO_06)����Ain2�������AB
PIN62(GPIO_07)����Bin1�������AB
PIN63(GPIO_08)����Bin2�������AB


Edit By ������
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



//�����ʼ������
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
//motor: 0������A�����֣���1������B�����֣�
//peed�� 0�رյ����255����ٶȣ�0~255
//direction�� 0��ת��1��ת
void move(unsigned char motor,int speed,unsigned char direction)
{
	//��λStby
	GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1,GPIO_PIN_1);
	if (0==motor)
	{
	//�������A
		if (0==direction)
		{
		//��ת
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,0);
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,GPIO_PIN_6);
		}
		else if (1==direction)
		{
		//��ת
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5,GPIO_PIN_5);
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_6,0);
		}
		//�����ٶ�
		if(speed<=MAX_MOTOR && speed>=0)
		{
			UpdateDutyCycle(TIMERA3_BASE, TIMER_A, speed);
		}
	}
	else if (1==motor)
	{

	//�������B
		if (0==direction)
		{
		//��ת
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,0);
			GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,GPIO_PIN_0);
		}
		else if (1==direction)
		{
		//��ת
			GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7,GPIO_PIN_7);
			GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_0,0);
		}
		//�����ٶ�
		if(speed<=MAX_MOTOR && speed>=1)
		{
			UpdateDutyCycle(TIMERA3_BASE, TIMER_B, speed);
		}
	}

}
//motor��0ֹͣ���A��1ֹͣ���B��2ȫ��ֹͣ����IO������
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

void delayms(int x)
{
	int i,j;
	for (i=1;i<x;i++)
		for(j=1;j<1200;j++);
}
//ԭ����ת
//angle�� 0:ԭ�ز��������� 1��ԭ����ת90�ȣ� 2��ԭ����ת180�ȣ� 3��ԭ����ת270�ȣ� 4��ԭ����ת360��

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

    //Ain1���1��������ת PIN60��GPIO_05
    MAP_PinTypeGPIO(PIN_60, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_5, GPIO_DIR_MODE_OUT);
    //Ain2���1��������ת PIN61,GPIO_06
    MAP_PinTypeGPIO(PIN_61, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_6, GPIO_DIR_MODE_OUT);
    //Bin1���2��������ת PIN62,GPIO_07
    MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_7, GPIO_DIR_MODE_OUT);
    //Bin2���2��������ת PIN64,GPIO_08
    MAP_PinTypeGPIO(PIN_63, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, GPIO_PIN_0, GPIO_DIR_MODE_OUT);


    //�ܽ�64��Ҳ����GPIO09��ΪGPIOģʽ��STD�������������޿�©��ģʽ�����ģʽ
    //Stby ������Ƶ��ֹͣ����һ���ɿ�������ת
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
                     unsigned int ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,ucLevel);
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

