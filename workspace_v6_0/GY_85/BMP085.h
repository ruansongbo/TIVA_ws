/*
 * BMP085.h
 *
 *  Created on: 2015Äê4ÔÂ24ÈÕ
 *      Author: ideapad
 */

#ifndef BMP085_H_
#define BMP085_H_
#include <stdint.h>

void Init_BMP085(void);
void read_BMP085(void);
long bmp085ReadTemp(void);
long bmp085ReadPressure(void);
#endif /* BMP085_H_ */
