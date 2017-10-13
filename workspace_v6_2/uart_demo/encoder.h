/*
 * encoder.h
 *
 *  Created on: 2016年8月22日
 *      Author: Z
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"


unsigned long g_ulSamples[2];

void InitEncoder(void);		//初始化encorder
void ReadEncoder(void);

#endif /* ENCODER_H_ */
