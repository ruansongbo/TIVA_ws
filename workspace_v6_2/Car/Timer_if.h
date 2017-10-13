/*
 * Delay.h
 *
 *  Created on: 2016Äê8ÔÂ19ÈÕ
 *      Author: Z
 */

#ifndef DELAY_H_
#define DELAY_H_

#define SYS_CLK				    80000000
#define MILLISECONDS_TO_TICKS(ms)   ((SYS_CLK/1000) * (ms))
#define PERIODIC_TEST_LOOPS     5

extern void Timer_IF_Init( unsigned long ePeripheralc, unsigned long ulBase,
    unsigned long ulConfig, unsigned long ulTimer, unsigned long ulValue);
extern void Timer_IF_IntSetup(unsigned long ulBase, unsigned long ulTimer,
                   void (*TimerBaseIntHandler)(void));
extern void Timer_IF_InterruptClear(unsigned long ulBase);
extern void Timer_IF_Start(unsigned long ulBase, unsigned long ulTimer,
                unsigned long ulValue);
extern void Timer_IF_Stop(unsigned long ulBase, unsigned long ulTimer);
extern void Timer_IF_ReLoad(unsigned long ulBase, unsigned long ulTimer,
                unsigned long ulValue);
extern unsigned int Timer_IF_GetCount(unsigned long ulBase, unsigned long ulTimer);
void Timer_IF_DeInit(unsigned long ulBase,unsigned long ulTimer);


void initdelay(unsigned long mSecValue,void (*TimerBaseIntHandler)(void));
void deinitdelay();
void TimerBaseIntHandler(void);

#endif /* DELAY_H_ */
