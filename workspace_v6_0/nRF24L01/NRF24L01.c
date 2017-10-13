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



#define	RX_DR			0x40
#define	TX_DS			0x20
#define	MAX_RT			0x10

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xb2,0xb2,0xb3,0xb4,0x01};  // 定义一个静态发送地址



uint8_t RX_BUF[TX_PLOAD_WIDTH];

uint8_t TX_BUF[TX_PLOAD_WIDTH]= {1,2,3,4,5};



//SPI initialization,配置TM4C123GH6PM的SPI硬件
void SPI_TM4C123_init(void)
{
    uint32_t ui32Data;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,SSI_MODE_MASTER,1000000,8);
    SSIEnable(SSI0_BASE);
    while(SSIDataGetNonBlocking(SSI0_BASE, &ui32Data)){}
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_7);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
}

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

//用SPI配置nrf24l01的寄存器函数

//********************************************************************
/****向寄存器reg写一个字节，同时返回状态字节**************/
uint8_t SPI_RW_Reg(uint8_t WriteReg,uint8_t value)
{
    uint8_t status;
    CSN(0);
    status=SPI_RW(WriteReg);
    SPI_RW(value);
    CSN(1);
    return(status);
}
/****************写入bytes字节的数据*******************/
uint8_t SPI_Write_Buf(uint8_t WriteReg, uint8_t *pBuf, uint8_t bytes)
{
    uint8_t i,status;
    CSN(0);
    status=SPI_RW(WriteReg);
    SysCtlDelay(18);
    for(i=0; i<bytes; i++) // then write all byte in buffer(*pBuf)
    {
        SPI_RW(*pBuf++);
    }
    CSN(1);
    return(status);
}

/****向寄存器reg读一个字节，同时返回状态字节**************/
uint8_t SPI_Read_Reg(uint8_t reg)
{
	uint8_t status;
	CSN(0);
	SPI_RW(reg);
	status=SPI_RW(0);   //select register  and write value to it
	CSN(1);
	return(status);
}
/********读出bytes字节的数据*************************/
uint8_t SPI_Read_Buf(uint8_t reg,uint8_t *pBuf,uint8_t bytes)
{
	uint8_t status,byte_ctr;
	CSN(0);
	status=SPI_RW(reg);
	for(byte_ctr=0;byte_ctr<bytes;byte_ctr++)
		pBuf[byte_ctr]=SPI_RW(0);
	CSN(1);
	return(status);
}

//判断SPI接口是否接入NRF芯片是否可用
uint8_t NRF24L01_Check(void)
{
   uint8_t buf[5]={0xC2,0xC2,0xC2,0xC2,0xC2};
   uint8_t buf1[5];
   uint8_t i=0;

   /*写入5 个字节的地址.  */
   SPI_Write_Buf(WRITE_REG_NRF24L01+TX_ADDR,buf,5);

   /*读出写入的地址 */
   SPI_Read_Buf(TX_ADDR,buf1,5);

    /*比较*/
   for (i=0;i<5;i++)
   {
      if (buf1[i]!=0xC2)
      break;
   }

   if (i==5)   {UARTprintf("初始化NRF24L01成功...\r\n");return 1 ;}        //MCU 与NRF 成功连接
   else        {UARTprintf("初始化NRF24L01出错...\r\n");return 0 ;}        //MCU与NRF不正常连接
}




/*接收函数，返回1表示有数据收到，否则没有数据接收到**/
uint8_t nRF24L01_RxPacket(uint8_t* rx_buf)
{
    uint8_t status,revale=0;
	CE(0);
	SysCtlDelay(185);
	status=SPI_Read_Reg(STATUS);	// 读取状态寄存其来判断数据接收状况
//	CE(0);
//	status=0x40;
	UARTprintf("STATUS接受状态：0x%2x\r\n",status);

	if(status & RX_DR)				// 判断是否接收到数据
	{
//		CE(1);
		SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
//		CE(0);
		revale =1;			//读取数据完成标志
	}
	SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS,status);   //接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清楚中断标志
	CE(1);
	return revale;	
}

 /****************发送函数***************************/
void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	CE(0);			//StandBy I模式	
	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 装载接收端地址
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 // 装载数据	
	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0e);   		 // IRQ收发完成中断响应，16位CRC，主发送
	CE(1);		 //置高CE，激发数据发送
	SysCtlDelay(185);
}

void RX_Mode(void)
{
	CE(0);
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // 接收设备接收通道0使用和发送设备相同的发送地址
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RX_PW_P0, TX_PLOAD_WIDTH);  // 接收通道0选择和发送通道相同有效数据宽度 
 
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_AA, 0x3f);               // 使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_RXADDR, 0x3f);           // 使能接收通道0
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_CH, 40);                 // 选择射频通道0x40

  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_SETUP, 0x07);            // 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0f);              // CRC使能，16位CRC校验，上电，接收模式
  	CE(1);
}

void TX_Mode(uint8_t * tx_buf)
{
	CE(0);
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // 写入发送地址
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // 为了应答接收设备，接收通道0地址和发送地址相同
  	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 // 装载数据
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_AA, 0x3f);       // 使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_RXADDR, 0x3f);   // 使能接收通道0
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + SETUP_RETR, 0x0a);  // 自动重发延时等待250us+86us，自动重发10次
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_CH, 40);         // 选择射频通道0x40
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_SETUP, 0x07);    // 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
	SPI_RW_Reg(WRITE_REG_NRF24L01 + RX_PW_P0, TX_PLOAD_WIDTH);  // 接收通道0选择和发送通道相同有效数据宽度
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0e);      // CRC使能，16位CRC校验，上电
	CE(1);
	SysCtlDelay(185);
} 

 
void NRF24L01_Send(void)
{
    uint8_t status=0x00;
//	CSN(0);
	TX_Mode(TX_BUF);
	while(IRQ);

	CE(0);
	SysCtlDelay(185);
	status=SPI_Read_Reg(STATUS);	// 读取状态寄存其来判断数据接收状况
	if(status&TX_DS)	/*tx_ds == 0x20*/
	{
		UARTprintf("STATUS接受状态：0x%2x\r\n",status);
		UARTprintf("\r\n发送完数据：%s\r\n",RX_BUF);
		SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x20);      // 清除TX，让IRQ拉低；
			
	}
	else if(status&MAX_RT)
		{
			UARTprintf("发送达到最大发送次数\n");
			SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x10);      // 清除TX，让IRQ拉低；			
		}
	CE(1);
//	status=20;
}

void NRF24L01_Receive(void)
{   
    uint8_t i,status=0x01;
	RX_Mode();
	while(IRQ);
//	UARTprintf("产生中断\n");
	CE(0);
	SysCtlDelay(185);
	status=SPI_Read_Reg(STATUS);	// 读取状态寄存其来判断数据接收状况
	UARTprintf("STATUS接受状态：0x%2x\r\n",status);
	if(status & 0x40)			//接受中断标志位
	{
		UARTprintf("接受成功");
//		CE(1);
		SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer

		UARTprintf("\r\n i=%d,接收到数据：%x\r\n",i,RX_BUF[0]);
//		for(i=0;i<32;i++)
//		{
//			RX_BUF[1] = 0X06;
//			UARTprintf("\r\n i=%d,接收到数据：%x\r\n",i,RX_BUF[i]);
//		}
		SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x40);      // 清除TX，让IRQ拉低
	}  
	CE(1);

}









