/*
 * control.h
 *
 *  Created on: 2015年7月24日
 *      Author: rsb
 */

#ifndef CONTROL_H_
#define CONTROL_H_
#define IMU_UPDATE_DT 0.01
#include "extern.h"

typedef struct{
    float desired;     //期望                //< set point
    float error;       //积分                //< error
    float prevError;                         //< previous error
    float integ;                             //< integral
    float deriv;       //微分                //< derivative
    float kp;          //比例系数            //< proportional gain
    float ki;          //积分系数            //< integral gain
    float kd;          //微分系数            //< derivative gain
    float outP;         //< proportional output (debugging)
    float outI;         //< integral output (debugging)
    float outD;         //< derivative output (debugging)
    float iLimit;      //< integral limit
    float out;
}PID_Typedef;



extern PID_Typedef	alt_PID;
extern PID_Typedef alt_acc_PID;

float act_ACC;

extern float Thro,Roll,Pitch,Yaw;

#define PID_ROLL_RATE_KP  1.2
#define PID_ROLL_RATE_KI  0//.1
#define PID_ROLL_RATE_KD  0.08
#define PID_ROLL_RATE_INTEGRATION_LIMIT    100.0

#define PID_ROLL_KP  10.0
#define PID_ROLL_KI  0.1
#define PID_ROLL_KD  0.0
#define PID_ROLL_INTEGRATION_LIMIT    20.0

#define PID_PITCH_RATE_KP  1.3
#define PID_PITCH_RATE_KI  0.1
#define PID_PITCH_RATE_KD  0.08
#define PID_PITCH_RATE_INTEGRATION_LIMIT   100.0

#define PID_PITCH_KP  8//4.2
#define PID_PITCH_KI  0//.5
#define PID_PITCH_KD  0//.09
#define PID_PITCH_INTEGRATION_LIMIT   20.0

#define PID_YAW_RATE_KP  5.0
#define PID_YAW_RATE_KI  0.0001
#define PID_YAW_RATE_KD  0.0
#define PID_YAW_RATE_INTEGRATION_LIMIT     500.0

#define PID_YAW_KP  0.8
#define PID_YAW_KI  0.005
#define PID_YAW_KD  0.0
#define PID_YAW_INTEGRATION_LIMIT     360.0

#define PID_ATL_KP  0.57
#define PID_ATL_KI  1.65
#define PID_ATL_KD  0.48//.004
#define PID_ATL_INTEGRATION_LIMIT     380.0


#define basethro 6400    //33
//#define basethro 6390      //22

#define DEFAULT_PID_INTEGRATION_LIMIT  5000.0

void PID_Calculate(axis_f *EXP_ANGLE);
void InitPID(void);
void Atl_PID(unsigned int set_hight);

#endif /* CONTROL_H_ */
