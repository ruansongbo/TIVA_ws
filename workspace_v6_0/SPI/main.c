
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


//�޸ĸý��պͷ��͵�ַ�����Թ������������ͬһ������У����ݲ��ܸ���
uint8_t  TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x11};    //���ص�ַ
uint8_t  RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0xc3,0x10,0x10,0x00};   //���յ�ַ


uint8_t TX_Buf[TX_PLOAD_WIDTH]={'s','p','i','s','t'};
uint8_t RX_Buf[TX_PLOAD_WIDTH];
//*********************************************************************
//SPI initialization,����TM4C123GH6PM��SPIӲ��
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
//UART��ʼ��
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
//��SPI����nrf24l01�ļĴ�������
uint8_t SPI_Write_register(uint8_t WriteReg,uint8_t Value)//WriteRegҪд�ļĴ�����ַ��ValueҪд�ļĴ���ֵ
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
//���Ĵ���
uint8_t SPI_Read_register(uint8_t reg)
{
    uint8_t reg_val;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    SPI_RW(reg);
    reg_val = SPI_RW(0);      /* ��ȡ�üĴ����������� */
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);

    return  reg_val;
}
//��������
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
    uint8_t i;
    uint8_t status;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);                         /* ѡͨ���� */
    status = SPI_RW(reg);   /* д�Ĵ�����ַ */
    for(i=0; i<uchars; i++)
    {
        pBuf[i] = SPI_RW(0); /* ��ȡ�������� */
    }
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);                        /* ��ֹ������ */
    return  status;
}
//***********************************************************************
void nrf24l01init(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);    // ���ڴ���ģʽһ�����Կ���д�Ĵ���
    SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // Writes TX_Address to nRF24L01
    SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
    SPI_Write_register(WRITE_REG+EN_AA,0X01);//ʧ��ͨ��0�Զ�Ӧ��,���㵥������
    SPI_Write_register(WRITE_REG+EN_RXADDR,0X01);//ʧ�ܽ���ͨ��0�����㵥�����Է���ģ��
    SPI_Write_register(WRITE_REG + RF_CH, 0x00);        // Select RF channel 40
    SPI_Write_register(WRITE_REG + RX_PW_P0,RX_PLOAD_WIDTH);
    SPI_Write_register(WRITE_REG + RF_SETUP, 0x07);   // TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR
    SPI_Write_register(WRITE_REG + CONFIG,0x0f);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    SysCtlDelay(2045);//
}
//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
    /*uint8_t sta;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);//NRF24L01_CE=0;
    SPI_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);//NRF24L01_CE=1;//��������
    while(NRF24L01_IRQ!=0);//�ȴ��������
    sta=SPI_Read_register(STATUS);  //��ȡ״̬�Ĵ�����ֵ
    SPI_Write_register(WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
    if(sta&MAX_RT)//�ﵽ����ط�����
    {
        SPI_Write_register(FLUSH_TX,0xff);//���TX FIFO�Ĵ���
        return MAX_RT;
    }
    if(sta&TX_DR)//�������
    {
        return TX_DR;
    }
    return 0xff;//����ԭ����ʧ��*/
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);
    SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
    SPI_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    return 0;
}



//���պ���
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
    uint8_t sta;
    sta=SPI_Read_register(STATUS);  //��ȡ״̬�Ĵ�����ֵ
    SPI_Write_register(WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
    if(sta&RX_DR)//���յ�����
    {
        SPI_Write_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
        SPI_Write_register(FLUSH_RX,0xff);//���RX FIFO�Ĵ���
        return 1;
    }
    return 0;//û�յ��κ�����
}

void SetRX_Mode(void)
{
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0);
    SPI_Write_register(WRITE_REG + CONFIG,0x0f);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6);
    SysCtlDelay(2045);

}


/***************************************************************************
//����Ƿ���ɹ����ɹ���ɫ�ĵ���һ��
void testTXWin(void)
{
    uint32_t ui32Loop;
    uint8_t status,FIFOStatus;
    status=SPIReadStatus(STATUS);
    FIFOStatus=SPIReadStatus(FIFO_STATUS);
    if(status==0x2e&&FIFOStatus==0x11)//��ɫ����
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
    //����һֱ����״̬
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
