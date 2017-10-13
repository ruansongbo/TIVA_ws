#include "i2c.h"
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



