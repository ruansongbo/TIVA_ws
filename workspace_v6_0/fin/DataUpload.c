/*
 * DataUpload.c
 *
 *  Created on: 2015年7月10日
 *      Author: rsb
 */
#include <stdbool.h>
#include <stdint.h>
#include "extern.h"
#include "DataUpload.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "control.h"





#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

/* ----------------------- RC Channel Definition---------------------------- */
#define RC_CH_VALUE_MIN ((uint16_t)364 )
#define RC_CH_VALUE_OFFSET ((uint16_t)1024)
#define RC_CH_VALUE_MAX ((uint16_t)1684)
/* ----------------------- RC Switch Definition----------------------------- */
#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)


#define UART_RXBUF_SIZE         18

static uint8_t g_ui8RxBuf[UART_RXBUF_SIZE];

uint8_t buff_index;

uint8_t buffer_length[6] = {18,23,21,25,29,29};

uint8_t statusbuff[18]={1},senserbuff[23]={2},motopwmbuff[21]={3},rcdatabuff[25]={4},tempbuff[29],temp1buff[29];
uint8_t *buff_indic[6] = {statusbuff,senserbuff,motopwmbuff,rcdatabuff,temp1buff,tempbuff};


uint8_t ui8ControlTable[1024];
void InitUARTuDMA(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(ui8ControlTable);
}

void
UART1IntHandler(void)
{
    uint32_t ui32Mode;
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART1_BASE, 1);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    ui32Mode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT);   //主结构体
    if(ui32Mode == UDMA_MODE_STOP)
    {
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
                                    UDMA_MODE_BASIC,
                                   (void *)(UART1_BASE + UART_O_DR),
                                   g_ui8RxBuf, sizeof(g_ui8RxBuf));
        Rc_D.PITCH = (g_ui8RxBuf[0]| (g_ui8RxBuf[1] << 8)) & 0x07ff; //!< Channel 0
        Rc_D.ROLL = ((g_ui8RxBuf[1] >> 3) | (g_ui8RxBuf[2] << 5)) & 0x07ff; //!< Channel 1
        Rc_D.YAW = ((g_ui8RxBuf[2] >> 6) | (g_ui8RxBuf[3] << 2) | (g_ui8RxBuf[4] << 10)) & 0x07ff;   //!< Channel 2
        Rc_D.THROTTLE = ((g_ui8RxBuf[4] >> 1) | (g_ui8RxBuf[5] << 7)) & 0x07ff; //!< Channel 3
        Rc_D.S1 = ((g_ui8RxBuf[5] >> 4)& 0x000C) >> 2; //!< Switch left
        Rc_D.S2 = ((g_ui8RxBuf[5] >> 4)& 0x0003); //!< Switch right
        Data_Send_RCData();
        ROM_uDMAChannelEnable(UDMA_CHANNEL_UART1RX);
    }


}

void
UART0IntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART0_BASE, 1);
    ROM_UARTIntClear(UART0_BASE, ui32Status);
    if(!ROM_uDMAChannelIsEnabled(UDMA_CHANNEL_UART0TX))
    {
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                                    UDMA_MODE_BASIC, buff_indic[buff_index],
                                    (void *)(UART0_BASE + UART_O_DR),
                                    buffer_length[buff_index]);
        ROM_uDMAChannelEnable(UDMA_CHANNEL_UART0TX);
        buff_index++;
        if(buff_index == 1)
        {
            buff_index = 0;
            ROM_IntDisable(INT_UART0);
        }
    }
}
void
InitUART0Transfer(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);/**/
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                                UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                UART_CONFIG_PAR_NONE);
    ROM_UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);
    ROM_UARTEnable(UART0_BASE);
    ROM_UARTDMAEnable(UART0_BASE, UART_DMA_TX);
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0TX,
                                    UDMA_ATTR_ALTSELECT |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    ROM_uDMAChannelAttributeEnable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_USEBURST);

    ROM_uDMAChannelControlSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE |
                              UDMA_ARB_4);
    ROM_uDMAChannelEnable(UDMA_CHANNEL_UART0TX);
}
void
InitUART1Transfer(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 100000,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_EVEN);
    ROM_UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);
    ROM_UARTEnable(UART1_BASE);
    ROM_UARTDMAEnable(UART1_BASE, UART_DMA_RX | UART_DMA_TX);
    ROM_IntEnable(INT_UART1);

    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_UART1RX,
                                    UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 |
                              UDMA_ARB_1024);
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
                                UDMA_MODE_BASIC,
                               (void *)(UART1_BASE + UART_O_DR),
                               g_ui8RxBuf, sizeof(g_ui8RxBuf));
    ROM_uDMAChannelEnable(UDMA_CHANNEL_UART1RX);
}


//1：发送基本信息（姿态、锁定状态）
void Data_Send_Status(void)
{
    uint8_t _cnt=0;
    statusbuff[_cnt++]=0xAA;
    statusbuff[_cnt++]=0xAA;
    statusbuff[_cnt++]=0x01;
    statusbuff[_cnt++]=0;
    int16_t _temp;
    _temp = (int16_t)(Euler.roll*100);
    statusbuff[_cnt++]=BYTE1(_temp);
    statusbuff[_cnt++]=BYTE0(_temp);
    _temp = (int16_t)(Euler.pitch*100);
    statusbuff[_cnt++]=BYTE1(_temp);
    statusbuff[_cnt++]=BYTE0(_temp);
    _temp = (int16_t)(Euler.yaw*100);
    statusbuff[_cnt++]=BYTE1(_temp);
    statusbuff[_cnt++]=BYTE0(_temp);
    int32_t _temp2 = (int32_t)g_Alt_Hight;
    statusbuff[_cnt++]=BYTE3(_temp2);
    statusbuff[_cnt++]=BYTE2(_temp2);
    statusbuff[_cnt++]=BYTE1(_temp2);
    statusbuff[_cnt++]=BYTE0(_temp2);

    statusbuff[_cnt++]=0;

    statusbuff[_cnt++]=0xA1;

    statusbuff[3] = _cnt-4;

    uint8_t sum = 0;
    uint8_t i;
    for(i=0;i<_cnt;i++)
    {
        sum += statusbuff[i];
    }
    statusbuff[_cnt++]=sum;
}

/*//2：发送传感器数据
void Data_Send_Senser(void)
{
    uint8_t _cnt=0;
    int i;
    senserbuff[_cnt++]=0xAA;
    senserbuff[_cnt++]=0xAA;
    senserbuff[_cnt++]=0x02;
    senserbuff[_cnt++]=0;
    int16_t temp = (int16_t)(EXP_Alt);
    senserbuff[_cnt++]=BYTE1(temp);  //高8位
    senserbuff[_cnt++]=BYTE0(temp);  //低8位
    temp = (int16_t)(g_Alt_Hight);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(hight_error);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(hight_increment);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(hight_error-hight_errorold);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(Thro);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;

    senserbuff[3] = _cnt-4;

    uint8_t sum = 0;

    for(i=0;i<_cnt;i++)
        sum += senserbuff[i];
    senserbuff[_cnt++] = sum;
}*/

//2：发送传感器数据
void Data_Send_Senser(void)
{
    uint8_t _cnt=0;
    int i;
    senserbuff[_cnt++]=0xAA;
    senserbuff[_cnt++]=0xAA;
    senserbuff[_cnt++]=0x02;
    senserbuff[_cnt++]=0;
    int16_t temp = (int16_t)(Accel.X*100);
    senserbuff[_cnt++]=BYTE1(temp);  //高8位
    senserbuff[_cnt++]=BYTE0(temp);  //低8位
    temp = (int16_t)(Accel.Y*100);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(Accel.Z*100);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(Gyro.X*100);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(Gyro.Y*100);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(Gyro.Z*100);
    senserbuff[_cnt++]=BYTE1(temp);
    senserbuff[_cnt++]=BYTE0(temp);
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;
    senserbuff[_cnt++]=0;

    senserbuff[3] = _cnt-4;

    uint8_t sum = 0;

    for(i=0;i<_cnt;i++)
        sum += senserbuff[i];
    senserbuff[_cnt++] = sum;
}

//3：发送遥控数据
void Data_Send_RCData(void)
{
    uint8_t _cnt=0;
    rcdatabuff[_cnt++]=0xAA;
    rcdatabuff[_cnt++]=0xAA;
    rcdatabuff[_cnt++]=0x03;
    rcdatabuff[_cnt++]=0;
    rcdatabuff[_cnt++]=BYTE1(Rc_D.THROTTLE);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.THROTTLE);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.YAW);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.YAW);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.ROLL);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.ROLL);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.PITCH);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.PITCH);
    int16_t temp = (int16_t)(EXP_ANGLE.X*10);
    rcdatabuff[_cnt++]=BYTE1(temp);
    rcdatabuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(EXP_ANGLE.Y*10);
    rcdatabuff[_cnt++]=BYTE1(temp);
    rcdatabuff[_cnt++]=BYTE0(temp);
    temp = (int16_t)(EXP_ANGLE.Z*10);
    rcdatabuff[_cnt++]=BYTE1(temp);
    rcdatabuff[_cnt++]=BYTE0(temp);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.AUX4);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.AUX4);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.AUX5);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.AUX5);
    rcdatabuff[_cnt++]=BYTE1(Rc_D.AUX6);
    rcdatabuff[_cnt++]=BYTE0(Rc_D.AUX6);

    rcdatabuff[3] = _cnt-4;

    uint8_t sum = 0;
    uint8_t i;
    for(i=0;i<_cnt;i++)
        sum += rcdatabuff[i];

    rcdatabuff[_cnt++]=sum;
}



//4：发送PWM值
void Data_Send_MotoPWM(void)
{
    uint8_t _cnt=0;
    motopwmbuff[_cnt++]=0xAA;
    motopwmbuff[_cnt++]=0xAA;
    motopwmbuff[_cnt++]=0x06;
    motopwmbuff[_cnt++]=0;
    int16_t temp = MOTO1_PWM/10;
    motopwmbuff[_cnt++]=BYTE1(temp);
    motopwmbuff[_cnt++]=BYTE0(temp);
    temp = MOTO2_PWM/10;
    motopwmbuff[_cnt++]=BYTE1(temp);
    motopwmbuff[_cnt++]=BYTE0(temp);
    temp = MOTO3_PWM/10;
    motopwmbuff[_cnt++]=BYTE1(temp);
    motopwmbuff[_cnt++]=BYTE0(temp);
    temp = MOTO4_PWM/10;
    motopwmbuff[_cnt++]=BYTE1(temp);
    motopwmbuff[_cnt++]=BYTE0(temp);
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;
    motopwmbuff[_cnt++]=0;

    motopwmbuff[3] = _cnt-4;

    uint8_t sum = 0;
    uint8_t i;
    for(i=0;i<_cnt;i++)
        sum += motopwmbuff[i];

    motopwmbuff[_cnt++]=sum;
}

void temp1send(PID_Typedef *pid,float temp)
{
    uint8_t _cnt=0;
    tempbuff[_cnt++]=0xAA;
    tempbuff[_cnt++]=0xAA;
    tempbuff[_cnt++]=0xF1;
    tempbuff[_cnt++]=0;

    tempbuff[_cnt++]=BYTE3(pid->desired);
    tempbuff[_cnt++]=BYTE2(pid->desired);
    tempbuff[_cnt++]=BYTE1(pid->desired);
    tempbuff[_cnt++]=BYTE0(pid->desired);

    tempbuff[_cnt++]=BYTE3(pid->outP);
    tempbuff[_cnt++]=BYTE2(pid->outP);
    tempbuff[_cnt++]=BYTE1(pid->outP);
    tempbuff[_cnt++]=BYTE0(pid->outP);

    tempbuff[_cnt++]=BYTE3(pid->outD);
    tempbuff[_cnt++]=BYTE2(pid->outD);
    tempbuff[_cnt++]=BYTE1(pid->outD);
    tempbuff[_cnt++]=BYTE0(pid->outD);

    tempbuff[_cnt++]=BYTE3(pid->outI);
    tempbuff[_cnt++]=BYTE2(pid->outI);
    tempbuff[_cnt++]=BYTE1(pid->outI);
    tempbuff[_cnt++]=BYTE0(pid->outI);

    tempbuff[_cnt++]=BYTE3(pid->integ);
    tempbuff[_cnt++]=BYTE2(pid->integ);
    tempbuff[_cnt++]=BYTE1(pid->integ);
    tempbuff[_cnt++]=BYTE0(pid->integ);

    tempbuff[_cnt++]=BYTE3(temp);
    tempbuff[_cnt++]=BYTE2(temp);
    tempbuff[_cnt++]=BYTE1(temp);
    tempbuff[_cnt++]=BYTE0(temp);

    tempbuff[3] = _cnt-4;

    uint8_t sum = 0;
    uint8_t i;
    for(i=0;i<_cnt;i++)
        sum += tempbuff[i];

    tempbuff[_cnt++]=sum;
}


void temp2send(PID_Typedef *pid,float temp)
{
    uint8_t _cnt=0;
    temp1buff[_cnt++]=0xAA;
    temp1buff[_cnt++]=0xAA;
    temp1buff[_cnt++]=0xF2;
    temp1buff[_cnt++]=0;

    temp1buff[_cnt++]=BYTE3(pid->desired);
    temp1buff[_cnt++]=BYTE2(pid->desired);
    temp1buff[_cnt++]=BYTE1(pid->desired);
    temp1buff[_cnt++]=BYTE0(pid->desired);

    temp1buff[_cnt++]=BYTE3(pid->outP);
    temp1buff[_cnt++]=BYTE2(pid->outP);
    temp1buff[_cnt++]=BYTE1(pid->outP);
    temp1buff[_cnt++]=BYTE0(pid->outP);

    temp1buff[_cnt++]=BYTE3(pid->outD);
    temp1buff[_cnt++]=BYTE2(pid->outD);
    temp1buff[_cnt++]=BYTE1(pid->outD);
    temp1buff[_cnt++]=BYTE0(pid->outD);

    temp1buff[_cnt++]=BYTE3(pid->outI);
    temp1buff[_cnt++]=BYTE2(pid->outI);
    temp1buff[_cnt++]=BYTE1(pid->outI);
    temp1buff[_cnt++]=BYTE0(pid->outI);

    temp1buff[_cnt++]=BYTE3(pid->out);
    temp1buff[_cnt++]=BYTE2(pid->out);
    temp1buff[_cnt++]=BYTE1(pid->integ);
    temp1buff[_cnt++]=BYTE0(pid->integ);

    temp1buff[_cnt++]=BYTE3(temp);
    temp1buff[_cnt++]=BYTE2(temp);
    temp1buff[_cnt++]=BYTE1(temp);
    temp1buff[_cnt++]=BYTE0(temp);

    temp1buff[3] = _cnt-4;

    uint8_t sum = 0;
    uint8_t i;
    for(i=0;i<_cnt;i++)
        sum += temp1buff[i];

    temp1buff[_cnt++]=sum;
}



