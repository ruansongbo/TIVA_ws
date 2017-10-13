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

//****************************************
// 定义MPU6050内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)
#define	mpu_SlaveAddress 0x68	//IIC写入时的地址字节数据，+1为读取

#define hmc_SlaveAddress 0x1E   //定义器件在IIC总线中的从地址

#define Gx_offset -1.711
#define Gy_offset 7.5345
#define Gz_offset -10.6085
#define Ax_offset 456.47
#define Ay_offset -484.927
#define Az_offset -643.502

#define PI 3.141529

uint8_t hmc_data[6];

//----------------------------------------------------------------------------------------------------
// Definitions

#define Kp 2.0f                        // proportional gain governs rate of convergence to accelerometer/magnetometer
#define Ki 0.005f                // integral gain governs rate of convergence of gyroscope biases
#define halfT 0.05f                // half the sample period

//---------------------------------------------------------------------------------------------------
// Variable definitions

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;        // quaternion elements representing the estimated orientation
float exInt = 0, eyInt = 0, ezInt = 0;        // scaled integral error
float t11,t12,t13,t21,t22,t23,t31,t32,t33;

//====================================================================================================
// Function
//====================================================================================================

void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az) {
        float norm;
        float vx, vy, vz;
        float ex, ey, ez;

        // normalise the measurements
        norm = sqrt(ax*ax + ay*ay + az*az);
        ax = ax / norm;
        ay = ay / norm;
        az = az / norm;


        // estimated direction of gravity
        vx = 2*(q1*q3 - q0*q2);
        vy = 2*(q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;


        // error is sum of cross product between reference direction of field and direction measured by sensor
        ex = (ay*vz - az*vy);
        ey = (az*vx - ax*vz);
        ez = (ax*vy - ay*vx);



        // integral error scaled integral gain
        exInt = exInt + ex*Ki;
        eyInt = eyInt + ey*Ki;
        ezInt = ezInt + ez*Ki;

        // adjusted gyroscope measurements
        gx = gx + Kp*ex + exInt;
        gy = gy + Kp*ey + eyInt;
        gz = gz + Kp*ez + ezInt;





        // integrate quaternion rate and normalise
        q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
        q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
        q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
        q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;



        // normalise quaternion
        norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
        q0 = q0 / norm;
        q1 = q1 / norm;
        q2 = q2 / norm;
        q3 = q3 / norm;
}

//====================================================================================================
// END OF CODE
//====================================================================================================

typedef struct
{
	int16_t X;
	int16_t Y;
	int16_t Z;
}Magnet;

typedef struct
{
	struct
	{
		int16_t X;
		int16_t Y;
		int16_t Z;
	}ACCEL;
	struct
	{
		int16_t X;
		int16_t Y;
		int16_t Z;
	}GYRO;
}DATA;

typedef struct
{
	float roll;
	float pitch;
	float yaw;
}Euler;
Euler euler;
float Ax,Ay,Az;//单位 g(9.8m/s^2)
float Gx,Gy,Gz;//单位 °/s
float Angel_accX,Angel_accY,Angel_accZ;//存储加速度计算出的角度
double Angle;
uint16_t Acr;

uint8_t data[6];

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



void InitMPU6050(void)
{
	writeI2C1(mpu_SlaveAddress,PWR_MGMT_1, 0x00);	//解除休眠状态
	SysCtlDelay(SysCtlClockGet()/5);
	writeI2C1(mpu_SlaveAddress,PWR_MGMT_1, 0x03);
	writeI2C1(mpu_SlaveAddress,SMPLRT_DIV, 0x13);
	writeI2C1(mpu_SlaveAddress,CONFIG, 0x06);
	writeI2C1(mpu_SlaveAddress,GYRO_CONFIG, 0x18);
	writeI2C1(mpu_SlaveAddress,ACCEL_CONFIG, 0x01);
	writeI2C1(mpu_SlaveAddress,0x37, 0x02);
	writeI2C1(mpu_SlaveAddress,0x6A, 0x00);

}

void HMC5883_Init(void)
{
    writeI2C1(hmc_SlaveAddress,0x02, 0x00);
}
void
InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //


    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

void systeminit(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
}
DATA mpu;
void mpu_getdata(void)
{
	mpu.ACCEL.X = ((int16_t)readI2C1(mpu_SlaveAddress, ACCEL_XOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, ACCEL_XOUT_L);
	mpu.ACCEL.Y = ((int16_t)readI2C1(mpu_SlaveAddress, ACCEL_YOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, ACCEL_YOUT_L);
	mpu.ACCEL.Z = ((int16_t)readI2C1(mpu_SlaveAddress, ACCEL_ZOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, ACCEL_ZOUT_L);
	mpu.GYRO.X = ((int16_t)readI2C1(mpu_SlaveAddress, GYRO_XOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, GYRO_XOUT_L);
	mpu.GYRO.Y = ((int16_t)readI2C1(mpu_SlaveAddress, GYRO_YOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, GYRO_YOUT_L);
	mpu.GYRO.Z = ((int16_t)readI2C1(mpu_SlaveAddress, GYRO_ZOUT_H)<<8)+
			readI2C1(mpu_SlaveAddress, GYRO_ZOUT_L);
	Ax=(float)(mpu.ACCEL.X-Ax_offset)/16384.00;
	Ay=(float)(mpu.ACCEL.Y-Ay_offset)/16384.00;
	Az=(float)(mpu.ACCEL.Z-Az_offset)/16384.00;
	Angel_accX=atan(Ax/sqrt(Az*Az+Ay*Ay))*180/3.14;
	Angel_accY=atan(Ay/sqrt(Ax*Ax+Az*Az))*180/3.14;
	Angel_accZ=atan(Az/sqrt(Ax*Ax+Ay*Ay))*180/3.14;
	Gx=(float)(mpu.GYRO.X-Gx_offset)/131.00;
	Gy=(float)(mpu.GYRO.Y-Gy_offset)/131.00;
	Gz=(float)(mpu.GYRO.Z-Gz_offset)/131.00;
	SysCtlDelay(SysCtlClockGet()/10);
}
Magnet magnet;
void hmc_getdata(void)
{
    IICreadBytes(hmc_SlaveAddress,0x03,6,hmc_data);
    magnet.X = (hmc_data[0]<<8)|hmc_data[1];
    magnet.Z = (hmc_data[2]<<8)|hmc_data[3];
    magnet.Y = (hmc_data[4]<<8)|hmc_data[5];
    Angle= atan2((double)magnet.Y,(double)magnet.X)*(180/3.14159265)+180;
    Acr=(uint16_t)Angle;
}
uint8_t dd;
void main(void)
{
	systeminit();
	InitConsole();
	InitI2C1();
	UARTprintf("sdsdfsfsdfasdfsd");
	dd = readI2C1(0x68, 0X75);
	UARTprintf("%d\n",dd);
	//InitMPU6050();
	//HMC5883_Init();
	while(1)
	{
		//mpu_getdata();
		//hmc_getdata();
		//IMUupdate(Gx, Gy, Gz, Ax, Ay, Az);
		//更新方向余弦矩阵
		/*t11=q0*q0+q1*q1-q2*q2-q3*q3;
		t12=2.0*(q1*q2+q0*q3);
		t13=2.0*(q1*q3-q0*q2);
		t21=2.0*(q1*q2-q0*q3);
		t22=q0*q0-q1*q1+q2*q2-q3*q3;
		t23=2.0*(q2*q3+q0*q1);
		t31=2.0*(q1*q3+q0*q2);
		t32=2.0*(q2*q3-q0*q1);
		t33=q0*q0-q1*q1-q2*q2+q3*q3;
		//求出欧拉角
		euler.roll = atan2(t23,t33)*180/PI;
		euler.pitch = -asin(t13)*180/PI;
		euler.yaw = atan2(t12,t11)*180/PI;
		euler.roll = (int16_t)euler.roll;
		euler.pitch = (int16_t)euler.pitch;
		euler.yaw = (int16_t)euler.yaw;
		//Gx=(uint16_t)Gx*1000;
		//UARTprintf("Gx = %d\n",Gx);
		//UARTprintf("Gy = %d\n",Gy);
		//UARTprintf("Gz = %d\n",Gz);
		//UARTprintf("Ax = %d\n",Ax);
		//UARTprintf("Ay = %d\n",Ay);
		//UARTprintf("Az = %d\n",Az);
		UARTprintf("%d,",(int)euler.roll);
		UARTprintf("%d,",(int)euler.pitch);
		UARTprintf("%d\n",(int)euler.yaw);*/
	}
}
