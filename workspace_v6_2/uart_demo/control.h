/*
 * control.h
 *
 *  Created on: 2016��10��18��
 *      Author: bb
 */

#ifndef CONTROL_H_
#define CONTROL_H_5
#define PID_LEFT_VEL_KP  7
#define PID_LEFT_VEL_KI  17
#define PID_LEFT_VEL_KD  5
#define PID_LEFT_LIMIT  5000

#define PID_RIGHT_VEL_KP  8
#define PID_RIGHT_VEL_KI  7
#define PID_RIGHT_VEL_KD  5
#define PID_RIGHT_LIMIT  5000


typedef struct{
	int error;       //����                //< error
	int prevError;                         //< previous error
    int integ;                             //< integral
    int deriv;       //΢��                //< derivative
	int kp;          //����ϵ��            //< proportional gain
    int ki;          //����ϵ��            //< integral gain
	int kd;          //����ϵ��            //< integral gain
    int iLimit;      //< integral limit
	int outP;         //< proportional output (debugging)
    int outI;         //< integral output (debugging)
	int outD;         //< integral output (debugging)
	int out;
}PID_Typedef;


//----PID�ṹ��ʵ����----
PID_Typedef left_vel_PID;	//�ǶȻ���PID
PID_Typedef right_vel_PID;		//�����ʻ���PID


unsigned char data_to_send[23];

void InitTimer(void);
void DataSendParam(PID_Typedef * left, PID_Typedef * right, int left_encoder, int right_encoder);
void TimerIntControlHandler(void);

void Set_Pwm(int left_motor, int right_motor);
int myabs(int a);
void pidInit(PID_Typedef * pid, const int kp,  const int ki,  const int kd,  const int limit);
void InitPIparam(void);
int PIUpdate(PID_Typedef* pid, const int measured, const int desired);

#endif /* CONTROL_H_ */
