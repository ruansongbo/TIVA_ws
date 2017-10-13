/*
 * control.c
 *
 *  Created on: 2015年7月24日
 *      Author: rsb
 */

#include "control.h"
#include "extern.h"
#include "moto.h"
#include "DataUpload.h"
#include "math.h"

#define RtA 		57.324841

float rollRateDesired;
float pitchRateDesired;
float yawRateDesired;
float altAccDesired;

//----PID结构体实例化----
PID_Typedef pitch_angle_PID;	//角度环的PID
PID_Typedef pitch_rate_PID;		//角速率环的PID

PID_Typedef roll_angle_PID;
PID_Typedef roll_rate_PID;

PID_Typedef yaw_angle_PID;
PID_Typedef yaw_rate_PID;

PID_Typedef	alt_PID;
PID_Typedef alt_acc_PID;

float Thro=0,Roll=0,Pitch=0,Yaw=0;

axis_f DIF_ACC;		//实际去期望相差的加速度

void pidInit(PID_Typedef * pid, const float desired, const float kp,
             const float ki, const float kd)
{
  pid->error = 0;
  pid->prevError = 0;
  pid->integ = 0;
  pid->deriv = 0;
  pid->desired = desired;
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;

  pid->iLimit = DEFAULT_PID_INTEGRATION_LIMIT;
}

void pidSetIntegralLimit(PID_Typedef * PID, const float limit)
{
	PID->iLimit = limit;
}

void InitPID(void)
{
	pidInit(&roll_rate_PID, 0, PID_ROLL_RATE_KP, PID_ROLL_RATE_KI, PID_ROLL_RATE_KD);
	pidInit(&pitch_rate_PID, 0, PID_PITCH_RATE_KP, PID_PITCH_RATE_KI, PID_PITCH_RATE_KD);
	pidInit(&yaw_rate_PID, 0, PID_YAW_RATE_KP, PID_YAW_RATE_KI, PID_YAW_RATE_KD);
	pidSetIntegralLimit(&roll_rate_PID, PID_ROLL_RATE_INTEGRATION_LIMIT);
	pidSetIntegralLimit(&pitch_rate_PID, PID_PITCH_RATE_INTEGRATION_LIMIT);
	pidSetIntegralLimit(&yaw_rate_PID, PID_YAW_RATE_INTEGRATION_LIMIT);

	pidInit(&roll_angle_PID, 0, PID_ROLL_KP, PID_ROLL_KI, PID_ROLL_KD);
	pidInit(&pitch_angle_PID, 0, PID_PITCH_KP, PID_PITCH_KI, PID_PITCH_KD);
	pidInit(&yaw_angle_PID, 0, PID_YAW_KP, PID_YAW_KI, PID_YAW_KD);
	pidInit(&alt_PID, 0, PID_ATL_KP, PID_ATL_KI, PID_ATL_KD);
	pidSetIntegralLimit(&roll_angle_PID, PID_ROLL_INTEGRATION_LIMIT);
	pidSetIntegralLimit(&pitch_angle_PID, PID_PITCH_INTEGRATION_LIMIT);
	pidSetIntegralLimit(&yaw_angle_PID, PID_YAW_INTEGRATION_LIMIT);
	pidSetIntegralLimit(&alt_PID, PID_ATL_INTEGRATION_LIMIT);
}

float pidUpdate(PID_Typedef* pid, const float measured, const bool updateError,float dt)
{
	float output;

	if (updateError)
	{
		pid->error = pid->desired - measured;
	}

	pid->integ += pid->error * dt;
	if (pid->integ > pid->iLimit)
	{
		pid->integ = pid->iLimit;
	}
	else if (pid->integ < -pid->iLimit)
	{
		pid->integ = -pid->iLimit;
	}

	pid->deriv = (pid->error - pid->prevError) / dt;

	pid->outP = pid->kp * pid->error;
	pid->outI = pid->ki * pid->integ;
	pid->outD = pid->kd * pid->deriv;

	output = (pid->kp * pid->error) +
			 (pid->ki * pid->integ) +
			 (pid->kd * pid->deriv);
	pid->out = output;

	pid->prevError = pid->error;

	return output;
}

//函数名：PID_Calculate()

void PID_Calculate(axis_f *EXP_ANGLE)
{
    pitch_angle_PID.desired = EXP_ANGLE->Y;
    roll_angle_PID.desired = EXP_ANGLE->X;
    yaw_angle_PID.desired = EXP_ANGLE->Z;

    rollRateDesired = pidUpdate(&roll_angle_PID, Euler.roll, 1, IMU_UPDATE_DT);
    roll_rate_PID.desired = rollRateDesired;
    Roll = pidUpdate(&roll_rate_PID, Gyro.X, 1, IMU_UPDATE_DT);

    pitchRateDesired = pidUpdate(&pitch_angle_PID, Euler.pitch, 1, IMU_UPDATE_DT);
    pitch_rate_PID.desired = pitchRateDesired;
    Pitch = pidUpdate(&pitch_rate_PID, Gyro.Y, 1, IMU_UPDATE_DT);

    float yawError;
    yawError = yaw_angle_PID.desired - Euler.yaw;
    if (yawError > 180.0)
        yawError -= 360.0;
    else if (yawError < -180.0)
        yawError += 360.0;

    yaw_angle_PID.error = yawError;
    yawRateDesired = pidUpdate(&yaw_angle_PID, Euler.yaw, 0, IMU_UPDATE_DT);

    yaw_rate_PID.desired = yawRateDesired;
    Yaw = pidUpdate(&yaw_rate_PID, Gyro.Z, 1, IMU_UPDATE_DT);
   //将输出值融合到四个电机
    MOTO1_PWM = (int16_t)(Thro - Roll - Pitch + Yaw ) + basethro;    //M1
    MOTO2_PWM = (int16_t)(Thro + Roll - Pitch - Yaw ) + basethro;    //M2
    MOTO3_PWM = (int16_t)(Thro + Roll + Pitch + Yaw ) + basethro;    //M3
    MOTO4_PWM = (int16_t)(Thro - Roll + Pitch - Yaw ) + basethro;    //M4
    if(FLY_ENABLE)
    {
    	MotorPwmFlash(MOTO1_PWM,MOTO2_PWM,MOTO3_PWM,MOTO4_PWM);
    }
}


unsigned int g_HightOld=0;
float g_Alt_Hight=0,g_Alt_HightOld=0;
void Atl_PID(unsigned int set_hight)
{
	static float Alt_Hight[3];
	if(Hight>3000||Hight<28)
		Hight=g_HightOld;
	else
		g_HightOld=Hight;
	Alt_Hight[2]=Alt_Hight[1];
	Alt_Hight[1]=Alt_Hight[0];
	Alt_Hight[0]=(float)Hight*((float)cos(Euler.roll/RtA)*(float)cos(Euler.pitch/RtA));
	g_Alt_HightOld=g_Alt_Hight;
	g_Alt_Hight=(Alt_Hight[0]+Alt_Hight[1]+Alt_Hight[2])/3;

	if(g_Alt_Hight-g_Alt_HightOld>50)						   //防测量错误
	    g_Alt_Hight=g_Alt_HightOld+50;
	else if(g_Alt_HightOld-g_Alt_Hight>50)
		g_Alt_Hight=g_Alt_HightOld-50;
	alt_PID.desired = set_hight;
	Thro = pidUpdate(&alt_PID, g_Alt_Hight, 1, 0.05);
}

