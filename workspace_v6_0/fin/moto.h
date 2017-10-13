/*
 * moto.h
 *
 *  Created on: 2015Äê7ÔÂ18ÈÕ
 *      Author: rsb
 */

#ifndef MOTO_H_
#define MOTO_H_


#define Moto_PwmMax 9999

void MotorPwmFlash(int16_t MOTO1_PWM,int16_t MOTO2_PWM,int16_t MOTO3_PWM,int16_t MOTO4_PWM);
void MotorInit(void);


#endif /* MOTO_H_ */
