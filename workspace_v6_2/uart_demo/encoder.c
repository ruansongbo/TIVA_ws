/*
 * encoder.c
 *
 *  Created on: 2016年8月22日
 *      Author: Z
 */

#include "encoder.h"
#include "Timer_if.h"
#include "filter.h"
extern int left_sign,right_sign;
int left_encoder_buf[5] = {0,0,0,0,0};
int right_encoder_buf[5] = {0,0,0,0,0};
extern int left_encoder;
extern int right_encoder;

void InitEncoder(void)		//初始化encorder
{
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);	//使能时钟
    MAP_PinTypeTimer(PIN_05, PIN_MODE_12);  //GT_CC05 TimerB
    MAP_PinTypeTimer(PIN_15, PIN_MODE_5);  //GT_CC04 TimerA
    //管脚下拉设置
    MAP_PinConfigSet(PIN_05,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin05号管脚  GT_CC05 TimerB
    MAP_PinConfigSet(PIN_15,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin15号管脚 GT_CC04 TimerA
    MAP_TimerConfigure(TIMERA2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT|TIMER_CFG_B_CAP_COUNT));
    MAP_TimerControlEvent(TIMERA2_BASE,TIMER_BOTH,TIMER_EVENT_BOTH_EDGES);
    MAP_TimerLoadSet(TIMERA2_BASE,TIMER_BOTH,0xffff);
    MAP_TimerEnable(TIMERA2_BASE,TIMER_BOTH);
}
void ReadEncoder(void)
{
	left_encoder = (65535 - TimerValueGet(TIMERA2_BASE, TIMER_A));
	if(left_encoder > 500)
	{
		left_encoder = 500;
	}
	if(left_encoder < 0)
	{
		left_encoder = 0;
	}
	left_encoder *=left_sign;
	left_encoder = left_encoder/10;
	right_encoder = (65535 - TimerValueGet(TIMERA2_BASE, TIMER_B));
	if(right_encoder > 50)
	{
		right_encoder = 50;
	}
	if(right_encoder < 0)
	{
		right_encoder = 0;
	}
	right_encoder *= right_sign;
	MAP_TimerLoadSet(TIMERA2_BASE,TIMER_BOTH,0xffff);
}

/*void InitEncoder(void)		//初始化encorder
{

    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PinTypeTimer(PIN_03, PIN_MODE_12);
    MAP_PinConfigSet(PIN_03,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);
    MAP_TimerConfigure(TIMERA1_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT));
    MAP_TimerControlEvent(TIMERA1_BASE,TIMER_B,TIMER_EVENT_BOTH_EDGES);
    MAP_TimerLoadSet(TIMERA1_BASE,TIMER_B,0xffff);
    MAP_TimerEnable(TIMERA1_BASE,TIMER_B);
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);	//使能时钟
    MAP_PinTypeTimer(PIN_05, PIN_MODE_12);  //GT_CC05 TimerB
    //管脚下拉设置
    MAP_PinConfigSet(PIN_05,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);	//pin05号管脚  GT_CC05 TimerB
    MAP_TimerConfigure(TIMERA2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT));
    MAP_TimerControlEvent(TIMERA2_BASE,TIMER_B,TIMER_EVENT_BOTH_EDGES);
    MAP_TimerLoadSet(TIMERA2_BASE,TIMER_B,0xffff);
    MAP_TimerEnable(TIMERA2_BASE,TIMER_B);
}

int ReadEncoder(unsigned char motor)
{
	int encoder;
	if(motor)
	{
		encoder = (65535 - TimerValueGet(TIMERA1_BASE, TIMER_B));
		MAP_TimerLoadSet(TIMERA1_BASE,TIMER_B,0xffff);
		if(encoder > 50)
		{
			encoder = 50;
		}
		if(encoder < 0)
		{
			encoder = 0;
		}
		encoder *=left_sign;
		encoder = filter(left_encoder_buf,encoder);
	}
	else
	{
		encoder = (65535 - TimerValueGet(TIMERA2_BASE, TIMER_B));
		MAP_TimerLoadSet(TIMERA2_BASE,TIMER_B,0xffff);
		if(encoder > 50)
		{
			encoder = 50;
		}
		if(encoder < 0)
		{
			encoder = 0;
		}
		encoder *= right_sign;
		encoder = filter(right_encoder_buf,encoder);
	}
	return encoder;
}*/
