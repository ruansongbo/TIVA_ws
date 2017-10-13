/*
 * control.c
 *
 *  Created on: 2016年10月18日
 *      Author: bb
 */
#include "hw_memmap.h"
#include "hw_types.h"
#include "control.h"
#include "timer_if.h"
#include "prcm.h"
#include "timer.h"
#include "encoder.h"
#include "motor.h"
#include "uart_if.h"
#include "uartdma.h"
#include "filter.h"

#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))

int left_sign = 1;
int right_sign = 1;
int left_motor_buf[5],right_motor_buf[5];
int left_encoder,right_encoder;


//*****************************************************************************
//
//! Init timer for control loop.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
void
InitTimer(void)
{
    //
    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerIntControlHandler);

    //
    // Turn on the timers feeding values in mSec
    //
    Timer_IF_Start(TIMERA0_BASE, TIMER_A, 10);

}

//*****************************************************************************
//
//! The interrupt handler for control.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
void
TimerIntControlHandler(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	int left_motor,right_motor;
	int desired = 10;
	ReadEncoder();
	right_motor = PIUpdate(&right_vel_PID, right_encoder, desired);
	left_motor = PIUpdate(&left_vel_PID, left_encoder, desired);
	Set_Pwm(left_motor, right_motor);
	DataSendParam(&left_vel_PID, &right_vel_PID, left_encoder, right_encoder);
}

/**************************************************************************
Set direction of motor
**************************************************************************/
void Set_Pwm(int left_motor, int right_motor)
{
	if(left_motor >= 0)
	{
		move(0,left_motor,1);
		left_sign = 1;
	}
	else
	{
		move(0,myabs(left_motor),0);
		left_sign = -1;
	}
	if(right_motor >= 0)
	{
		move(1,right_motor,1);
		right_sign = 1;
	}
	else
	{
		move(1,myabs(right_motor),0);
		right_sign = -1;
	}
}



/**************************************************************************
myabs
**************************************************************************/
int myabs(int a)
{
	  int temp;
		if(a<0)  temp=-a;
	  else temp=a;
	  return temp;
}

void pidInit(PID_Typedef * pid, const int kp,  const int ki,  const int kd,  const int limit)
{
  pid->error = 0;
  pid->prevError = 0;
  pid->integ = 0;
  pid->deriv = 0;
  pid->kp = kp;
  pid->kd = kd;
  pid->ki = ki;
  pid->iLimit = limit;
}

void InitPIparam(void)
{
	pidInit(&left_vel_PID, PID_LEFT_VEL_KP, PID_LEFT_VEL_KI, PID_LEFT_VEL_KD, PID_LEFT_LIMIT);
	pidInit(&right_vel_PID, PID_RIGHT_VEL_KP, PID_RIGHT_VEL_KI, PID_RIGHT_VEL_KD, PID_RIGHT_LIMIT);
}

int PIUpdate(PID_Typedef* pid, const int measured, const int desired)
{
	int output;
	pid->error = desired - measured;

	pid->integ += pid->error;
	if (pid->integ > pid->iLimit)
	{
		pid->integ = pid->iLimit;
	}
	else if (pid->integ < -pid->iLimit)
	{
		pid->integ = -pid->iLimit;
	}

	pid->deriv = (pid->error - pid->prevError);

	pid->outP = pid->kp * pid->error;
	pid->outI = pid->ki * pid->integ;
	pid->outD = pid->kd * pid->deriv;

	output = pid->outP + pid->outD + pid->outI;

    if(output < (-1*MAX_MOTOR)) output = (-1*MAX_MOTOR);
	if(output > MAX_MOTOR)  output = MAX_MOTOR;
	if(desired > 0)
	{
		if(output < 0)
		{
			output = 0;
		}
	}
	if(desired < 0)
	{
		if(output > 0)
		{
			output = 0;
		}
	}
	pid->out = output;

	pid->prevError = pid->error;

	return pid->out;
}

void DataSendParam(PID_Typedef * left, PID_Typedef * right, int left_encoder, int right_encoder)
{
	unsigned char _cnt=0;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0x02;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=BYTE1(left->outP);  //高8位
	data_to_send[_cnt++]=BYTE0(left->outP);  //低8位
	data_to_send[_cnt++]=BYTE1(left->outD);
	data_to_send[_cnt++]=BYTE0(left->outD);
	data_to_send[_cnt++]=BYTE1(left->out);
	data_to_send[_cnt++]=BYTE0(left->out);
	data_to_send[_cnt++]=BYTE1(left_encoder);
	data_to_send[_cnt++]=BYTE0(left_encoder);
	data_to_send[_cnt++]=BYTE1(right->outP);  //高8位
	data_to_send[_cnt++]=BYTE0(right->outP);  //低8位
	data_to_send[_cnt++]=BYTE1(right->outD);
	data_to_send[_cnt++]=BYTE0(right->outD);
	data_to_send[_cnt++]=BYTE1(right->out);
	data_to_send[_cnt++]=BYTE0(right->out);
	data_to_send[_cnt++]=BYTE1(right_encoder);
	data_to_send[_cnt++]=BYTE0(right_encoder);
	data_to_send[_cnt++]=BYTE1(left->outI);
	data_to_send[_cnt++]=BYTE0(left->outI);

	data_to_send[3] = _cnt-4;

	unsigned char sum = 0;
	unsigned char i;
	for(i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;
	uart0dmatx(data_to_send);
}
