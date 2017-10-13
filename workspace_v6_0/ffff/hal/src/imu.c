/**
 *    ||          ____  _ __                           
 * +------+      / __ )(_) /_______________ _____  ___ 
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * imu.c - inertial measurement unit
 */
#define DEBUG_MODULE "IMU"

#include <math.h>

#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "debug.h"
#include "configblock.h"
#include "cfassert.h"
#include "imu.h"
#include "i2cdev.h"
#include "mpu6050.h"
#include "hmc5883l.h"
#include "ms5611.h"
#include "ledseq.h"
#include "uart.h"

#define IMU_ENABLE_MAG_HMC5883
#define IMU_ENABLE_PRESSURE_MS5611

#define IMU_GYRO_FS_CFG       MPU6050_GYRO_FS_1000
#define IMU_DEG_PER_LSB_CFG   MPU6050_DEG_PER_LSB_1000
#define IMU_ACCEL_FS_CFG      MPU6050_ACCEL_FS_8
#define IMU_G_PER_LSB_CFG     MPU6050_G_PER_LSB_8
#define IMU_1G_RAW            (int16_t)(1.0 / MPU6050_G_PER_LSB_8)

#define IMU_TEST_DELAY_MS     10
#define IMU_STARTUP_TIME_MS   1000

#define GYRO_NBR_OF_AXES 3
#define GYRO_X_SIGN      (-1)
#define GYRO_Y_SIGN      (-1)
#define GYRO_Z_SIGN      (-1)
#define GYRO_NBR_OF_AXES            3
#define GYRO_MIN_BIAS_TIMEOUT_MS    M2T(1*1000)

#define IMU_NBR_OF_BIAS_SAMPLES  128

#define GYRO_VARIANCE_BASE        4000
#define GYRO_VARIANCE_THRESHOLD_X (GYRO_VARIANCE_BASE)
#define GYRO_VARIANCE_THRESHOLD_Y (GYRO_VARIANCE_BASE)
#define GYRO_VARIANCE_THRESHOLD_Z (GYRO_VARIANCE_BASE)

typedef struct
{
  Axis3i16   bias;
  bool       isBiasValueFound;
  bool       isBufferFilled;
  Axis3i16*  bufHead;
  Axis3i16   buffer[IMU_NBR_OF_BIAS_SAMPLES];
} BiasObj;

BiasObj    gyroBias;
BiasObj    accelBias;
int32_t    varianceSampleTime;
Axis3i16   gyroMpu;
Axis3i16   accelMpu;
Axis3i16   accelLPF;
Axis3i16   accelLPFAligned;
Axis3i16   mag;
Axis3i32   accelStoredFilterValues;
static bool isMagPresent;
static bool isBaroPresent;

// Pre-calculated values for accelerometer alignment
float cosPitch;
float sinPitch;
float cosRoll;
float sinRoll;

static bool imuMPU6050evaluateTest(float low, float high, float value, char* string);
static void imuBiasInit(BiasObj* bias);
static void imuCalculateBiasMean(BiasObj* bias, Axis3i32* meanOut);
static void imuCalculateVarianceAndMean(BiasObj* bias, Axis3i32* varOut, Axis3i32* meanOut);
static bool imuFindBiasValue(BiasObj* bias);
static void imuAddBiasValue(BiasObj* bias, Axis3i16* dVal);
static void imuAccIIRLPFilter(Axis3i16* in, Axis3i16* out,
                              Axis3i32* storedValues, int32_t attenuation);
static void imuAccAlignToGravity(Axis3i16* in, Axis3i16* out);

// TODO: Fix __errno linker error with math lib
int __errno;

static bool isInit;

void imu6Init(void)
{
  if(isInit)
    return;

 isMagPresent = FALSE;
 isBaroPresent = FALSE;

  // Wait for sensors to startup
  while (xTaskGetTickCount() < M2T(IMU_STARTUP_TIME_MS));

  i2cdevInit(I2C1);
  mpu6050Init(I2C1);
  if (mpu6050Test() == TRUE)
  {
    DEBUG_PRINT("MPU6050 I2C connection [OK].\n");
  }
  else
  {
    DEBUG_PRINT("MPU6050 I2C connection [FAIL].\n");
  }

  mpu6050Reset();
  vTaskDelay(M2T(50));
  mpu6050SetSleepEnabled(FALSE);
  mpu6050SetTempSensorEnabled(TRUE);
  mpu6050SetIntEnabled(FALSE);
  mpu6050SetI2CBypassEnabled(TRUE);
  mpu6050SetClockSource(MPU6050_CLOCK_PLL_XGYRO);
  mpu6050SetFullScaleGyroRange(IMU_GYRO_FS_CFG);
  mpu6050SetFullScaleAccelRange(IMU_ACCEL_FS_CFG);
  mpu6050SetRate(15);  // 8000 / (1 + 15) = 500Hz
  mpu6050SetDLPFMode(MPU6050_DLPF_BW_256);

#ifdef IMU_ENABLE_MAG_HMC5883
  hmc5883lInit(I2C1);
  if (hmc5883lTestConnection() == TRUE)
  {
    isMagPresent = TRUE;
    DEBUG_PRINT("HMC5883 I2C connection [OK].\n");
  }
  else
  {
    DEBUG_PRINT("HMC5883 I2C connection [FAIL].\n");
  }
#endif

#ifdef IMU_ENABLE_PRESSURE_MS5611
  if (ms5611Init(I2C1) == TRUE)
  {
    isBaroPresent = TRUE;
    DEBUG_PRINT("MS5611 I2C connection [OK].\n");
  }
  else
  {
    DEBUG_PRINT("MS5611 I2C connection [FAIL].\n");
  }
#endif

  imuBiasInit(&gyroBias);
  imuBiasInit(&accelBias);
  varianceSampleTime = -GYRO_MIN_BIAS_TIMEOUT_MS + 1;

  cosPitch = cos(configblockGetCalibPitch() * M_PI/180);
  sinPitch = sin(configblockGetCalibPitch() * M_PI/180);
  cosRoll = cos(configblockGetCalibRoll() * M_PI/180);
  sinRoll = sin(configblockGetCalibRoll() * M_PI/180);

  isInit = TRUE;
}

bool imu6Test(void)
{
  bool testStatus = TRUE;

  if (!isInit)
  {
    DEBUG_PRINT("Uninitialized");
    testStatus = FALSE;
  }

  if (testStatus)
  {
    Axis3f gyro;
    Axis3f acc;
    Axis3f gyroTst;
    Axis3f accTst;
    Axis3f gyroDiff;
    Axis3f accDiff;
    uint32_t scrap;

    // First values read are zero. Read a couple
    for (scrap = 0; scrap < 20; scrap++)
    {
      mpu6050GetMotion6(&accelMpu.x, &accelMpu.y, &accelMpu.z, &gyroMpu.x, &gyroMpu.y, &gyroMpu.z);
      vTaskDelay(M2T(2));
    }
    gyro.x = gyroMpu.x * IMU_DEG_PER_LSB_CFG;
    gyro.y = gyroMpu.y * IMU_DEG_PER_LSB_CFG;
    gyro.z = gyroMpu.z * IMU_DEG_PER_LSB_CFG;
    acc.x = accelMpu.x * IMU_G_PER_LSB_CFG;
    acc.y = accelMpu.y * IMU_G_PER_LSB_CFG;
    acc.z = accelMpu.z * IMU_G_PER_LSB_CFG;

    mpu6050SetGyroXSelfTest(TRUE);
    mpu6050SetGyroYSelfTest(TRUE);
    mpu6050SetGyroZSelfTest(TRUE);
    mpu6050SetAccelXSelfTest(TRUE);
    mpu6050SetAccelYSelfTest(TRUE);
    mpu6050SetAccelZSelfTest(TRUE);

    vTaskDelay(M2T(IMU_TEST_DELAY_MS));
    mpu6050GetMotion6(&accelMpu.x, &accelMpu.y, &accelMpu.z, &gyroMpu.x, &gyroMpu.y, &gyroMpu.z);
    gyroTst.x = gyroMpu.x * IMU_DEG_PER_LSB_CFG;
    gyroTst.y = gyroMpu.y * IMU_DEG_PER_LSB_CFG;
    gyroTst.z = gyroMpu.z * IMU_DEG_PER_LSB_CFG;
    accTst.x = accelMpu.x * IMU_G_PER_LSB_CFG;
    accTst.y = accelMpu.y * IMU_G_PER_LSB_CFG;
    accTst.z = accelMpu.z * IMU_G_PER_LSB_CFG;

    mpu6050SetGyroXSelfTest(FALSE);
    mpu6050SetGyroYSelfTest(FALSE);
    mpu6050SetGyroZSelfTest(FALSE);
    mpu6050SetAccelXSelfTest(FALSE);
    mpu6050SetAccelYSelfTest(FALSE);
    mpu6050SetAccelZSelfTest(FALSE);

    gyroDiff.x = gyroTst.x - gyro.x;
    gyroDiff.y = gyroTst.y - gyro.y;
    gyroDiff.z = gyroTst.z - gyro.z;
    accDiff.x = accTst.x - acc.x;
    accDiff.y = accTst.y - acc.y;
    accDiff.z = accTst.z - acc.z;

    if (imuMPU6050evaluateTest(MPU6050_ST_GYRO_LOW, MPU6050_ST_GYRO_HIGH, gyroDiff.x, "Gyro X") &&
        imuMPU6050evaluateTest(-MPU6050_ST_GYRO_HIGH, -MPU6050_ST_GYRO_LOW, gyroDiff.y, "Gyro Y") &&
        imuMPU6050evaluateTest(MPU6050_ST_GYRO_LOW, MPU6050_ST_GYRO_HIGH, gyroDiff.z, "Gyro Z") &&
        imuMPU6050evaluateTest(MPU6050_ST_ACCEL_LOW, MPU6050_ST_ACCEL_HIGH, accDiff.x, "Acc X") &&
        imuMPU6050evaluateTest(MPU6050_ST_ACCEL_LOW, MPU6050_ST_ACCEL_HIGH, accDiff.y, "Acc Y") &&
        imuMPU6050evaluateTest(MPU6050_ST_ACCEL_LOW, MPU6050_ST_ACCEL_HIGH, accDiff.z, "Acc Z"))
    {
      DEBUG_PRINT("MPU6050 sensor test [OK].\n");
    }
    else
    {
      testStatus = FALSE;
    }
  }




  return testStatus;
}

bool imuMPU6050evaluateTest(float low, float high, float value, char* string)
{
  if (value < low || value > high)
  {
    DEBUG_PRINT("MPU6050 %s test [FAIL]. low: %0.2f, high: %0.2f, measured: %0.2f\n",
                string, low, high, value);
    return FALSE;
  }
  return TRUE;
}


void imu6Read(Axis3f* gyroOut, Axis3f* accOut)
{
  mpu6050GetMotion6(&accelMpu.x, &accelMpu.y, &accelMpu.z, &gyroMpu.x, &gyroMpu.y, &gyroMpu.z);

  imuAddBiasValue(&gyroBias, &gyroMpu);
  if (!accelBias.isBiasValueFound)
  {
    imuAddBiasValue(&accelBias, &accelMpu);
  }
  if (!gyroBias.isBiasValueFound)
  {
    imuFindBiasValue(&gyroBias);
    if (gyroBias.isBiasValueFound)
    {
      ledseqRun(LED_RED, seq_calibrated);
//      uartPrintf("Gyro bias: %i, %i, %i\n",
//                  gyroBias.bias.x, gyroBias.bias.y, gyroBias.bias.z);
    }
  }

#ifdef IMU_TAKE_ACCEL_BIAS
  if (gyroBias.isBiasValueFound &&
      !accelBias.isBiasValueFound)
  {
    Axis3i32 mean;

    imuCalculateBiasMean(&accelBias, &mean);
    accelBias.bias.x = mean.x;
    accelBias.bias.y = mean.y;
    accelBias.bias.z = mean.z - IMU_1G_RAW;
    accelBias.isBiasValueFound = TRUE;
    //uartPrintf("Accel bias: %i, %i, %i\n",
    //            accelBias.bias.x, accelBias.bias.y, accelBias.bias.z);
  }
#endif


  imuAccIIRLPFilter(&accelMpu, &accelLPF, &accelStoredFilterValues,
                    IMU_ACC_IIR_LPF_ATT_FACTOR);

  imuAccAlignToGravity(&accelLPF, &accelLPFAligned);

  // Re-map outputs
  gyroOut->x = (gyroMpu.x - gyroBias.bias.x) * IMU_DEG_PER_LSB_CFG;
  gyroOut->y = (gyroMpu.y - gyroBias.bias.y) * IMU_DEG_PER_LSB_CFG;
  gyroOut->z = (gyroMpu.z - gyroBias.bias.z) * IMU_DEG_PER_LSB_CFG;
  accOut->x = (accelLPFAligned.x - accelBias.bias.x) * IMU_G_PER_LSB_CFG;
  accOut->y = (accelLPFAligned.y - accelBias.bias.y) * IMU_G_PER_LSB_CFG;
  accOut->z = (accelLPFAligned.z - accelBias.bias.z) * IMU_G_PER_LSB_CFG;

//  uartSendData(sizeof(Axis3f), (uint8_t*)gyroOut);
//  uartSendData(sizeof(Axis3f), (uint8_t*)accOut);

#if 0
  static uint32_t count = 0;
  if (++count >= 19)
  {
    count = 0;
    uartPrintf("%d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                (int32_t)(gyroOut->x * 10),
                (int32_t)(gyroOut->y * 10),
                (int32_t)(gyroOut->z * 10),
                (int32_t)(accOut->x * 1000),
                (int32_t)(accOut->y * 1000),
                (int32_t)(accOut->z * 1000),
                mag.x,
                mag.y,
                mag.z);
  }
#endif
}

bool imu6IsCalibrated(void)
{
  bool status;

  status = gyroBias.isBiasValueFound;
#ifdef IMU_TAKE_ACCEL_BIAS
  status &= accelBias.isBiasValueFound;
#endif

  return status;
}

static void imuBiasInit(BiasObj* bias)
{
  bias->isBufferFilled = FALSE;
  bias->bufHead = bias->buffer;
}

/**
 * Calculates the variance and mean for the bias buffer.
 */
static void imuCalculateVarianceAndMean(BiasObj* bias, Axis3i32* varOut, Axis3i32* meanOut)
{
  uint32_t i;
  int32_t sum[GYRO_NBR_OF_AXES] = {0};
  int64_t sumSq[GYRO_NBR_OF_AXES] = {0};

  for (i = 0; i < IMU_NBR_OF_BIAS_SAMPLES; i++)
  {
    sum[0] += bias->buffer[i].x;
    sum[1] += bias->buffer[i].y;
    sum[2] += bias->buffer[i].z;
    sumSq[0] += bias->buffer[i].x * bias->buffer[i].x;
    sumSq[1] += bias->buffer[i].y * bias->buffer[i].y;
    sumSq[2] += bias->buffer[i].z * bias->buffer[i].z;
  }

  varOut->x = (sumSq[0] - ((int64_t)sum[0] * sum[0]) / IMU_NBR_OF_BIAS_SAMPLES);
  varOut->y = (sumSq[1] - ((int64_t)sum[1] * sum[1]) / IMU_NBR_OF_BIAS_SAMPLES);
  varOut->z = (sumSq[2] - ((int64_t)sum[2] * sum[2]) / IMU_NBR_OF_BIAS_SAMPLES);

  meanOut->x = sum[0] / IMU_NBR_OF_BIAS_SAMPLES;
  meanOut->y = sum[1] / IMU_NBR_OF_BIAS_SAMPLES;
  meanOut->z = sum[2] / IMU_NBR_OF_BIAS_SAMPLES;

  isInit = TRUE;
}

/**
 * Calculates the mean for the bias buffer.
 */
static void __attribute__((used)) imuCalculateBiasMean(BiasObj* bias, Axis3i32* meanOut)
{
  uint32_t i;
  int32_t sum[GYRO_NBR_OF_AXES] = {0};

  for (i = 0; i < IMU_NBR_OF_BIAS_SAMPLES; i++)
  {
    sum[0] += bias->buffer[i].x;
    sum[1] += bias->buffer[i].y;
    sum[2] += bias->buffer[i].z;
  }

  meanOut->x = sum[0] / IMU_NBR_OF_BIAS_SAMPLES;
  meanOut->y = sum[1] / IMU_NBR_OF_BIAS_SAMPLES;
  meanOut->z = sum[2] / IMU_NBR_OF_BIAS_SAMPLES;

}

/**
 * Adds a new value to the variance buffer and if it is full
 * replaces the oldest one. Thus a circular buffer.
 */
static void imuAddBiasValue(BiasObj* bias, Axis3i16* dVal)
{
  bias->bufHead->x = dVal->x;
  bias->bufHead->y = dVal->y;
  bias->bufHead->z = dVal->z;
  bias->bufHead++;

  if (bias->bufHead >= &bias->buffer[IMU_NBR_OF_BIAS_SAMPLES])
  {
    bias->bufHead = bias->buffer;
    bias->isBufferFilled = TRUE;
  }
}

/**
 * Checkes if the variance is blow the predefined thresholds.
 * The bias value should have been added before calling this.
 * @param bias  The bias object
 */
static bool imuFindBiasValue(BiasObj* bias)
{
  bool foundBias = FALSE;

  if (bias->isBufferFilled)
  {
    Axis3i32 variance;
    Axis3i32 mean;

    imuCalculateVarianceAndMean(bias, &variance, &mean);

    //uartSendData(sizeof(variance), (uint8_t*)&variance);
    //uartSendData(sizeof(mean), (uint8_t*)&mean);
    //uartPrintf("%i, %i, %i", variance.x, variance.y, variance.z);
    //uartPrintf("    %i, %i, %i\n", mean.x, mean.y, mean.z);

    if (variance.x < GYRO_VARIANCE_THRESHOLD_X &&
        variance.y < GYRO_VARIANCE_THRESHOLD_Y &&
        variance.z < GYRO_VARIANCE_THRESHOLD_Z &&
        (varianceSampleTime + GYRO_MIN_BIAS_TIMEOUT_MS < xTaskGetTickCount()))
    {
      varianceSampleTime = xTaskGetTickCount();
      bias->bias.x = mean.x;
      bias->bias.y = mean.y;
      bias->bias.z = mean.z;
      foundBias = TRUE;
      bias->isBiasValueFound = TRUE;
    }
  }

  return foundBias;
}

static void imuAccIIRLPFilter(Axis3i16* in, Axis3i16* out, Axis3i32* storedValues, int32_t attenuation)
{
  out->x = iirLPFilterSingle(in->x, attenuation, &storedValues->x);
  out->y = iirLPFilterSingle(in->y, attenuation, &storedValues->y);
  out->z = iirLPFilterSingle(in->z, attenuation, &storedValues->z);
}


/**
 * Compensate for a miss-aligned accelerometer. It uses the trim
 * data gathered from the UI and written in the config-block to
 * rotate the accelerometer to be aligned with gravity.
 */
static void imuAccAlignToGravity(Axis3i16* in, Axis3i16* out)
{
  Axis3i16 rx;
  Axis3i16 ry;

  // Rotate around x-axis
  rx.x = in->x;
  rx.y = in->y * cosRoll - in->z * sinRoll;
  rx.z = in->y * sinRoll + in->z * cosRoll;

  // Rotate around y-axis
  ry.x = rx.x * cosPitch - rx.z * sinPitch;
  ry.y = rx.y;
  ry.z = -rx.x * sinPitch + rx.z * cosPitch;

  out->x = ry.x;
  out->y = ry.y;
  out->z = ry.z;
}
