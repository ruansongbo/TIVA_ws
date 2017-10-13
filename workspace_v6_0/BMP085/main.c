#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "math.h"

#define     BMP085_Addr   0x77
#define     BMP085_SlaveAddress   0x77

#define     OSS 0

//***BMP085使用
short ac1;
short ac2;
short ac3;
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1;
short b2;
short mb;
short mc;
short md;

long  temperature;
long  pressure;
//********

void Delayms(void)
{
    SysCtlDelay(378);
}
void
InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

void systeminit(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                           SYSCTL_XTAL_16MHZ);
}

void InitI2C1(void)
{
    //This function is for eewiki and is to be updated to handle any port

    //enable I2C module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    //reset I2C module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

    //enable GPIO peripheral that contains I2C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    //clear I2C FIFOs
    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;
}

uint8_t readI2C1(uint16_t device_address, uint16_t device_register)
{
    //specify that we want to communicate to device address with an intended write to bus
    I2CMasterSlaveAddrSet(I2C1_BASE, device_address, false);

    //the register to be read
    I2CMasterDataPut(I2C1_BASE, device_register);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    //wait for MCU to complete send transaction
    while(I2CMasterBusy(I2C1_BASE));

    //read from the specified slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, device_address, true);

    //send control byte and read from the register from the MCU
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait while checking for MCU to complete the transaction
    while(I2CMasterBusy(I2C1_BASE));

    //Get the data from the MCU register and return to caller
    return( I2CMasterDataGet(I2C1_BASE));
}


int writeI2C1(unsigned char slave_addr,
              unsigned char reg_addr,
              unsigned char data)
{
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);
    I2CMasterDataPut(I2C1_BASE, reg_addr);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    I2CMasterDataPut(I2C1_BASE, data);
    // 主模块开始发送数据
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_STOP);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    return 0;
}

int8_t IICreadBytes(unsigned char slave_addr,
                 unsigned char reg_addr,
                 unsigned char length,
                 unsigned char *data)
{
    if(0 == length)
        return -1;
    if(1 == length)
    {
        I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);
        I2CMasterDataPut(I2C1_BASE, reg_addr);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        data[0] = I2CMasterDataGet(I2C1_BASE);
        return 1;
    }
    else
    {
        unsigned char i;
        I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);
        I2CMasterDataPut(I2C1_BASE, reg_addr);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        data[0] = I2CMasterDataGet(I2C1_BASE);
        for(i=1; i<length-1; i++)
        {

            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
            while(I2CMasterBusy(I2C1_BASE))
            {
            }
            data[i] = I2CMasterDataGet(I2C1_BASE);
        }
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        data[i] = I2CMasterDataGet(I2C1_BASE);
    return i;
    }
}

short Multiple_read(uint8_t ST_Address)
{
    uint8_t msb, lsb;
    short _data;
    msb = readI2C1(BMP085_SlaveAddress, ST_Address);
    lsb = readI2C1(BMP085_SlaveAddress, ST_Address+1);
    _data = msb << 8;
    _data |= lsb;
    return _data;
}
/********************************************************************
long bmp085ReadTemp(void)
{
    writeI2C1(BMP085_SlaveAddress,0xF4,0x2E);
    Delayms();
    return (long) Multiple_read(0xF6);
}

long bmp085ReadPressure(void)
{
    long pressure = 0;
    writeI2C1(BMP085_SlaveAddress,0xF4,0x34);
    pressure = Multiple_read(0xF6);
    pressure &= 0x0000FFFF;
    return pressure;
}*/
//********************************************************************
long bmp085ReadTemp(void)
{   short  temp_ut;
    writeI2C1(BMP085_Addr,0xF4,0x2E);
    Delayms(); // max time is 4.5ms
    temp_ut = readI2C1(BMP085_Addr,0xF6);
    temp_ut = (temp_ut<<8)| readI2C1(BMP085_Addr,0xF7);
    return (long) temp_ut ;
}
//*************************************************************

long bmp085ReadPressure(void)
{
    long pressure = 0;
    writeI2C1(BMP085_Addr,0xF4,0x34);
    SysCtlDelay(37800);
    pressure = readI2C1(BMP085_Addr,0xF6);
    pressure = (pressure<<8)| readI2C1(BMP085_Addr,0xF7);
    pressure &= 0x0000FFFF;
    return pressure;
}
//*****************************************
void read_BMP085(void)
{

    long ut;
    long up;
    long x1, x2, b5, b6, x3, b3, p;
    unsigned long b4, b7;

    ut = bmp085ReadTemp();     // 读取温度
    ut = bmp085ReadTemp();     // 读取温度
    up = bmp085ReadPressure();  // 读取压强
    up = bmp085ReadPressure();  // 读取压强
    x1 = ((long)ut - ac6) * ac5 >> 15;
    x2 = ((long) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    temperature = (b5 + 8) >> 4;

     //****************

    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (((long)ac1 * 4 + x3) + 2)/4;
    x1 = ac3 * b6 >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
    b7 = ((unsigned long) up - b3) * (50000 >> OSS);
    if( b7 < 0x80000000)
         p = (b7 * 2) / b4 ;
           else
            p = (b7 / b4) * 2;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
     pressure = p + ((x1 + x2 + 3791) >> 4);
}
void  Init_BMP085(void)
{
   ac1 = readI2C1(BMP085_Addr,0xAA);
   ac1 = (ac1<<8)|readI2C1(BMP085_Addr,0xAB);

   ac2 = readI2C1(BMP085_Addr,0xAC);
   ac2 = (ac2<<8)| readI2C1(BMP085_Addr,0xAD);

   ac3 = readI2C1(BMP085_Addr,0xAE);
   ac3 = (ac3<<8)| readI2C1(BMP085_Addr,0xAF);

   ac4 = readI2C1(BMP085_Addr,0xB0);
   ac4 = (ac4<<8)| readI2C1(BMP085_Addr,0xB1);

   ac5 = readI2C1(BMP085_Addr,0xB2);
   ac5 = (ac5<<8)| readI2C1(BMP085_Addr,0xB3);

   ac6 = readI2C1(BMP085_Addr,0xB4);
   ac6 = (ac6<<8)| readI2C1(BMP085_Addr,0xB5);

   b1 = readI2C1(BMP085_Addr,0xB6);
   b1 = (b1<<8)| readI2C1(BMP085_Addr,0xB7);

   b2 = readI2C1(BMP085_Addr,0xB8);
   b2 = (b2<<8)| readI2C1(BMP085_Addr,0xB9);

   mb = readI2C1(BMP085_Addr,0xBA);
   mb = (mb<<8)| readI2C1(BMP085_Addr,0xBB);

   mc = readI2C1(BMP085_Addr,0xBC);
   mc = (mc<<8)| readI2C1(BMP085_Addr,0xBD);

   md = readI2C1(BMP085_Addr,0xBE);
   md = (md<<8)| readI2C1(BMP085_Addr,0xBF);

}
long x1,x2,x3,b3,b5,b6,b7,press_reg,pressure,temp_reg,temp;
unsigned long b4;
uint8_t oss=0;

uint8_t BUF[3];
/*
 * main.c
 */
void main(void)
{
    systeminit();
    InitI2C1();
    InitConsole();
    //Init_BMP085();
    while(1)
    {
        volatile uint32_t ui32Loop;
        //UARTprintf("\r\n Read start \r\n");
        Init_BMP085();
        //UARTprintf("ac1=%d\n,ac2=%d\n,ac3=%d\n,ac4=%d\n,ac5=%d\n,ac6=%d\n,b1=%d\n,b2=%d\n,mb=%d\n,mc=%d\n,md=%d\r\n",ac1,ac2,ac3,ac4,ac5,ac6,b1,b2,mb,mc,md);

        //read uncompensated temperature
        writeI2C1(BMP085_Addr,0xF4,0x2E);
        for(ui32Loop = 0; ui32Loop < 20000; ui32Loop++)
        {
        }
        temp_reg = readI2C1(BMP085_Addr,0xF6);
        temp_reg = (temp_reg<<8)| readI2C1(BMP085_Addr,0xF7);
        //UARTprintf("temp_reg %d \r\n",temp_reg);

        //calculate true temperature
        x1 = ((temp_reg - ac6) * ac5) >> 15;
        x2 = (mc << 11) / (x1 + md);
        b5 = x1 + x2;
        temp = (b5 + 8) >> 4;
        //UARTprintf("x1:%d, x2:%d, b5:%d, temp(*0.1):%d \r\n",x1,x2,b5,temp);

        //read uncompensated pressure
        writeI2C1(BMP085_Addr,0xF4,0x34+(oss<<6));
        for(ui32Loop = 0; ui32Loop < 20000; ui32Loop++)
        {
        }
        pressure = readI2C1(BMP085_Addr,0xF6);
        pressure = (pressure<<8)| readI2C1(BMP085_Addr,0xF7);
        pressure &= 0x0000FFFF;

        IICreadBytes(BMP085_Addr,0xf6,3,BUF);
        press_reg = ((BUF[0] << 16) | (BUF[1] << 8) | BUF[2]) >> (8 - OSS);
        //UARTprintf("press_reg %d \r\n",press_reg);
        //下面计算公式要注意括号的使用，模棱两可的都给它用上，一开始我觉得有些地方应该可以不用，计算出的气压结果却是错的，后来加上括号就对了
        b6 = b5 - 4000;
        //UARTprintf("b6 %d \r\n",b6);
        x1 = (b2 * (b6 * b6 )>> 12) >> 11;
        //UARTprintf("x1 %d \r\n",x1);
        x2 = (ac2 * b6) >> 11;
        //UARTprintf("x2 %d \r\n",x2);
        x3 = x1 + x2;
        //UARTprintf("x3 %d \r\n",x3);
        x1 = (ac3 * b6) >> 13;
        //UARTprintf("x1 %d \r\n",x1);
        b3 = ((((long)ac1 * 4 + x3) << OSS) + 2) / 4;
        //UARTprintf("b3 %d \r\n",b3);
        x2 = (b1 * (b6 * b6) >> 12) >> 16;
        //UARTprintf("x2 %d \r\n",x2);
        x3 = ((x1 + x2 )+ 2) >> 2;
        //UARTprintf("x3 %d \r\n",x3);
        b4 = (ac4 * (unsigned long)(x3 + 32768)) >> 15;
        //UARTprintf("b4 %d \r\n",b4);
        b7 = ((unsigned long)press_reg - b3) * (50000 >> OSS);
        //UARTprintf("b7 %d \r\n",b7);
        if(b7 < 0x80000000)
        {
            pressure = (b7 * 2) / b4;
        }
        else
        {
            pressure = (b7 / b4) * 2;
        }
        //UARTprintf("pressure %d \r\n",pressure);
        x1 = (pressure >> 8) * (pressure  >> 8);
        //UARTprintf("x1 %d \r\n",x1);
        x1 = (x1 * 3038) >> 16;
        //UARTprintf("x1 %d \r\n",x1);
        x2 = (-7357 * pressure) >> 16;
        //UARTprintf("x2 %d \r\n",x2);
        pressure = pressure + ((x1 + x2 + 3791) >> 4);
        //UARTprintf("pressure %d \r\n",pressure);
        for(ui32Loop = 0; ui32Loop < 20000; ui32Loop++)
        {
        }
    }
}
