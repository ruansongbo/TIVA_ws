/*
 * HMC5883.h
 *
 *  Created on: 2015年4月24日
 *      Author: ideapad
 */

#ifndef HMC5883_H_
#define HMC5883_H_
#include <stdint.h>
#include "extern.h"
#define hmc_SlaveAddress 0x1E   //定义器件在IIC总线中的从地址

void HMC5883_Init(void);
int HMC5883_getdata(axis_d *magnet);
void HMC5883_self_test(void);
void HMCgetdate(void);


#endif /* HMC5883_H_ */
