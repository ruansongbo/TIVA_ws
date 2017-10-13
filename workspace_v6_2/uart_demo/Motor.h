/*
 * Motor.h
 *
 *  Created on: 2016年4月3日
 *      Author: Z
 */

#ifndef MOTOR_H_
#define MOTOR_H_

extern void motorinit();
//电机初始化函数
extern void move(unsigned char motor,int speed,unsigned char direction);
//motor: 0代表电机A，1代表电机B
//peed： 0关闭电机，255最大速度，0~255
//direction： 0正转，1反转
extern void stop(unsigned char motor);
//motor：0停止电机A，1停止电机B，2全部停止所有IO口清零
extern void rotating(unsigned char angle);
//原地旋转
//angle： 0:原地不做操作， 1：原地旋转90度， 2：原地旋转180度， 3：原地旋转270度， 4：原地旋转360度

extern void PinMuxConfig(void);
// The PWM works based on the following settings:
//     Timer reload interval -> determines the time period of one cycle
//     Timer match value -> determines the duty cycle
//                          range [0, timer reload interval]
// The computation of the timer reload interval and dutycycle granularity
// is as described below:
// Timer tick frequency = 80 Mhz = 80000000 cycles/sec
// For a time period of 0.5 ms,
//      Timer reload interval = 80000000/2000 = 40000 cycles
// To support steps of duty cycle update from [0, 255]
//      duty cycle granularity = ceil(40000/255) = 157
// Based on duty cycle granularity,
//      New Timer reload interval = 255*157 = 40035
//      New time period = 0.5004375 ms
//      Timer match value = (update[0, 255] * duty cycle granularity)
//
extern void delayms(int x);

//#define TIMER_INTERVAL_RELOAD   40035 /* =(255*157) */
//#define DUTYCYCLE_GRANULARITY   157
#define TIMER_INTERVAL_RELOAD   7905 /* =(255*31) */
#define MAX_MOTOR   7500
extern void UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned int ucLevel);
extern void SetupTimerPWMMode(unsigned long ulBase, unsigned long ulTimer,
                       unsigned long ulConfig, unsigned char ucInvert);
extern void InitPWMModules();
extern void DeInitPWMModules();



#endif /* MOTOR_H_ */
