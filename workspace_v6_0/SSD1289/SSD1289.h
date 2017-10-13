#ifndef SSD1289_H_
#define SSD1289_H_
#include"char_labs.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
/*pin_map***********************
 * 8_DATE_L D0   -   PC5
 *          D1   -   PC6
 *          D2   -   PA2
 *          D3   -   PA3
 *          D4   -   PA4
 *          D5   -   PA5
 *          D6   -   PA6
 *          D7   -   PA7
 * 8_DATE_H D8   -   PB0
 *          D9   -   PB1
 *          D10  -   PB2
 *          D11  -   PB3
 *          D12  -   PB4
 *          D13  -   PB5
 *          D14  -   PB6
 *          D15  -   PB7
 * 控制信号              RS   -   PE1
 *          WR   -   PE2
 *          RD   -   PE3
 *          CS   -   PE4
 *          RSET -   PE5
 */
#define  LCD_RS_H       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1);
#define  LCD_RS_L       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
#define  LCD_WR_H       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
#define  LCD_WR_L       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
#define  LCD_RD_H       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3,GPIO_PIN_3);
#define  LCD_RD_L       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3,0x00);
#define  LCD_CS_H       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4,GPIO_PIN_4);
#define  LCD_CS_L       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4,0x00);
#define  LCD_REST_H     GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,GPIO_PIN_5);
#define  LCD_REST_L     GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,0x00);


#define SMALL 		0
#define MEDIUM 		1
#define LARGE 		2

#define NOFILL 		0
#define FILL 		1

#define SSD1289_LN	320
#define SSD1289_COL	240

#define WHITE          0xFFFF
#define BLACK          0x0000
#define GREY           0xF7DE
#define BLUE           0x001F
#define _BLUE          0x051F
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define TEST           0xFF55


/*     F149
#define DAT_OUT_H      P4OUT
#define DAT_OUT_L      P5OUT
#define CMD            P2OUT
#define CS             BIT0
#define RS             BIT1
#define WR             BIT2
#define RD             BIT3
#define RESET          BIT4

void CS_HIGH(){CMD |= CS;}
void CS_LOW(){CMD &=~ CS;}
void RS_HIGH(){CMD |= RS;}
void RS_LOW(){CMD &=~ RS;}
void WR_HIGH(){CMD |= WR;}
void WR_LOW(){CMD &=~ WR;}
void RD_HIGH(){CMD |= RD;}
void RD_LOW(){CMD &=~ RD;}
void RESET_HIGH(){CMD |= RESET;}
void RESET_LOW(){CMD &=~ RESET;}



void Write_cmd(int cmd)	 //命令
{	
    RS_LOW();
	CS_LOW();	 
	DAT_OUT_H = cmd>>8;	
	DAT_OUT_L = cmd;		
	WR_LOW();
	WR_HIGH();
	CS_HIGH();	
}
void Write_dat(int dat)	//数据	
{
    RS_HIGH();
	CS_LOW();	  				
	DAT_OUT_H = dat>>8;	
	DAT_OUT_L = dat;					
	WR_LOW();
	WR_HIGH();
	CS_HIGH();	
}*/
/*#define  LCD_DataPortH_0
#define  LCD_DataPortH     P3OUT        //8 wei 
//#define  LCD_DataPortL_0
#define  LCD_DataPortL     P2OUT        //P2.6 meiyou  P8.1=0wei


#define  O_DIR_DATA_H        P3DIR=0xff
#define  O_DIR_DATA_H_16     P4DIR|=BIT0
#define  O_DIR_DATA_L        P2DIR|=BIT0+BIT6+BIT2+BIT3+BIT4+BIT5+BIT7
#define  O_DIR_DATA_L_0      P8DIR|=BIT1

#define  I_DIR_DATA_H        P3DIR=0x00
#define  I_DIR_DATA_H_16     P4DIR&=~BIT0
#define  I_DIR_DATA_L        P2DIR&=~(BIT0+BIT6+BIT2+BIT3+BIT4+BIT5+BIT7)
#define  I_DIR_DATA_L_0      P8DIR&=~BIT1*/


void InitGPIO(void)
{
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
     GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, 0xFC);
     GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, 0xFF);
     GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, 0x60);
     GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x3e);
}



void Write_cmd(int cmd)	 //命令
{	
	char VH,VL;
	VH=cmd>>8;
	VL=cmd&0x00ff;
    LCD_RS_L;
	LCD_CS_L;

	GPIOPinWrite(GPIO_PORTB_BASE,0xFF,VH);
	GPIOPinWrite(GPIO_PORTA_BASE,0xFC,0xFC&VL);
	GPIOPinWrite(GPIO_PORTC_BASE,0x60,0x60&(VL<<5));

	LCD_WR_L;
	LCD_WR_H;
	LCD_CS_H;	
}
void Write_dat(int dat)	//数据	
{
	char VH,VL;
	VH=dat>>8;
	VL=dat&0x00ff;
    LCD_RS_H;
	LCD_CS_L;

    GPIOPinWrite(GPIO_PORTB_BASE,0xFF,VH);
    GPIOPinWrite(GPIO_PORTA_BASE,0xFC,0xFC&VL);
    GPIOPinWrite(GPIO_PORTC_BASE,0x60,0x60&(VL<<5));

	LCD_WR_L;
	LCD_WR_H;
	LCD_CS_H;	
}



void LCD_WriteReg(int cmd,int dat)	//命令数据一起 
{
   Write_cmd(cmd);
   Write_dat(dat);
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)   //显示区域
{ 
	LCD_WriteReg(0x0044,(x2<<8)+x1);
	LCD_WriteReg(0x0045,y1);
	LCD_WriteReg(0x0046,y2);
	LCD_WriteReg(0x004e,x1);
	LCD_WriteReg(0x004f,y1);
    Write_cmd(0x0022);	     	   	
}	

void init_ssd1289()
{

	
	LCD_REST_H;
	ROM_SysCtlDelay(460000);
	LCD_REST_L;
	ROM_SysCtlDelay(460000);
	LCD_REST_H;
	LCD_CS_H;
	LCD_RD_H;
	LCD_WR_H;
	ROM_SysCtlDelay(460000);                    // 读ID用
	LCD_CS_L;
    
    LCD_WriteReg(0x0000,0x0001);//打开晶振
    LCD_WriteReg(0x0003,0xA8A4);//0xA8A4
    LCD_WriteReg(0x000C,0x0000);    
    LCD_WriteReg(0x000D,0x080C);   
    LCD_WriteReg(0x000E,0x2B00);    
    LCD_WriteReg(0x001E,0x00B0);    
    LCD_WriteReg(0x0001,0x2B3F);//驱动输出控制320*240  0x6B3F 293F 2B3F 6B3F 7A3F
    LCD_WriteReg(0x0002,0x0600);
    LCD_WriteReg(0x0010,0x0000);
 
    LCD_WriteReg(0x0011,0x6070);
 
    LCD_WriteReg(0x0005,0x0000);  
    LCD_WriteReg(0x0006,0x0000);  
    LCD_WriteReg(0x0016,0xEF1C);  //设置每行有效像素的数目。每行有效像素数等于到XL[7:0]+1
    LCD_WriteReg(0x0017,0x0003);  
    LCD_WriteReg(0x0007,0x0233); //0x0233       
    LCD_WriteReg(0x000B,0x5308); //帧周期控制 65
    LCD_WriteReg(0x000F,0x0000); //栅极扫描开始地址
    LCD_WriteReg(0x0041,0x0000); //垂直滚动控制 
    LCD_WriteReg(0x0042,0x0000); //垂直滚动控制  
    LCD_WriteReg(0x0048,0x0000);  //第一个局部显示的起始位置 SS1[8:0]
    LCD_WriteReg(0x0049,0x013F);  //第一个局部显示的结束位置SE1[8:0]  SS1<SE1<13EF
    LCD_WriteReg(0x004A,0x0000);  //第二个局部显示的起始位置
    LCD_WriteReg(0x004B,0x0000);  //第二个局部显示的结束位置
    LCD_WriteReg(0x0044,0xEF00); //水平RAM的起始-结束地址位置 (00~EF)
    LCD_WriteReg(0x0045,0x0000);  //垂直RAM的起始地址 00
    LCD_WriteReg(0x0046,0x013F); //垂直RAM的结束地址 13F
    LCD_WriteReg(0x0030,0x0707);  
    LCD_WriteReg(0x0031,0x0204);  
    LCD_WriteReg(0x0032,0x0204);  
    LCD_WriteReg(0x0033,0x0502);  
    LCD_WriteReg(0x0034,0x0507);  
    LCD_WriteReg(0x0035,0x0204);  
    LCD_WriteReg(0x0036,0x0204);  
    LCD_WriteReg(0x0037,0x0502);  
    LCD_WriteReg(0x003A,0x0302);  
    LCD_WriteReg(0x003B,0x0302);  
    LCD_WriteReg(0x0023,0x0000); //RAM写入数据 
    LCD_WriteReg(0x0024,0x0000);  //RAM写入数据
    LCD_WriteReg(0x0025,0x8000); //帧频率 510K
    LCD_WriteReg(0x004f,0);        //设置GDDRAM Y的计数地址
    LCD_WriteReg(0x004e,0);        //设置GDDRAM X的计数地址 
}

void lcd_clear()                  //清屏
{
	int i,j;
	Address_set(0,0,239,319);
	
    for(i=0;i<240;i++)
	{
	    for (j=0;j<320;j++)
	    {
              Write_dat(WHITE);
	    }

    }		
}

void lcd_Fringe()                      //显示条纹
{
	int i,j;
	Address_set(0,0,239,319);
	
    for(i=0;i<80;i++)
	{
	    for (j=0;j<240;j++)
	    {
              Write_dat(0x0000);
	    }
    }	    
    for(i=80;i<160;i++)
	{
	    for (j=0;j<240;j++)
	    {
              Write_dat(0x07E0);
	    }
    }	    
    for(i=160;i<240;i++)
	{
	    for (j=0;j<240;j++)
	    {
              Write_dat(0xF800);
	    }
    }	    
    for(i=240;i<320;i++)
	{
	    for (j=0;j<240;j++)
	    {
              Write_dat(0xffff);
	    }
    }	    
}

void LCD_DefineDispWindow(unsigned int xAddr0,unsigned int xAddr1,unsigned int yAddr0,unsigned int yAddr1)
{
     Address_set(xAddr0,yAddr0,xAddr1,yAddr1);
}

void LCD_DispOnePixel(unsigned int xAddr,unsigned int yAddr,unsigned long color) 
{
	LCD_DefineDispWindow(xAddr,xAddr,yAddr,yAddr);
	Write_dat(color);
}

void LCD_DispOneChar(unsigned char chr,unsigned int xAddr,unsigned int yAddr,
					 unsigned char size,unsigned long fColor,unsigned long bColor) 
{
	unsigned char i,j;
	unsigned char nCols;
	unsigned char nRows;
	unsigned char nBytes;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned long Word;
	unsigned char *pFont;
	unsigned char *pChar;
	unsigned char *FontTable[] = {
									(unsigned char *)FONT6x8,
									(unsigned char *)FONT8x8,
									(unsigned char *)FONT8x16};
	// get pointer to the beginning of the selected font table
	pFont = (unsigned char *)FontTable[size];
	
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);

	// get pointer to the first byte of the desired character
	pChar = pFont + nBytes * (chr - 0x1F);
	
	//define display window
	LCD_DefineDispWindow(xAddr,xAddr + nCols - 1,yAddr,yAddr + nRows - 1);
	
	// loop on each row, working backwards from the top to the bottom
	for (i = 0; i < nRows; i++) 
	{
		// copy pixel row from font table and then increment row
		PixelRow = *pChar++;
		
		// loop on each pixel in the row (left to right)
		// Note: we do one pixel each loop
		Mask = 0x80;
		for (j = 0; j < nCols; j++) 
		{
			// if pixel bit set, use foreground color; else use the background color
			if ((PixelRow & Mask) == 0)
				Word = bColor;
			else
				Word = fColor;
			Mask = Mask >> 1;
			Write_dat(Word);
		
		}
	}
}

void LCD_DispStr(unsigned char *pStr,unsigned int xAddr,unsigned int yAddr,unsigned char size,unsigned long fColor,unsigned long bColor) 
{
	// loop until null-terminator is seen
	while (*pStr != 0x00) 
	{
		// draw the character
		LCD_DispOneChar(*pStr++,xAddr,yAddr,size,fColor,bColor);
		// advance the X position
		if (size == SMALL)
			xAddr += 6;
		else if (size == MEDIUM)
			xAddr += 8;
		else
			xAddr += 8;
		// bail out if X exceeds ILI9325_COL
		if (xAddr > SSD1289_COL) 
			break;
	}
}

void LCD_DispOneMaxChar(unsigned char chr,unsigned int xAddr,unsigned int yAddr,
					    unsigned long fColor,unsigned long bColor) 
{
	unsigned char i,j;
	unsigned char nCols;
	unsigned char nRows;
	unsigned char nBytes;
	unsigned int PixelRow;
	unsigned int Mask;
	unsigned long Word;
	unsigned int *pFont;
	unsigned int *pChar;
	
	// get pointer to the beginning of the selected font table
	pFont = (unsigned int *)FONT16x24;
	
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);

	// get pointer to the first byte of the desired character
	pChar = pFont + nBytes*(chr - 0x1F);
	
	//define display window
	LCD_DefineDispWindow(xAddr,xAddr + nCols - 1,yAddr,yAddr + nRows - 1);
	
	// loop on each row, working backwards from the top to the bottom
	for (i = 0; i < nRows; i++) 
	{
		// copy pixel row from font table and then increment row
		PixelRow = *pChar++;
		
		// loop on each pixel in the row (left to right)
		// Note: we do one pixel each loop
		Mask = 0x0001;
		for (j = 0; j < nCols; j++) 
		{
			// if pixel bit set, use foreground color; else use the background color
			if ((PixelRow & Mask) == 0)
				Word = bColor;
			else
				Word = fColor;
			Mask = Mask << 1;
			
		
		
			Write_dat(Word);
		
		}
	}
}

void LCD_DispMaxStr(unsigned char *pStr,unsigned int xAddr,unsigned int yAddr,unsigned long fColor,unsigned long bColor) 
{
	// loop until null-terminator is seen
	while (*pStr != 0x00) 
	{
		// draw the character
		LCD_DispOneMaxChar(*pStr++,xAddr,yAddr,fColor,bColor);
		// advance the X position
		xAddr += 16;
		// bail out if X exceeds ILI9325_COL
		if (xAddr > SSD1289_COL) 
			break;
	}
}
void LCD_DispNUM(const unsigned char *ptr,unsigned int xAddr,unsigned int yAddr,
					 unsigned char chr_width,unsigned long fColor,unsigned long bColor)
{
	unsigned char i,j,k;
	unsigned char nCols;
	unsigned char nRows;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned long Word;
	const unsigned char *pChar;
	
	// get pointer to the first byte of the desired character
	pChar = ptr;
	nCols = chr_width;
	nRows = 48;
	
	//define display window
	LCD_DefineDispWindow(xAddr,xAddr + nCols - 1,yAddr,yAddr + nRows - 1);
	
	// loop on each row, working backwards from the top to the bottom
	for (i = 0; i < nRows; i++) 
	{
		// copy pixel row from font table and then increment row
		for (j = 0; j < chr_width/8; j++)
		{
			PixelRow = *pChar++;
				
			// loop on each pixel in the row (left to right)
			// Note: we do one pixel each loop
			Mask = 0x80;
			for (k = 0; k < 8; k++) 
			{
				// if pixel bit set, use foreground color; else use the background color
				if ((PixelRow & Mask) == 0)
				        Word = bColor;
				else
					Word = fColor;
				Mask = Mask >> 1;
				Write_dat(Word);
			
			}
		}
		// copy the last byte of one pixel row from font table
                if(chr_width%8)
                {
                        PixelRow = *pChar++;
				
                        // loop on each pixel in the row (left to right)
                        // Note: we do one pixel each loop
                        Mask = 0x80;
                        for (k = 0; k < chr_width%8; k++) 
                        {
                                // if pixel bit set, use foreground color; else use the background color
                                if ((PixelRow & Mask) == 0)
                                        Word = bColor;
                                else
                                        Word = fColor;
                                Mask = Mask >> 1;
        
                                Write_dat(Word);
                       
                        }
                }
	}
}
void LCD_DispChinese(const unsigned char *ptr,unsigned int xAddr,unsigned int yAddr,
					 unsigned char chr_width,unsigned long fColor,unsigned long bColor)
{
	unsigned char i,j,k;
	unsigned char nCols;
	unsigned char nRows;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned long Word;
	const unsigned char *pChar;
	
	// get pointer to the first byte of the desired character
	pChar = ptr;
	nCols = chr_width;
	nRows = chr_width;
	
	//define display window
	LCD_DefineDispWindow(xAddr,xAddr + nCols - 1,yAddr,yAddr + nRows - 1);
	
	// loop on each row, working backwards from the top to the bottom
	for (i = 0; i < nRows; i++) 
	{
		// copy pixel row from font table and then increment row
		for (j = 0; j < chr_width/8; j++)
		{
			PixelRow = *pChar++;
				
			// loop on each pixel in the row (left to right)
			// Note: we do one pixel each loop
			Mask = 0x80;
			for (k = 0; k < 8; k++) 
			{
				// if pixel bit set, use foreground color; else use the background color
				if ((PixelRow & Mask) == 0)
				        Word = bColor;
				else
					Word = fColor;
				Mask = Mask >> 1;
				Write_dat(Word);
			
			}
		}
		// copy the last byte of one pixel row from font table
                if(chr_width%8)
                {
                        PixelRow = *pChar++;
				
                        // loop on each pixel in the row (left to right)
                        // Note: we do one pixel each loop
                        Mask = 0x80;
                        for (k = 0; k < chr_width%8; k++) 
                        {
                                // if pixel bit set, use foreground color; else use the background color
                                if ((PixelRow & Mask) == 0)
                                        Word = bColor;
                                else
                                        Word = fColor;
                                Mask = Mask >> 1;
        
                                Write_dat(Word);
                       
                        }
                }
	}
}

void LCD_DispChinese16(const unsigned char hanzi[3],unsigned int xAddr,unsigned int yAddr,
					unsigned long fColor,unsigned long bColor)
{
	unsigned int  x=0;
        unsigned char *ptr;
	while((hanzi[0]!=HANZI16[x].index[0])||(hanzi[1]!=HANZI16[x].index[1]))
        {
          x++;
          if(x>65000)
          {
            break;
          }
        }
        ptr=HANZI16[x].code;
        // get pointer to the first byte of the desired character
	LCD_DispChinese(ptr,xAddr,yAddr,16,fColor,bColor);
}

void LCD_DispChineseString16(const unsigned char *hanzi,unsigned char num,unsigned char distance,unsigned int xAddr,unsigned int yAddr,
					unsigned long fColor,unsigned long bColor)
{
	unsigned char i;
	unsigned int  x=0;
        unsigned char *ptr;
	for(i=0;i<num;i++)
	{
		while((hanzi[i*2]!=HANZI16[x].index[0])||(hanzi[(i*2)+1]!=HANZI16[x].index[1]))
              {
                  x++;
                  if(x>65000)
                  {
                   break;
                  }
               }
        ptr=HANZI16[x].code;
	LCD_DispChinese(ptr,xAddr+i*distance,yAddr,16,fColor,bColor);
	x=0;
	}
}	

void LCD_DispChineseStr(const unsigned char *ptr,unsigned int xAddr,unsigned int yAddr,
						unsigned char chr_width,unsigned char distance,unsigned char num,
						unsigned long fColor,unsigned long bColor)
{
	unsigned char i;

	for(i = 0;i < num;i++,xAddr += (chr_width+distance))
	{
		if(xAddr > SSD1289_COL)
			break;
		LCD_DispChinese(ptr,xAddr,yAddr,chr_width,fColor,bColor);
		if(chr_width%8)
			ptr += chr_width*(chr_width/8 + 1);
		else
			ptr += chr_width*(chr_width/8);
	}
}

void LCD_DispNum(unsigned int num,unsigned int xAddr,unsigned int yAddr,unsigned char size,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[5];
	
	do//data process
	{
		tmp[i] = (num%10 + '0');
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispOneChar(tmp[cnt-1-i],xAddr,yAddr,size,fColor,bColor);
		if (size == SMALL)
			xAddr += 6;
		else if (size == MEDIUM)
			xAddr += 8;
		else
			xAddr += 8;
	}
}

void LCD_DispDecimal(unsigned int num,unsigned char pos,unsigned int xAddr,unsigned int yAddr,unsigned char size,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[6];
	do//data process
	{
		tmp[i] = (num%10 + '0');
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	if(pos >= cnt)
	{
		for(i = 0; i < pos - cnt;i++)
			tmp[cnt + i] = '0'; 
		tmp[pos] = '.';
		tmp[pos + 1] = '0';
		cnt = pos + 2;
	}
	else
	{
		for(i = cnt - 1;i >= pos;i--)
			tmp[i + 1] = tmp[i];
		tmp[pos] = '.';
		cnt += 1;
	}

	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispOneChar(tmp[cnt-1-i],xAddr,yAddr,size,fColor,bColor);
		if (size == SMALL)
			xAddr += 6;
		else if (size == MEDIUM)
			xAddr += 8;
		else
			xAddr += 8;
	}
}

void LCD_DispMaxNum(unsigned int num,unsigned int xAddr,unsigned int yAddr,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[5];
	
	do//data process
	{
		tmp[i] = (num%10 + '0');
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispOneMaxChar(tmp[cnt-1-i],xAddr,yAddr,fColor,bColor);
		xAddr += 16;
	}
}
/*************************************************************
函数名称:LCD_DispMaxDecimal
功    能:将数字转换为字符串送到液晶显示
参    数:num:  要显示的数字(0~65535)16x24点阵
		 pos:  显示小数位数(0~2)
		 xAddr:  X坐标 
		 yAddr:  Y坐标
		 fColor:  字符前景色
		 bColor:  字符背景色
返 回 值:无
*************************************************************/
void LCD_DispMaxDecimal(unsigned int num,unsigned char pos,unsigned int xAddr,unsigned int yAddr,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[6];
	
	do//data process
	{
		tmp[i] = (num%10 + '0');
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	if(pos >= cnt)
	{
		for(i = 0; i < pos - cnt;i++)
			tmp[cnt + i] = '0'; 
		tmp[pos] = '.';
		tmp[pos + 1] = '0';
		cnt = pos + 2;
	}
	else
	{
		for(i = cnt - 1;i >= pos;i--)
			tmp[i + 1] = tmp[i];
		tmp[pos] = '.';
		cnt += 1;
	}

	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispOneMaxChar(tmp[cnt-1-i],xAddr,yAddr,fColor,bColor);
		xAddr += 16;
	}
}
void LCD_DispMaxNUM(unsigned int num,unsigned int xAddr,unsigned int yAddr,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[5];
	
	do//data process
	{
		tmp[i] = (num%10);
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispNUM(NUM+tmp[i]*144,xAddr,yAddr,24,fColor,bColor);
		xAddr -= 24;
	}
}
/*
void LCD_DispMaxDecimal(unsigned int num,unsigned char pos,unsigned int xAddr,unsigned int yAddr,unsigned long fColor,unsigned long bColor)
{
	unsigned char i = 0,cnt = 0,tmp[6];
	
	do//data process
	{
		tmp[i] = (num%10 + '0');
		i++;
		cnt++;
		num /= 10;	
	}while(num);
	
	if(pos >= cnt)
	{
		for(i = 0; i < pos - cnt;i++)
			tmp[cnt + i] = '0'; 
		tmp[pos] = '.';
		tmp[pos + 1] = '0';
		cnt = pos + 2;
	}
	else
	{
		for(i = cnt - 1;i >= pos;i--)
			tmp[i + 1] = tmp[i];
		tmp[pos] = '.';
		cnt += 1;
	}

	for( i = 0;i < cnt;i++)//to display
	{
		// bail out if X exceeds 131
		if (xAddr > SSD1289_COL) 
			break;
		LCD_DispOneMaxChar(tmp[cnt-1-i],xAddr,yAddr,fColor,bColor);
		xAddr += 16;
	}
}*/
/*************************************************************
函数名称:LCD_DispRandomLine
功    能:绘制任意直线
参    数:xAddr0:  起始点X坐标
		 yAddr0:  起始点Y坐标
		 xAddr1:  终点X坐标
		 yAddr1:  终点Y坐标
		 color:  直线颜色
返 回 值:无
*************************************************************/
void LCD_DispRandomLine(int xAddr0, int yAddr0, int xAddr1, int yAddr1, unsigned long color) 
{
	int dy = yAddr1 - yAddr0;
	int dx = xAddr1 - xAddr0;
	int stepx, stepy;
	
	if (dy < 0) 
	{ 
		dy = -dy; 
		stepy = -1; 
	} 
	else 
	{ 
		stepy = 1; 
	}
	if (dx < 0) 
	{ 
		dx = -dx; 
		stepx = -1; 
	} 
	else 
	{ 
		stepx = 1; 
	}
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx
	LCD_DispOnePixel(xAddr0, yAddr0, color);
	if (dx > dy) 
	{
		int fraction = dy - (dx >> 1); // same as 2*dy - dx
		while (xAddr0 != xAddr1) 
		{
			if (fraction >= 0) 
			{
				yAddr0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			xAddr0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			LCD_DispOnePixel(xAddr0, yAddr0, color);
		}
	} 
	else 
	{
		int fraction = dx - (dy >> 1);
		while (yAddr0 != yAddr1) 
		{
			if (fraction >= 0) 
			{
				xAddr0 += stepx;
				fraction -= dy;
			}
			yAddr0 += stepy;
			fraction += dx;
			LCD_DispOnePixel(xAddr0, yAddr0, color);
		}
	}
}
/*************************************************************
函数名称:LCD_DispRect
功    能:绘制矩形
参    数:xAddr0:  对角线起点X坐标
		 yAddr0:  对角线起始点Y坐标
		 xAddr1:  对角线终点X坐标
		 yAddr1:  对角线终点Y坐标
		 fill:  矩形是否填充(FILL: 填充 NOFILL: 不填充)
		 color:  矩形颜色
返 回 值:无
*************************************************************/
void LCD_DispRect(int xAddr0, int yAddr0, int xAddr1, int yAddr1, unsigned char fill, unsigned long color) 
{
	int xmin, xmax, ymin, ymax;
	int i;
	
	// check if the rectangle is to be filled
	if (fill == FILL) 
	{
		// best way to create a filled rectangle is to define a drawing box
		// and loop two pixels at a time
		// calculate the min and max for x and y directions
		xmin = (xAddr0 <= xAddr1) ? xAddr0 : xAddr1;
		xmax = (xAddr0 > xAddr1) ? xAddr0 : xAddr1;
		ymin = (yAddr0 <= yAddr1) ? yAddr0 : yAddr1;
		ymax = (yAddr0 > yAddr1) ? yAddr0 : yAddr1;
		
		// specify the controller drawing box according to those limits
		LCD_DefineDispWindow(xmin,xmax,ymin,ymax);

		// loop on total number of pixels
		for (i = 0; i < (((xmax - xmin + 1) * (ymax - ymin + 1))); i++) 
		{
			// use the color value to output three data bytes covering two pixels
		#ifdef _262K_COLORS_MODE
			Write_dat(color,GRAM);
		#endif

		#ifdef _65K_COLORS_MODE
			Write_dat(color);
		#endif
		}
	} 
	else 
	{
		// best way to draw un unfilled rectangle is to draw four lines
		LCD_DispRandomLine(xAddr0, yAddr0, xAddr1, yAddr0, color);
		LCD_DispRandomLine(xAddr0, yAddr1, xAddr1, yAddr1, color);
		LCD_DispRandomLine(xAddr0, yAddr0, xAddr0, yAddr1, color);
		LCD_DispRandomLine(xAddr1, yAddr0, xAddr1, yAddr1, color);
	}
}
/*************************************************************
函数名称:LCD_DispCircle
功    能:绘制圆
参    数:xAddr:  圆心X坐标
	     yAddr:  圆心Y坐标
		 radius:  半径
		 color:  绘制圆的颜色
返 回 值:无
*************************************************************/
void LCD_DispCircle(int xAddr, int yAddr, int radius, unsigned long color) 
{
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	LCD_DispOnePixel(xAddr, yAddr + radius, color);
	LCD_DispOnePixel(xAddr, yAddr - radius, color);
	LCD_DispOnePixel(xAddr + radius, yAddr, color);
	LCD_DispOnePixel(xAddr - radius, yAddr, color);
	while (x < y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		LCD_DispOnePixel(xAddr + x, yAddr + y, color);
		LCD_DispOnePixel(xAddr - x, yAddr + y, color);
		LCD_DispOnePixel(xAddr + x, yAddr - y, color);
		LCD_DispOnePixel(xAddr - x, yAddr - y, color);
		LCD_DispOnePixel(xAddr + y, yAddr + x, color);
		LCD_DispOnePixel(xAddr - y, yAddr + x, color);
		LCD_DispOnePixel(xAddr + y, yAddr - x, color);
		LCD_DispOnePixel(xAddr - y, yAddr - x, color);
	}
}
/*************************************************************
函数名称:LCD_DispIcon
功    能:显示黑白图标(注意:位图的宽度即xSize应取8的整数倍)
参    数:icon:  存储图标数据的数组地址
		 xAddr:  图标起始X坐标
		 yAddr:  图标起始Y坐标
		 xSize:  图标在X坐标上长度
		 ySize:  图标在Y坐标上长度
		 fColor:  字符前景色
		 bColor:  字符背景色 
返 回 值:无
*************************************************************/
void LCD_DispIcon(const unsigned char *icon,unsigned int xAddr,unsigned int yAddr,unsigned int xSize,unsigned int ySize,unsigned long fColor,unsigned long bColor)
{
	const unsigned char *pIcon = icon;
    unsigned int i,j,k;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned long Word;
	
	//define display window
	LCD_DefineDispWindow(xAddr,xAddr + xSize - 1,yAddr,yAddr + ySize - 1);
	
	for(j = 0;j < ySize;j++,pIcon += xSize/8)
	{
		for(i = 0;i < xSize/8;i++)
		{
			PixelRow = *(pIcon + i);
			// loop on each pixel in the row (right to left)
			// Note: we do one pixel each loop
			Mask = 0x80;
			for(k = 0;k < 8;k++)
			{
				// if pixel bit set, use foreground color; else use the background color
				if ((PixelRow & Mask) == 0)
					Word = bColor;
				else
					Word = fColor;
				Mask = Mask >> 1;


			
				Write_dat(Word);
			
			}
		}
	}
}


/*************************************************************
函数名称:LCD_DispImage
功    能:显示图片
参    数:icon:  存储图标数据的数组地址
		 x1:  图片起始X坐标
                 y1:  图片起始Y坐标
		 x2:  图片终止X坐标
		 y2: 图片终止y坐标
		 fColor:  字符前景色
		 bColor:  字符背景色 
返 回 值:无
*************************************************************/
void LCD_DispImage(unsigned char x1,unsigned char y1,unsigned char x2,unsigned int y2,const unsigned char *image)
{
  unsigned char x,y;
  unsigned int i,dd;
  LCD_DefineDispWindow(x1,x2,y1,y2);
  ROM_SysCtlDelay(18500);
  x=x2-x1;
  y=y2-y1;
 for(i=0;i<(x*y);i++)
 {
   dd=(0x00|image[i*2+1])<<8;
   dd=dd|image[i*2];
   Write_dat(dd);
 }
}
#endif /*SSD1289_H_*/
