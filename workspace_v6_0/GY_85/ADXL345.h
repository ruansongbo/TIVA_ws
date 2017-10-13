/*
 * ADXL345.h
 *
 *  Created on: 2015年4月24日
 *      Author: ideapad
 */

#ifndef ADXL345_H_
#define ADXL345_H_
#include "extern.h"

void Init_ADXL345(void);             //初始化ADXL345

void ADXL_getdate(axis_f *data);

#endif /* ADXL345_H_ */
