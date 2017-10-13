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
#include "i2c.h"
#include "ADXL345.h"
#include "extern.h"
#include "HMC5883.h"
#include "L3G4200D.h"
#include "BMP085.h"

#define ADXL_SlaveAddress       0x53
#define L3G4_SlaveAddress       0x69
#define BMP085_SlaveAddress     0x77
#define hmc_SlaveAddress        0x1E

axis_f accel;
axis_d magnet;
axis_d gyro;
extern BMP BMPs;

void Delay_ms(uint16_t time)
{
    SysCtlDelay(SysCtlClockGet()*time/6000);
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
uint32_t angle;
uint8_t device;
void main(void)
{
    systeminit();
    InitI2C1();
    InitConsole();
    //Init_ADXL345();
    //HMC5883_Init();
    //InitL3G4200D();

    while(1)
    {
        Init_BMP085();
        read_BMP085();
        //device=readI2C1(0x69, 0x0f);
        //ADXL_getdate(&accel);
        //angle=HMC5883_getdata(&magnet);
        //L3G4_getdata(&gyro);
        //read uncompensated temperature
        /*writeI2C1(BMP085_SlaveAddress,0xF4,0x2E);
        SysCtlDelay(14025); // max time is 4.5ms
        ret = I2C_Read(Open_I2C,ADDR_24LC02,0xf6,ReadBuffer,2);
        temp_reg = ReadBuffer[0] << 8 | ReadBuffer[1];
        printf("temp_reg %d \r\n",temp_reg);

        //calculate true temperature
        x1 = ((temp_reg - ac6) * ac5) >> 15;
        x2 = (mc << 11) / (x1 + md);
        b5 = x1 + x2;
        temp = (b5 + 8) >> 4;
        printf("x1:%d, x2:%d, b5:%d, temp(*0.1):%d \r\n",x1,x2,b5,temp);

        //read uncompensated pressure
        I2C_WriteOneByte(Open_I2C,ADDR_24LC02,0xf4,(0x3e + oss<<6));
        I2C_delay(200000);//delay 4.5ms
        ret = I2C_Read(Open_I2C,ADDR_24LC02,0xf6,ReadBuffer,3);
        press_reg = ((ReadBuffer[0] << 16) | (ReadBuffer[1] << 8) | ReadBuffer[2]) >> (8 - oss);
        printf("press_reg %d \r\n",press_reg);
        //下面计算公式要注意括号的使用，模棱两可的都给它用上，一开始我觉得有些地方应该可以不用，计算出的气压结果却是错的，后来加上括号就对了
        b6 = b5 - 4000;
        printf("b6 %ld \r\n",b6);
        x1 = (b2 * (b6 * b6 )>> 12) >> 11;
        printf("x1 %ld \r\n",x1);
        x2 = (ac2 * b6) >> 11;
        printf("x2 %ld \r\n",x2);
        x3 = x1 + x2;
        printf("x3 %ld \r\n",x3);
        b3 = ((((long)ac1 * 4 + x3) << oss) + 2) / 4;
        printf("b3 %ld \r\n",b3);
        x1 = (ac3 * b6) >> 13;
        printf("x1 %ld \r\n",x1);
        x2 = (b1 * (b6 * b6) >> 12) >> 16;
        printf("x2 %ld \r\n",x2);
        x3 = ((x1 + x2 )+ 2) >> 2;
        printf("x3 %ld \r\n",x3);
        b4 = (ac4 * (unsigned long)(x3 + 32768)) >> 15;
        printf("b4 %ld \r\n",b4);
        b7 = ((unsigned long)press_reg - b3) * (50000 >> oss);
        printf("b7 %ld \r\n",b7);
        if(b7 < 0x80000000)
        {
            pressure = (b7 * 2) / b4;
        }
        else
        {
            pressure = (b7 / b4) * 2;
        }
        printf("pressure %ld \r\n",pressure);
        x1 = (pressure >> 8) * (pressure >> 8);
        printf("x1 %ld \r\n",x1);
        x1 = (x1 * 3038) >> 16;
        printf("x1 %ld \r\n",x1);
        x2 = (-7357 * pressure) >> 16;
        printf("x2 %ld \r\n",x2);
        pressure = pressure + ((x1 + x2 + 3791) >> 4);
        printf("pressure %ld \r\n",pressure);

        for(i=0; i<200*3; i++)
            I2C_delay(200000);//delay 4.5ms
    }*/


}
