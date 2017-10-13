/*
 * DataUpload.h
 *
 *  Created on: 2015Äê7ÔÂ10ÈÕ
 *      Author: rsb
 */


#ifndef DATAUPLOAD_H_
#define DATAUPLOAD_H_
#include "control.h"



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
        //
        // Hang on runtime error.
        //
    }
}
#endif

void InitUARTuDMA(void);
void InitUART1Transfer(void);
void InitUART0Transfer(void);


//void Data_Receive_Anl(u8 *data_buf,u8 num);

void Data_Send_Status(void);
void Data_Send_Senser(void);
void Data_Send_RCData(void);
void Data_Send_MotoPWM(void);
void temp1send(PID_Typedef *pid,float temp);
void temp2send(PID_Typedef *pid,float temp);

#endif /* DATAUPLOAD_H_ */
