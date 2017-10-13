/*
 * encoder.h
 *
 *  Created on: 2016��8��22��
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

void InitEncoder(void);		//��ʼ��encorder
void ReadEncoder(void);

#endif /* ENCODER_H_ */
