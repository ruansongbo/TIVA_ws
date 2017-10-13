/*
 * encoder.c
 *
 *  Created on: 2016��8��22��
 *      Author: Z
 */

#include "encoder.h"
#include "Timer_if.h"
#include "uart.h"
#include "Motor.h"


void encoderinit()		//��ʼ��encorder
{
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);	//ʹ��ʱ��
    MAP_PinTypeTimer(PIN_05, PIN_MODE_12);  //GT_CC05 TimerB
    MAP_PinTypeTimer(PIN_04, PIN_MODE_12);  //GT_CC04 TimerA
    //�ܽ���������
    MAP_PinConfigSet(PIN_05,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin05�Źܽ�  GT_CC05 TimerB
    MAP_PinConfigSet(PIN_04,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin15�Źܽ� GT_CC04 TimerA
    MAP_TimerConfigure(TIMERA2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT|TIMER_CFG_B_CAP_COUNT));
    MAP_TimerControlEvent(TIMERA2_BASE,TIMER_BOTH,TIMER_EVENT_POS_EDGE);
    MAP_TimerLoadSet(TIMERA2_BASE,TIMER_BOTH,0xffff);
/*
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);	//ʹ��ʱ��
    MAP_PinTypeTimer(PIN_06, PIN_MODE_13);  //GT_CC06 TimerA
    MAP_PinConfigSet(PIN_07,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin6�Źܽ� GT_CC06 TimerA
    MAP_TimerIntRegister(TIMERA3_BASE,TIMER_A,TimermatchIntHandler);
    MAP_TimerConfigure(TIMERA3_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT));
    MAP_TimerControlEvent(TIMERA3_BASE,TIMER_A,TIMER_EVENT_POS_EDGE);
    MAP_TimerLoadSet(TIMERA3_BASE,TIMER_A,0xffff);
    MAP_TimerMatchSet(TIMERA3_BASE,TIMER_A,0x00);
    */
}


//����ʱ����coder����ֵ���Դ��������ٶȴ�С��mSecValueΪ��ʱʱ�䣬��λ���롣�ڶ�������Ϊ��ʱ�жϺ���
void limitencoder(unsigned long mSecValue)
{
	Timer_IF_Init(PRCM_TIMERA1, TIMERA1_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA1_BASE, TIMER_A, TimerlimitIntHandler);
	Timer_IF_Start(TIMERA1_BASE, TIMER_A, mSecValue);
	MAP_TimerLoadSet(TIMERA2_BASE,TIMER_BOTH,0xffff);
	MAP_TimerMatchSet(TIMERA2_BASE,TIMER_BOTH,0x00);
	MAP_TimerEnable(TIMERA2_BASE,TIMER_BOTH);
}


void dislimitencoder()
{
	Timer_IF_Stop(TIMERA1_BASE,TIMER_A);
}

//ʹ��match���ܣ��趨һ������ֵdis�������������ָ��ֵʱ�����жϡ�
void matchencoder(unsigned long dis)
{
	   MAP_TimerLoadSet(TIMERA3_BASE,TIMER_A,0xffff);
	   MAP_TimerMatchSet(TIMERA3_BASE,TIMER_A,dis);
	   MAP_TimerIntEnable(TIMERA3_BASE,TIMER_CAPA_MATCH);
	   MAP_TimerEnable(TIMERA3_BASE,TIMER_A);
}

void dismatchencoder()
{
	MAP_TimerIntDisable(TIMERA3_BASE,TIMER_CAPA_MATCH);
	MAP_TimerDisable(TIMERA3_BASE,TIMER_A);
}

void TimerlimitIntHandler(void)
{
	Timer_IF_InterruptClear(TIMERA1_BASE);
	g_ulSamples[0] = MAP_TimerValueGet(TIMERA2_BASE,TIMER_A);  //GT_CCP04
	g_ulSamples[1] = MAP_TimerValueGet(TIMERA2_BASE,TIMER_B);
	MAP_TimerLoadSet(TIMERA2_BASE,TIMER_BOTH,0xffff);
	//UARTCharPut(UARTA0_BASE,0xab);
}

void TimermatchIntHandler(void)
{
    MAP_TimerIntClear(TIMERA3_BASE,TIMER_CAPA_MATCH);
    MAP_TimerLoadSet(TIMERA3_BASE,TIMER_A,0xffff);
}
