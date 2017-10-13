
//*********************************************************************
//* spi_nrf24l01.c
//*********************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "nrf24l01.h"

#define SSI_BitRate 1000000
#define NRF24L01_IRQ GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7)
#define RX_DR    GPIO_PIN_6
#define TX_DR    GPIO_PIN_5
#define MAX_RT   GPIO_PIN_4


//修改该接收和发送地址，可以供多个飞行器在同一区域飞行，数据不受干扰
uint8_t  TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x11};    //本地地址
uint8_t  RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x00};   //接收地址


uint8_t TX_Buf[TX_PLOAD_WIDTH]={'s','p','i','s','t'};
uint8_t RX_Buf[TX_PLOAD_WIDTH];
//*********************************************************************
//SPI initialization,配置TM4C123GH6PM的SPI硬件
void NFR_SPI_TM4C123_init(void)
{

    uint32_t ui32Data;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,SSI_MODE_MASTER,SSI_BitRate,8);
    SSIEnable(SSI0_BASE);
    while(SSIDataGetNonBlocking(SSI0_BASE, &ui32Data)){}
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);
}
//*********************************************************************************
//UART初始化
void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}
//**********************************************************************
uint8_t SPI_RW(uint8_t value)
{
    uint32_t ui32Data;
    uint8_t ui8Data;
    SSIDataPut(SSI0_BASE,value);
    while (SSIBusy(SSI0_BASE)){}
    SSIDataGet(SSI0_BASE, &ui32Data);
    ui8Data = ui32Data & 0xff;
    return(ui8Data);
}
//********************************************************************
//用SPI配置nrf24l01的寄存器函数
uint8_t SPI_Write_register(uint8_t WriteReg,uint8_t Value)//WriteReg要写的寄存器地址，Value要写的寄存器值
{
    uint8_t status;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    status=SPI_RW(WriteReg);
    SPI_RW(Value);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);
    return(status);
}
//**********************************************************************
uint8_t SPI_Write_Buf(uint8_t WriteReg, uint8_t *pBuf, uint8_t bytes)
{
    uint8_t i,status;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    status=SPI_RW(WriteReg);
    for(i=0; i<bytes; i++) // then write all byte in buffer(*pBuf)
    {
        SPI_RW(*pBuf++);
    }
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);
    return(status);
}
//读寄存器
uint8_t SPI_Read_register(uint8_t reg)
{
    uint8_t reg_val;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    SPI_RW(reg);
    reg_val = SPI_RW(0);      /* 读取该寄存器返回数据 */
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);

    return  reg_val;
}
//读缓冲区
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
    uint8_t i;
    uint8_t status;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);                         /* 选通器件 */
    status = SPI_RW(reg);   /* 写寄存器地址 */
    for(i=0; i<uchars; i++)
    {
        pBuf[i] = SPI_RW(0); /* 读取返回数据 */
    }
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);                        /* 禁止该器件 */
    return  status;
}
//***********************************************************************
void nrf24l01init(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);    // 处于待机模式一，所以可以写寄存器
    SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // Writes TX_Address to nRF24L01
    SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
    SPI_Write_register(WRITE_REG+EN_AA,0X01);//失能通道0自动应答,方便单独调试
    SPI_Write_register(WRITE_REG+EN_RXADDR,0X01);//失能接受通道0，方便单独调试发送模块
    SPI_Write_register(WRITE_REG + RF_CH, 0x00);        // Select RF channel 40
    SPI_Write_register(WRITE_REG + RX_PW_P0,RX_PLOAD_WIDTH);
    SPI_Write_register(WRITE_REG + RF_SETUP, 0x07);   // TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR
    SPI_Write_register(WRITE_REG + CONFIG,0x0f);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    SysCtlDelay(2045);//
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
    /*uint8_t sta;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);//NRF24L01_CE=0;
    SPI_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);//NRF24L01_CE=1;//启动发送
    while(NRF24L01_IRQ!=0);//等待发送完成
    sta=SPI_Read_register(STATUS);  //读取状态寄存器的值
    SPI_Write_register(WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
    if(sta&MAX_RT)//达到最大重发次数
    {
        SPI_Write_register(FLUSH_TX,0xff);//清除TX FIFO寄存器
        return MAX_RT;
    }
    if(sta&TX_DR)//发送完成
    {
        return TX_DR;
    }
    return 0xff;//其他原因发送失败*/
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);
    SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
    SPI_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    return 0;
}



//接收函数
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
    uint8_t sta;
    sta=SPI_Read_register(STATUS);  //读取状态寄存器的值
    SPI_Write_register(WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
    if(sta&RX_DR)//接收到数据
    {
        SPI_Write_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
        SPI_Write_register(FLUSH_RX,0xff);//清除RX FIFO寄存器
        return 1;
    }
    return 0;//没收到任何数据
}

void SetRX_Mode(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);
    SPI_Write_register(WRITE_REG + CONFIG,0x0f);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    SysCtlDelay(2045);

}


/***************************************************************************
//检测是否发射成功，成功红色的灯闪一下
void testTXWin(void)
{
    uint32_t ui32Loop;
    uint8_t status,FIFOStatus;
    status=SPIReadStatus(STATUS);
    FIFOStatus=SPIReadStatus(FIFO_STATUS);
    if(status==0x2e&&FIFOStatus==0x11)//红色灯亮
    {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);
        for(ui32Loop = 0; ui32Loop < 2000000; ui32Loop++){}
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0);
        for(ui32Loop = 0; ui32Loop < 2000000; ui32Loop++){}
    }
}*/
//***************************************************************************
int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    InitConsole();
    UARTprintf("SSI ->\n");
    UARTprintf("  Mode: SPI\n");
    UARTprintf("  Data: 8-bit\n\n");
    uint8_t Rxbuf[32];
    uint32_t ui32Loop;
    NFR_SPI_TM4C123_init();

    nrf24l01init();
    //进入一直发射状态
    while(1)
    {
        SetRX_Mode();
        if(NRF24L01_RxPacket(Rxbuf))
        {
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1);
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);
            for(ui32Loop = 0; ui32Loop < 2000000; ui32Loop++){}
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0);
            for(ui32Loop = 0; ui32Loop < 2000000; ui32Loop++){}
        }
        SPI_Write_register(WRITE_REG+STATUS,0xff);
    }
}
