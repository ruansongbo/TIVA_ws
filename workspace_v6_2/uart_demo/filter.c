/*
 * filter.c
 *
 *  Created on: 2016Äê10ÔÂ22ÈÕ
 *      Author: bb
 */



#include "filter.h"

int filter(int data[5], int datain)
{
	int dataout = 0;
	float sum;
	data[0] = data[1];
	data[1] = data[2];
	data[2] = data[3];
	data[3] = data[4];
	data[4] = datain;
	sum = data[0] + data[1] + data[2] + data[3] + data[4];
	dataout = (int)(sum/5);
	sum = 0;
	return dataout;
}
