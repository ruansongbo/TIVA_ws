/*
 * time.h
 *
 *  Created on: 2015Äê7ÔÂ4ÈÕ
 *      Author: rsb
 */

#ifndef TIME_H_
#define TIME_H_


extern void SysTickIntHandler(void);
extern uint32_t micros(void);
extern uint32_t millis(void);
extern void DelayMs(uint16_t nms);


#endif /* TIME_H_ */
