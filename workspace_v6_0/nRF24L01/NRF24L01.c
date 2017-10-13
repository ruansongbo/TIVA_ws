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

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xb2,0xb2,0xb3,0xb4,0x01};  // ����һ����̬���͵�ַ



uint8_t RX_BUF[TX_PLOAD_WIDTH];

uint8_t TX_BUF[TX_PLOAD_WIDTH]= {1,2,3,4,5};



//SPI initialization,����TM4C123GH6PM��SPIӲ��
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

//��SPI����nrf24l01�ļĴ�������

//********************************************************************
/****��Ĵ���regдһ���ֽڣ�ͬʱ����״̬�ֽ�**************/
uint8_t SPI_RW_Reg(uint8_t WriteReg,uint8_t value)
{
    uint8_t status;
    CSN(0);
    status=SPI_RW(WriteReg);
    SPI_RW(value);
    CSN(1);
    return(status);
}
/****************д��bytes�ֽڵ�����*******************/
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

/****��Ĵ���reg��һ���ֽڣ�ͬʱ����״̬�ֽ�**************/
uint8_t SPI_Read_Reg(uint8_t reg)
{
	uint8_t status;
	CSN(0);
	SPI_RW(reg);
	status=SPI_RW(0);   //select register  and write value to it
	CSN(1);
	return(status);
}
/********����bytes�ֽڵ�����*************************/
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

//�ж�SPI�ӿ��Ƿ����NRFоƬ�Ƿ����
uint8_t NRF24L01_Check(void)
{
   uint8_t buf[5]={0xC2,0xC2,0xC2,0xC2,0xC2};
   uint8_t buf1[5];
   uint8_t i=0;

   /*д��5 ���ֽڵĵ�ַ.  */
   SPI_Write_Buf(WRITE_REG_NRF24L01+TX_ADDR,buf,5);

   /*����д��ĵ�ַ */
   SPI_Read_Buf(TX_ADDR,buf1,5);

    /*�Ƚ�*/
   for (i=0;i<5;i++)
   {
      if (buf1[i]!=0xC2)
      break;
   }

   if (i==5)   {UARTprintf("��ʼ��NRF24L01�ɹ�...\r\n");return 1 ;}        //MCU ��NRF �ɹ�����
   else        {UARTprintf("��ʼ��NRF24L01����...\r\n");return 0 ;}        //MCU��NRF����������
}




/*���պ���������1��ʾ�������յ�������û�����ݽ��յ�**/
uint8_t nRF24L01_RxPacket(uint8_t* rx_buf)
{
    uint8_t status,revale=0;
	CE(0);
	SysCtlDelay(185);
	status=SPI_Read_Reg(STATUS);	// ��ȡ״̬�Ĵ������ж����ݽ���״��
//	CE(0);
//	status=0x40;
	UARTprintf("STATUS����״̬��0x%2x\r\n",status);

	if(status & RX_DR)				// �ж��Ƿ���յ�����
	{
//		CE(1);
		SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
//		CE(0);
		revale =1;			//��ȡ������ɱ�־
	}
	SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS,status);   //���յ����ݺ�RX_DR,TX_DS,MAX_PT���ø�Ϊ1��ͨ��д1������жϱ�־
	CE(1);
	return revale;	
}

 /****************���ͺ���***************************/
void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	CE(0);			//StandBy Iģʽ	
	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // װ�ؽ��ն˵�ַ
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 // װ������	
	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0e);   		 // IRQ�շ�����ж���Ӧ��16λCRC��������
	CE(1);		 //�ø�CE���������ݷ���
	SysCtlDelay(185);
}

void RX_Mode(void)
{
	CE(0);
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // �����豸����ͨ��0ʹ�úͷ����豸��ͬ�ķ��͵�ַ
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RX_PW_P0, TX_PLOAD_WIDTH);  // ����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ�� 
 
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_AA, 0x3f);               // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_RXADDR, 0x3f);           // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_CH, 40);                 // ѡ����Ƶͨ��0x40

  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_SETUP, 0x07);            // ���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0f);              // CRCʹ�ܣ�16λCRCУ�飬�ϵ磬����ģʽ
  	CE(1);
}

void TX_Mode(uint8_t * tx_buf)
{
	CE(0);
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // д�뷢�͵�ַ
  	SPI_Write_Buf(WRITE_REG_NRF24L01 + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ
  	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 // װ������
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_AA, 0x3f);       // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + EN_RXADDR, 0x3f);   // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + SETUP_RETR, 0x0a);  // �Զ��ط���ʱ�ȴ�250us+86us���Զ��ط�10��
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_CH, 40);         // ѡ����Ƶͨ��0x40
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + RF_SETUP, 0x07);    // ���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
	SPI_RW_Reg(WRITE_REG_NRF24L01 + RX_PW_P0, TX_PLOAD_WIDTH);  // ����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ��
  	SPI_RW_Reg(WRITE_REG_NRF24L01 + CONFIG, 0x0e);      // CRCʹ�ܣ�16λCRCУ�飬�ϵ�
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
	status=SPI_Read_Reg(STATUS);	// ��ȡ״̬�Ĵ������ж����ݽ���״��
	if(status&TX_DS)	/*tx_ds == 0x20*/
	{
		UARTprintf("STATUS����״̬��0x%2x\r\n",status);
		UARTprintf("\r\n���������ݣ�%s\r\n",RX_BUF);
		SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x20);      // ���TX����IRQ���ͣ�
			
	}
	else if(status&MAX_RT)
		{
			UARTprintf("���ʹﵽ����ʹ���\n");
			SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x10);      // ���TX����IRQ���ͣ�			
		}
	CE(1);
//	status=20;
}

void NRF24L01_Receive(void)
{   
    uint8_t i,status=0x01;
	RX_Mode();
	while(IRQ);
//	UARTprintf("�����ж�\n");
	CE(0);
	SysCtlDelay(185);
	status=SPI_Read_Reg(STATUS);	// ��ȡ״̬�Ĵ������ж����ݽ���״��
	UARTprintf("STATUS����״̬��0x%2x\r\n",status);
	if(status & 0x40)			//�����жϱ�־λ
	{
		UARTprintf("���ܳɹ�");
//		CE(1);
		SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer

		UARTprintf("\r\n i=%d,���յ����ݣ�%x\r\n",i,RX_BUF[0]);
//		for(i=0;i<32;i++)
//		{
//			RX_BUF[1] = 0X06;
//			UARTprintf("\r\n i=%d,���յ����ݣ�%x\r\n",i,RX_BUF[i]);
//		}
		SPI_RW_Reg(WRITE_REG_NRF24L01 + STATUS, 0x40);      // ���TX����IRQ����
	}  
	CE(1);

}









