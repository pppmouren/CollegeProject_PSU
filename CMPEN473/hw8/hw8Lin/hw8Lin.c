/**************************************************
* File:  hw8Lin.c 
* Homework 8, manual car driving, self locate orientation function, and IMU data collecting and analyzing
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 8, we will use command to control car driving
* Three Modes: 'm0' for 5s data collecting by MPU6250 IU chip, 'm1' mode for manual driving, 'm2' mode for self location orientation
* 				(data will be collected all the time during 'm1' and 'm2' mode)
* 1. "m0" mode:
 (1) 'q': quit the program
 (2) 'm1': change to m1 mode
 (3) 'm2': change to m2 mode
 (4) 'p': Print data
 * note: m0 mode will not chnage to mode to m1 or m2 until it finish the 5s data collecting
 
* 2. "m1" mode:
 (1) 's': Stop, and stop IMU data collecting
 (2) 'w': Forward, and also start IMU data collecting
 (3) 'x': Backward
 (4) 'i': Faster, 5% PWM power increase for each 'i' key hit
 (5) 'j': Slower, 5% PWM power decrease for each 'j' key hit
 (6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition
 (7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition
 (8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key
 (9) 'm2': mode change to m2
 (10) 'm0': mode change to m0
 (11) 'p': print data
 (12) 'n': Display total distance and average speed

* 3. "m2" mode:
 (1) 's': Stop to pause 
 (2) 'w': to start the re-orient its direction
 (3) 'q': to quit the program
 (4) 'm1': mode change to m1
 (5) 'm0': mode change to m0
 (6) 'p': print data
 (7) 'n': Display total distance and average speed
 (8) 'r': Reset and start collecting/recording the IMU data
*Program must end with ‘q’ command and when the program ends.
* 
* GPIO Usage:
* 1. GPIO 12,12 used as PWM channel 1,2 for wheel speed
* 2. GPIO 22,23 used as output for right wheel direction, 00 for stop, 01 for forward, 10 or backward
* 3. GPIO 5,6 usd as output for left wheel direction, 00 for stop, 01 for forward, 10 for backward
* 4. GPIO 24, 25 used as input port to receive the information from the IR sensors. 24 for left sensor, 25 for right
* 		(when the input pin is low, indicator light is on, obsticle detected, NO BLACK LINE!)
* 		(when the input pin is high, indicator light is off, no obsticle detected, BLACK LINE!)
* 5. GPIO 2,3 used as SCL and SDA for data transformation between the MPU6050 chip and raspberry pi
* 
* QUICK NOTE:
* 1. always display on a terminal window the prompt ‘Hw8>' and display
*	 the user key command input as they enter (echo print)
* 2. m1 mode:
* 		2.1. For the ' w ' and ' x ' commands, be sure to have your car stop for 0.01 second 
* 			 (ramp down, stop,reverse ramp up) if the car is currently in opposite motion. That
* 			 is, avoid moving the car to go backward immediately if it is currently going forward
* 		2.2. One should consider the fact that PWM level less than 35% may not start the motor 
* 			 (varies from one motor to another motor). Moreover, the available speed range may 
*			 be 30% to 100%. If the left and right motor characteristics are different, write your 
* 			 program to compensate - left and right motor speed balancing (may use look-up tables).
* 		2.3. Collecting 3-axis accelerometer and 3-axis gyroscope data suring driving.
* 3. m2 mode:
* 		3.1. Hit 'r' key to RESET and start collecting/recording the IMU data
* 		3.2. The RoboCar is to remember the current position/direction. Then have the RoboCar moved 
*            by a person to different orientation
* 		3.3. When a person hits 'w', the RoboCar moves to re-orient its direction
* 		3.4. Collecting 3-axis accelerometer and 3-axis gyroscope data during driving.
* 4. m0 mode:
* 		4.1. Automatically collecting data for 5s, 100 times per second. Totally 600 data.
* 		4.2. Each IMU data consists of 3-axis accelerometer sensor readings and 3-axis gyroscope sensor 
* 			 readings, total 6 numbers. The data recording rate must be 100 IMU data recording per second
*			 (ie. 600 numbers per second). Then save the data into “hw7m0data.txt” file
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*
* RPi4 GPIO pin configuration - OUTPUT and INPUT
*   OUTPUT: Logic high => 3.3V at the pin, up to 10mA current source
*           Logic low  => 0.0V at the pin, up to 10mA current sink
***************************************************/

// header files - at /usr/include and ../include and .
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <math.h>
#include "../include/import_registers.h"
#include "../include/cm.h"
#include "../include/gpio.h"
#include "../include/uart.h"
#include "../include/spi.h"
#include "../include/bsc.h"
#include "../include/pwm.h"
#include "../include/enable_pwm_clock.h"
#include "../include/io_peripherals.h"
#include "../include/wait_period.h"
#include "../include/FIFO.h"
#include "../include/MPU6050.h"
#include "../include/MPU9250.h"
#include "../include/wait_key.h"   
#include "keypress.h"
#include "pwmsetup.h"


#define PWM_RANGE 100
#define FIFO_LENGTH 2048
#define stopTurnTime 150*1000
#define onMovingTurnTime 240*1000
#define m2TurnTime 5*1000
#define m0Mode 0
#define m1Mode 1
#define m2Mode 2
#define APB_CLOCK 250000000
#define ROUND_DIVISION(x,y) (((x) + (y)/2)/(y))
#define Acce_factor 0.50
#define AcceZ_factor 10.00
#define Gyro_factor 25.00
#define Time_Interval_S 0.01
#define Uint_Degree_Turn_Time 4.5*1000

uint32_t rightPWMLevel = 50;    /* right wheel PWM as duty cycle, set default to 50 */
uint32_t leftPWMLevel = 50;     /* lefwt wheel PWM as duty cycle set default to 50*/
int leftMode[2] = {0,0};
int rightMode[2] = {0,0};
int currMode = m1Mode;
char commandBuffer[2] = {'N','N'};
int accelBufferX[60000];
int accelBufferY[60000];
int accelBufferZ[60000];
int gyroBufferX[60000];
int gyroBufferY[60000];
int gyroBufferZ[60000];
int printIndex = 0;
float preAccelX = 0.0;
float preAccelY = 0.0;
//float preAccelZ = 0.0;
//float totalSpeed = 0.0;
float totalDistance = 0.0;
float preSpeedX = 0.0;
float preSpeedY = 0.0;
int m1m2DataCounter = 0;
//int printMapFlag = 0;
float totalAngel = 0.0;

//define two fifos, one for left, one for right
FIFO_TYPE(char, FIFO_LENGTH, MYFIFO);
struct MYFIFO left_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
struct MYFIFO right_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
struct MYFIFO MPU9250_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};

union uint16_to_2uint8
{
  struct uint16_to_2uint8_field
  {
    uint8_t   L;  /* Little Endian byte order means that the least significant byte goes in the lowest address */
    uint8_t   H;
  }         field;
  uint16_t  unsigned_value;
  int16_t   signed_value;
};

struct calibration_data
{
  float scale;
  float offset_x;
  float offset_y;
  float offset_z;
};

struct thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
  volatile struct pwm_register *pwm; // PWM port registers
};

struct MPU9250_param
{
	struct calibration_data calibration_accelerometer;
	struct calibration_data calibration_gyroscope;
	volatile struct bsc_register *bsc;
	FILE *fd_m0;
	FILE *fd_m1;
	FILE *fd_m2;
};


// READ REG
void read_MPU9250_registers(                          /* read a register */
    uint8_t                         I2C_address,      /* the address of the I2C device to talk to */
    MPU9250_REGISTER                register_address, /* the address to read from */
    uint8_t *                       read_data,        /* the data read from the SPI device */
    size_t                          data_length,      /* the length of data to send/receive */
    volatile struct bsc_register *  bsc )             /* the BSC address */
{
  bsc->S.field.DONE    = 1;
  bsc->A.field.ADDR    = I2C_address;
  bsc->C.field.READ    = 0;
  bsc->DLEN.field.DLEN = 1;
  bsc->FIFO.value      = register_address;
  bsc->C.field.ST      = 1;
  while (bsc->S.field.DONE == 0)
  {
    usleep( 100 );
  }
  bsc->S.field.DONE    = 1;
  bsc->A.field.ADDR    = I2C_address;
  bsc->C.field.READ    = 1;
  bsc->DLEN.field.DLEN = data_length;
  bsc->C.field.ST      = 1;
  while (bsc->S.field.DONE == 0)
  {
    usleep( 100 );
  }

  while (data_length > 0)
  {
    *read_data = bsc->FIFO.field.DATA;

    read_data++;
    data_length--;
  }

  return;
}



// READ REG
union MPU9250_transaction_field_data read_MPU9250_register( /* read a register, returning the read value */
    uint8_t                         I2C_address,            /* the address of the I2C device to talk to */
    MPU9250_REGISTER                register_address,       /* the address to read from */
    volatile struct bsc_register *  bsc )                   /* the BSC address */
{
  union MPU9250_transaction transaction;

  read_MPU9250_registers( I2C_address, register_address, &(transaction.value[1]), 1, bsc );

  return transaction.field.data;
}



// WRITE REG
void write_MPU9250_register(                                /* write a register */
    uint8_t                               I2C_address,      /* the address of the I2C device to talk to */
    MPU9250_REGISTER                      register_address, /* the address to read from */
    union MPU9250_transaction_field_data  value,            /* the value to write */
    volatile struct bsc_register *        bsc )             /* the BSC address */
{
  union MPU9250_transaction transaction;

  transaction.field.data = value;
  bsc->S.field.DONE    = 1;
  bsc->A.field.ADDR    = I2C_address;
  bsc->C.field.READ    = 0;
  bsc->DLEN.field.DLEN = 2;
  bsc->FIFO.value      = register_address;
  bsc->FIFO.value      = transaction.value[1];
  bsc->C.field.ST      = 1;
  while (bsc->S.field.DONE == 0)
  {
    usleep( 100 );
  }

  return;
}



// CAL AG
void calibrate_accelerometer_and_gyroscope(
    struct calibration_data *     calibration_accelerometer,
    struct calibration_data *     calibration_gyroscope,
    volatile struct bsc_register *bsc )
{
  union MPU9250_transaction_field_data  transaction;
  uint8_t                               data_block_fifo_count[2];
  union uint16_to_2uint8                reconstructor;
  uint16_t                              ii;
  uint16_t                              packet_count;
  int32_t                               gyro_bias_x;
  int32_t                               gyro_bias_y;
  int32_t                               gyro_bias_z;
  int32_t                               accel_bias_x;
  int32_t                               accel_bias_y;
  int32_t                               accel_bias_z;
  uint8_t                               data_block_fifo_packet[12];
  union uint16_to_2uint8                reconstructor_accel_x;
  union uint16_to_2uint8                reconstructor_accel_y;
  union uint16_to_2uint8                reconstructor_accel_z;
  union uint16_to_2uint8                reconstructor_gyro_x;
  union uint16_to_2uint8                reconstructor_gyro_y;
  union uint16_to_2uint8                reconstructor_gyro_z;

  // reset device
  transaction.PWR_MGMT_1.CLKSEL       = 0;
  transaction.PWR_MGMT_1.PD_PTAT      = 0;
  transaction.PWR_MGMT_1.GYRO_STANDBY = 0;
  transaction.PWR_MGMT_1.CYCLE        = 0;
  transaction.PWR_MGMT_1.SLEEP        = 0;
  transaction.PWR_MGMT_1.H_RESET      = 1;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );
  usleep( 100000 );

  // get stable time source; auto select clock source to be PLL gyroscope reference if ready
  // else use the internal oscillator
  transaction.PWR_MGMT_1.CLKSEL       = 1;
  transaction.PWR_MGMT_1.PD_PTAT      = 0;
  transaction.PWR_MGMT_1.GYRO_STANDBY = 0;
  transaction.PWR_MGMT_1.CYCLE        = 0;
  transaction.PWR_MGMT_1.SLEEP        = 0;
  transaction.PWR_MGMT_1.H_RESET      = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );
  transaction.PWR_MGMT_2.DIS_ZG   = 0;
  transaction.PWR_MGMT_2.DIS_YG   = 0;
  transaction.PWR_MGMT_2.DIS_XG   = 0;
  transaction.PWR_MGMT_2.DIS_ZA   = 0;
  transaction.PWR_MGMT_2.DIS_YA   = 0;
  transaction.PWR_MGMT_2.DIS_XA   = 0;
  transaction.PWR_MGMT_2.reserved = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_2, transaction, bsc );
  usleep( 200000 );

  // configure device for bias calculation
  transaction.INT_ENABLE.RAW_RDY_EN    = 0; // disable all interrupts
  transaction.INT_ENABLE.reserved0     = 0;
  transaction.INT_ENABLE.FSYNC_INT_EN  = 0;
  transaction.INT_ENABLE.FIFO_OFLOW_EN = 0;
  transaction.INT_ENABLE.reserved1     = 0;
  transaction.INT_ENABLE.WOM_EN        = 0;
  transaction.INT_ENABLE.reserved2     = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_INT_ENABLE, transaction, bsc );
  transaction.FIFO_EN.SLV0         = 0; // disable FIFO
  transaction.FIFO_EN.SLV1         = 0;
  transaction.FIFO_EN.SLV2         = 0;
  transaction.FIFO_EN.ACCEL        = 0;
  transaction.FIFO_EN.GYRO_ZO_UT   = 0;
  transaction.FIFO_EN.GYRO_YO_UT   = 0;
  transaction.FIFO_EN.GYRO_XO_UT   = 0;
  transaction.FIFO_EN.TEMP_FIFO_EN = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_FIFO_EN, transaction, bsc );
  transaction.PWR_MGMT_1.CLKSEL       = 0;  // turn on internal clock source
  transaction.PWR_MGMT_1.PD_PTAT      = 0;
  transaction.PWR_MGMT_1.GYRO_STANDBY = 0;
  transaction.PWR_MGMT_1.CYCLE        = 0;
  transaction.PWR_MGMT_1.SLEEP        = 0;
  transaction.PWR_MGMT_1.H_RESET      = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );
  transaction.I2C_MST_CTRL.I2C_MST_CLK   = 0; // disable I2C master
  transaction.I2C_MST_CTRL.I2C_MST_P_NSR = 0;
  transaction.I2C_MST_CTRL.SLV_3_FIFO_EN = 0;
  transaction.I2C_MST_CTRL.WAIT_FOR_ES   = 0;
  transaction.I2C_MST_CTRL.MULT_MST_EN   = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_I2C_MST_CTRL, transaction, bsc );
  transaction.USER_CTRL.SIG_COND_RST = 0; // disable FIFO and I2C master modes
  transaction.USER_CTRL.I2C_MST_RST  = 0;
  transaction.USER_CTRL.FIFO_RST     = 0;
  transaction.USER_CTRL.reserved0    = 0;
  transaction.USER_CTRL.I2C_IF_DIS   = 0;
  transaction.USER_CTRL.I2C_MST_EN   = 0;
  transaction.USER_CTRL.FIFO_EN      = 0;
  transaction.USER_CTRL.reserved1    = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_USER_CTRL, transaction, bsc );
  transaction.USER_CTRL.SIG_COND_RST = 0; // reset FIFO and DMP
  transaction.USER_CTRL.I2C_MST_RST  = 0;
  transaction.USER_CTRL.FIFO_RST     = 1;
  transaction.USER_CTRL.reserved0    = 0;
  transaction.USER_CTRL.I2C_IF_DIS   = 0;
  transaction.USER_CTRL.I2C_MST_EN   = 0;
  transaction.USER_CTRL.FIFO_EN      = 0;
  transaction.USER_CTRL.reserved1    = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_USER_CTRL, transaction, bsc );
  usleep( 15000 );

  // configure MPU9250 gyro and accelerometer for bias calculation
  transaction.CONFIG.DLPF_CFG     = 1;  // set low-pass filter to 188Hz
  transaction.CONFIG.EXT_SYNC_SET = 0;
  transaction.CONFIG.FIFO_MODE    = 0;
  transaction.CONFIG.reserved     = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_CONFIG, transaction, bsc );
  transaction.SMPLRT_DIV.SMPLRT_DIV = 0;  // set sample rate to 1kHz
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_SMPLRT_DIV, transaction, bsc );
  transaction.GYRO_CONFIG.FCHOICE_B   = 0; // set gyro full-scale to 250dps, maximum sensitivity
  transaction.GYRO_CONFIG.reserved    = 0;
  transaction.GYRO_CONFIG.GYRO_FS_SEL = 0;
  transaction.GYRO_CONFIG.ZGYRO_Cten  = 0;
  transaction.GYRO_CONFIG.YGYRO_Cten  = 0;
  transaction.GYRO_CONFIG.XGYRO_Cten  = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_GYRO_CONFIG, transaction, bsc );
  transaction.ACCEL_CONFIG.reserved     = 0; // set accelerometer full-scale to 2g, maximum sensitivity
  transaction.ACCEL_CONFIG.ACCEL_FS_SEL = 0;
  transaction.ACCEL_CONFIG.az_st_en     = 0;
  transaction.ACCEL_CONFIG.ay_st_en     = 0;
  transaction.ACCEL_CONFIG.ax_st_en     = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_ACCEL_CONFIG, transaction, bsc );

  calibration_accelerometer->scale = 2.0/32768.0;  // measurement scale/signed numeric range
  calibration_accelerometer->offset_x = 0;
  calibration_accelerometer->offset_y = 0;
  calibration_accelerometer->offset_z = 0;

  calibration_gyroscope->scale = 250.0/32768.0;
  calibration_gyroscope->offset_x = 0;
  calibration_gyroscope->offset_y = 0;
  calibration_gyroscope->offset_z = 0;

  // configure FIFO to capture accelerometer and gyro data for bias calculation
  transaction.USER_CTRL.SIG_COND_RST = 0; // enable FIFO
  transaction.USER_CTRL.I2C_MST_RST  = 0;
  transaction.USER_CTRL.FIFO_RST     = 0;
  transaction.USER_CTRL.reserved0    = 0;
  transaction.USER_CTRL.I2C_IF_DIS   = 0;
  transaction.USER_CTRL.I2C_MST_EN   = 0;
  transaction.USER_CTRL.FIFO_EN      = 1;
  transaction.USER_CTRL.reserved1    = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_USER_CTRL, transaction, bsc );
  transaction.FIFO_EN.SLV0         = 0; // enable gyro and accelerometer sensors for FIFO (max size 512 bytes in MPU9250)
  transaction.FIFO_EN.SLV1         = 0;
  transaction.FIFO_EN.SLV2         = 0;
  transaction.FIFO_EN.ACCEL        = 1;
  transaction.FIFO_EN.GYRO_ZO_UT   = 1;
  transaction.FIFO_EN.GYRO_YO_UT   = 1;
  transaction.FIFO_EN.GYRO_XO_UT   = 1;
  transaction.FIFO_EN.TEMP_FIFO_EN = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_FIFO_EN, transaction, bsc );
  usleep( 40000 );  // accumulate 40 samples in 40 milliseconds = 480 bytes

  // at end of sample accumulation, turn off FIFO sensor read
  transaction.FIFO_EN.SLV0         = 0; // disable gyro and accelerometer sensors for FIFO
  transaction.FIFO_EN.SLV1         = 0;
  transaction.FIFO_EN.SLV2         = 0;
  transaction.FIFO_EN.ACCEL        = 0;
  transaction.FIFO_EN.GYRO_ZO_UT   = 0;
  transaction.FIFO_EN.GYRO_YO_UT   = 0;
  transaction.FIFO_EN.GYRO_XO_UT   = 0;
  transaction.FIFO_EN.TEMP_FIFO_EN = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_FIFO_EN, transaction, bsc );
  read_MPU9250_registers( MPU9250_ADDRESS, MPU9250_REGISTER_FIFO_COUNTH, data_block_fifo_count, sizeof(data_block_fifo_count), bsc ); // read FIFO sample count
  reconstructor.field.H = data_block_fifo_count[0];
  reconstructor.field.L = data_block_fifo_count[1];
  packet_count = reconstructor.unsigned_value / 12; // how many sets of full gyro and accelerometer data for averaging

  accel_bias_x = 0;
  accel_bias_y = 0;
  accel_bias_z = 0;
  gyro_bias_x = 0;
  gyro_bias_y = 0;
  gyro_bias_z = 0;
  for (ii = 0; ii < packet_count; ii++)
  {
    read_MPU9250_registers( MPU9250_ADDRESS, MPU9250_REGISTER_FIFO_R_W, data_block_fifo_packet, sizeof(data_block_fifo_packet), bsc ); // read data for averaging

    reconstructor_accel_x.field.H = data_block_fifo_packet[0];
    reconstructor_accel_x.field.L = data_block_fifo_packet[1];
    reconstructor_accel_y.field.H = data_block_fifo_packet[2];
    reconstructor_accel_y.field.L = data_block_fifo_packet[3];
    reconstructor_accel_z.field.H = data_block_fifo_packet[4];
    reconstructor_accel_z.field.L = data_block_fifo_packet[5];
    reconstructor_gyro_x.field.H  = data_block_fifo_packet[6];
    reconstructor_gyro_x.field.L  = data_block_fifo_packet[7];
    reconstructor_gyro_y.field.H  = data_block_fifo_packet[8];
    reconstructor_gyro_y.field.L  = data_block_fifo_packet[9];
    reconstructor_gyro_z.field.H  = data_block_fifo_packet[10];
    reconstructor_gyro_z.field.L  = data_block_fifo_packet[11];

    accel_bias_x += reconstructor_accel_x.signed_value; // sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias_y += reconstructor_accel_y.signed_value;
    accel_bias_z += reconstructor_accel_z.signed_value;
    gyro_bias_x  += reconstructor_gyro_x.signed_value;
    gyro_bias_y  += reconstructor_gyro_y.signed_value;
    gyro_bias_z  += reconstructor_gyro_z.signed_value;
  }
  accel_bias_x /= (int32_t)packet_count;
  accel_bias_y /= (int32_t)packet_count;
  accel_bias_z /= (int32_t)packet_count;
  gyro_bias_x /= (int32_t)packet_count;
  gyro_bias_y /= (int32_t)packet_count;
  gyro_bias_z /= (int32_t)packet_count;

  if (accel_bias_z > 0) // remove gravity from the z-axis accelerometer bias calculation
  {
    accel_bias_z -= (int32_t)(1.0/calibration_accelerometer->scale);
  }
  else
  {
    accel_bias_z += (int32_t)(1.0/calibration_accelerometer->scale);
  }

  // the code that this is based off of tried to push the bias calculation values to hardware correction registers
  // these registers do not appear to be functioning, so rely on software offset correction

  // output scaled gyro biases
  calibration_gyroscope->offset_x = ((float)gyro_bias_x)*calibration_gyroscope->scale;
  calibration_gyroscope->offset_y = ((float)gyro_bias_y)*calibration_gyroscope->scale;
  calibration_gyroscope->offset_z = ((float)gyro_bias_z)*calibration_gyroscope->scale;

  // output scaled accelerometer biases
  calibration_accelerometer->offset_x = ((float)accel_bias_x)*calibration_accelerometer->scale;
  calibration_accelerometer->offset_y = ((float)accel_bias_y)*calibration_accelerometer->scale;
  calibration_accelerometer->offset_z = ((float)accel_bias_z)*calibration_accelerometer->scale;

  return;
}



// INIT AG
void initialize_accelerometer_and_gyroscope(
    struct calibration_data *     calibration_accelerometer,
    struct calibration_data *     calibration_gyroscope,
    volatile struct bsc_register *bsc )
{
  union MPU9250_transaction_field_data  transaction;

  /* print WHO_AM_I */
  printf( "accel WHOAMI (0x71) = 0x%2.2X\n",
      read_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_WHO_AM_I, bsc ).WHO_AM_I.WHOAMI );

  // based off https://github.com/brianc118/MPU9250/blob/master/MPU9250.cpp

  calibrate_accelerometer_and_gyroscope( calibration_accelerometer, calibration_gyroscope, bsc );

  // reset MPU9205
  transaction.PWR_MGMT_1.CLKSEL        = 0;
  transaction.PWR_MGMT_1.PD_PTAT       = 0;
  transaction.PWR_MGMT_1.GYRO_STANDBY  = 0;
  transaction.PWR_MGMT_1.CYCLE         = 0;
  transaction.PWR_MGMT_1.SLEEP         = 0;
  transaction.PWR_MGMT_1.H_RESET       = 1;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );
  usleep( 1000 ); // wait for all registers to reset

  // clock source
  transaction.PWR_MGMT_1.CLKSEL       = 1;
  transaction.PWR_MGMT_1.PD_PTAT      = 0;
  transaction.PWR_MGMT_1.GYRO_STANDBY = 0;
  transaction.PWR_MGMT_1.CYCLE        = 0;
  transaction.PWR_MGMT_1.SLEEP        = 0;
  transaction.PWR_MGMT_1.H_RESET      = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );

  // enable acc & gyro
  transaction.PWR_MGMT_2.DIS_ZG   = 0;
  transaction.PWR_MGMT_2.DIS_YG   = 0;
  transaction.PWR_MGMT_2.DIS_XG   = 0;
  transaction.PWR_MGMT_2.DIS_ZA   = 0;
  transaction.PWR_MGMT_2.DIS_YA   = 0;
  transaction.PWR_MGMT_2.DIS_XA   = 0;
  transaction.PWR_MGMT_2.reserved = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_PWR_MGMT_1, transaction, bsc );

  // use DLPF set gyro bandwidth 184Hz, temperature bandwidth 188Hz
  transaction.CONFIG.DLPF_CFG     = 1;
  transaction.CONFIG.EXT_SYNC_SET = 0;
  transaction.CONFIG.FIFO_MODE    = 0;
  transaction.CONFIG.reserved     = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_CONFIG, transaction, bsc );

  // +-250dps
  transaction.GYRO_CONFIG.FCHOICE_B   = 0;
  transaction.GYRO_CONFIG.reserved    = 0;
  transaction.GYRO_CONFIG.GYRO_FS_SEL = 0;
  transaction.GYRO_CONFIG.ZGYRO_Cten  = 0;
  transaction.GYRO_CONFIG.YGYRO_Cten  = 0;
  transaction.GYRO_CONFIG.XGYRO_Cten  = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_GYRO_CONFIG, transaction, bsc );

  // +-2G
  transaction.ACCEL_CONFIG.reserved     = 0;
  transaction.ACCEL_CONFIG.ACCEL_FS_SEL = 0;
  transaction.ACCEL_CONFIG.az_st_en     = 0;
  transaction.ACCEL_CONFIG.ay_st_en     = 0;
  transaction.ACCEL_CONFIG.ax_st_en     = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_ACCEL_CONFIG, transaction, bsc );

  // set acc data rates,enable acc LPF, bandwidth 184Hz
  transaction.ACCEL_CONFIG_2.A_DLPF_CFG      = 0;
  transaction.ACCEL_CONFIG_2.ACCEL_FCHOICE_B = 0;
  transaction.ACCEL_CONFIG_2.reserved        = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_ACCEL_CONFIG_2, transaction, bsc );

  // force into I2C mode, disabling I2C master
  transaction.USER_CTRL.SIG_COND_RST = 0;
  transaction.USER_CTRL.I2C_MST_RST  = 0;
  transaction.USER_CTRL.FIFO_RST     = 0;
  transaction.USER_CTRL.reserved0    = 0;
  transaction.USER_CTRL.I2C_IF_DIS   = 0;
  transaction.USER_CTRL.I2C_MST_EN   = 0;
  transaction.USER_CTRL.FIFO_EN      = 0;
  transaction.USER_CTRL.reserved1    = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_USER_CTRL, transaction, bsc );

  // enable bypass mode
  transaction.INT_PIN_CFG.reserved          = 0;
  transaction.INT_PIN_CFG.BYPASS_EN         = 1;
  transaction.INT_PIN_CFG.FSYNC_INT_MODE_EN = 0;
  transaction.INT_PIN_CFG.ACTL_FSYNC        = 0;
  transaction.INT_PIN_CFG.INT_ANYRD_2CLEAR  = 0;
  transaction.INT_PIN_CFG.LATCH_INT_EN      = 0;
  transaction.INT_PIN_CFG.OPEN              = 0;
  transaction.INT_PIN_CFG.ACTL              = 0;
  write_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_INT_PIN_CFG, transaction, bsc );

  return;
}



// INIT M
void initialize_magnetometer(
    struct calibration_data *     calibration_magnetometer,
    volatile struct bsc_register *bsc )
{
  union MPU9250_transaction_field_data  transaction;
  uint8_t                               data_block[3];

  // read WHOAMI from the magnetometer
  transaction = read_MPU9250_register( AK8963_ADDRESS, AK8963_REGISTER_WIA, bsc );
  printf( "mag   WHOAMI (0x48) = 0x%2.2X\n", transaction.WIA.WIA );

  // reset AK8963
  transaction.CNTL2.SRST      = 1;
  transaction.CNTL2.reserved  = 0;
  write_MPU9250_register( AK8963_ADDRESS, AK8963_REGISTER_CNTL2, transaction, bsc );
  usleep( 1000 );

  // I2C slave 0 register address from where to being data transfer
  // register value to 100Hz continuous measurement in 14bit
  transaction.CNTL1.MODE      = 6;
  transaction.CNTL1.BIT       = 0;
  transaction.CNTL1.reserved  = 0;
  write_MPU9250_register( AK8963_ADDRESS, AK8963_REGISTER_CNTL1, transaction, bsc );
  usleep( 1000 );

  // get the magnetometer calibration... extracted from the "calib_mag" function at https://github.com/brianc118/MPU9250/blob/master/MPU9250.cpp
  read_MPU9250_registers( AK8963_ADDRESS, AK8963_REGISTER_ASAX, data_block, sizeof(data_block), bsc );
  calibration_magnetometer->scale = (float)1;
  calibration_magnetometer->offset_x = ((((float)data_block[0])-128.0)/256.0+1.0);
  calibration_magnetometer->offset_y = ((((float)data_block[1])-128.0)/256.0+1.0);
  calibration_magnetometer->offset_z = ((((float)data_block[2])-128.0)/256.0+1.0);

  return;
}



// READ AG
void read_accelerometer_gyroscope(
    struct calibration_data *     calibration_accelerometer,
    struct calibration_data *     calibration_gyroscope,
    volatile struct bsc_register *bsc,
    FILE* fd)
{
  uint8_t                   data_block[6+2+6];
  union uint16_to_2uint8    ACCEL_XOUT;
  union uint16_to_2uint8    ACCEL_YOUT;
  union uint16_to_2uint8    ACCEL_ZOUT;
  union uint16_to_2uint8    GYRO_XOUT;
  union uint16_to_2uint8    GYRO_YOUT;
  union uint16_to_2uint8    GYRO_ZOUT;

  /*
   * poll the interrupt status register and it tells you when it is done
   * once it is done, read the data registers
   */
  do
  {
    usleep( 1000 );
  } while (read_MPU9250_register( MPU9250_ADDRESS, MPU9250_REGISTER_INT_STATUS, bsc ).INT_STATUS.RAW_DATA_RDY_INT == 0);

  // read the accelerometer values
  read_MPU9250_registers( MPU9250_ADDRESS, MPU9250_REGISTER_ACCEL_XOUT_H, data_block, sizeof(data_block), bsc );
  ACCEL_XOUT.field.H  = data_block[0];
  ACCEL_XOUT.field.L  = data_block[1];
  ACCEL_YOUT.field.H  = data_block[2];
  ACCEL_YOUT.field.L  = data_block[3];
  ACCEL_ZOUT.field.H  = data_block[4];
  ACCEL_ZOUT.field.L  = data_block[5];
  // TEMP_OUT.field.H = data_block[6];
  // TEMP_OUT.field.L = data_block[7];
  GYRO_XOUT.field.H   = data_block[8];
  GYRO_XOUT.field.L   = data_block[9];
  GYRO_YOUT.field.H   = data_block[10];
  GYRO_YOUT.field.L   = data_block[11];
  GYRO_ZOUT.field.H   = data_block[12];
  GYRO_ZOUT.field.L   = data_block[13];
	
  fprintf(fd, "Gyro X: %.2f deg\ty=%.2f deg\tz=%.2f deg\n",
      GYRO_XOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_x,
      GYRO_YOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_y,
      GYRO_ZOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_z);
  
  fprintf(fd, "Accel X: %.2f m/s^2\ty=%.2f m/s^2\tz=%.2f m/s^2\n",
      (ACCEL_XOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_x)*9.81,
      (ACCEL_YOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_y)*9.81,
      (ACCEL_ZOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_z)*9.81);

 /*fprintf(fd, "%.2f %.2f %.2f\n",
      GYRO_XOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_x,
      GYRO_YOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_y,
      GYRO_ZOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_z);*/
  

  /*printf( "Gyro X: %.2f deg\ty=%.2f deg\tz=%.2f deg\n",
      GYRO_XOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_x,
      GYRO_YOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_y,
      GYRO_ZOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_z );*/

  /*printf( "Accel X: %.2f m/s^2\ty=%.2f m/s^2\tz=%.2f m/s^2\n",
      (ACCEL_XOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_x)*9.81,
      (ACCEL_YOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_y)*9.81,
      (ACCEL_ZOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_z)*9.81 );*/

  int GYRO_X = (int)((GYRO_XOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_x) / Gyro_factor);
  int GYRO_Y = (int)((GYRO_YOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_y) / Gyro_factor);
  int GYRO_Z = (int)((GYRO_ZOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_z) / Gyro_factor);

  int ACCEL_X = (int)((ACCEL_XOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_x)*9.81 / Acce_factor);
  int ACCEL_Y = (int)((ACCEL_YOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_y)*9.81 / Acce_factor);
  int ACCEL_Z = (int)((ACCEL_ZOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_z)*9.81 / AcceZ_factor);

  //printf("Gx:%d, Gy:%d, Gz:%d, Ax:%d, Ay:%d, Az:%d\n", GYRO_X, GYRO_Y, GYRO_Z, ACCEL_X, ACCEL_Y, ACCEL_Z);
  
  // Put the data into buffer
  accelBufferX[printIndex] = abs(ACCEL_X);
  accelBufferY[printIndex] = abs(ACCEL_Y);
  accelBufferZ[printIndex] = abs(ACCEL_Z);
  gyroBufferX[printIndex] = abs(GYRO_X);
  gyroBufferY[printIndex] = abs(GYRO_Y);
  gyroBufferZ[printIndex] = abs(GYRO_Z);
  printIndex++;
  if(printIndex == 60000){
	  printf("\ndata buffer for accel and gyro are full, new value will be stocked at the head");
	  printIndex = 0;
  }
  
  // below calculate the average speed and distance
  // convert raw acelerometer data to acceleration in m/s^2
  float currAccelX = (ACCEL_XOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_x)*9.81;
  float currAccelY = (ACCEL_YOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_y)*9.81;
  //float currAccelZ = (ACCEL_ZOUT.signed_value*calibration_accelerometer->scale - calibration_accelerometer->offset_z)*9.81;
  
  // Estimate speed using trapezoidal rule
  float avgAccelX = (currAccelX + preAccelX) / 2;
  float avgAccelY = (currAccelY + preAccelY) / 2;
  //float avgAccelZ = (currAccelZ + preAccelZ) / 2;
  //float avgAccelAll = sqrt(avgAccelX * avgAccelX + avgAccelY * avgAccelY);
  float deltaSpeedX = avgAccelX * Time_Interval_S;
  float deltaSpeedY = avgAccelY * Time_Interval_S;
  float currSpeedX = preSpeedX + deltaSpeedX;
  float currSpeedY = preSpeedY + deltaSpeedY;
  float currSpeedConb = sqrt(currSpeedX * currSpeedX + currSpeedY * currSpeedY);
  
  // Estimate distance using trapezoidal rule
  float deltaDistance = currSpeedConb * Time_Interval_S;
  totalDistance += deltaDistance;
  
  // Update previous acceleration values
  preAccelX = currAccelX;
  preAccelY = currAccelY;
  preSpeedX = currSpeedX;
  preSpeedY = currSpeedY;
  //printf("\navgAccelX = %.2f, avgAccelY = %.2f, currSpeedX = %.2f, currSpeedY = %.2f, currSpeedConb = %.2f, deltaDistance = %.2f, totalDistance = %.2f",
  //				avgAccelX, avgAccelY, currSpeedX, currSpeedY, currSpeedConb, deltaDistance, totalDistance);
  
  // Below calaulte the angel changed in z axis
  float angular_velocity_Z = GYRO_ZOUT.signed_value*calibration_gyroscope->scale - calibration_gyroscope->offset_z;
  if (angular_velocity_Z > 4 || angular_velocity_Z < -4){
  	totalAngel += angular_velocity_Z * Time_Interval_S;
	}
  //printf("total angel now = %.2f\n", totalAngel + totalAngel * 0.286);
  return;
}



// READ M
void read_magnetometer(
    struct calibration_data *     calibration_magnetometer,
    volatile struct bsc_register *bsc )
{
  uint8_t                               data_block[7];
  union uint16_to_2uint8                MAG_XOUT;
  union uint16_to_2uint8                MAG_YOUT;
  union uint16_to_2uint8                MAG_ZOUT;
  union MPU9250_transaction_field_data  transaction;

  read_MPU9250_registers( AK8963_ADDRESS, AK8963_REGISTER_HXL, data_block, 7, bsc );
  // read must start from HXL and read seven bytes so that ST2 is read and the AK8963 will start the next conversion
  MAG_XOUT.field.L = data_block[0];
  MAG_XOUT.field.H = data_block[1];
  MAG_YOUT.field.L = data_block[2];
  MAG_YOUT.field.H = data_block[3];
  MAG_ZOUT.field.L = data_block[4];
  MAG_ZOUT.field.H = data_block[5];
  printf( "Mag X: %.2f uT\ty=%.2f uT\tz=%.2f uT\n",
      MAG_XOUT.signed_value*calibration_magnetometer->offset_x,
      MAG_YOUT.signed_value*calibration_magnetometer->offset_y,
      MAG_ZOUT.signed_value*calibration_magnetometer->offset_z );

  return;
}



void *left_wheel_control(void *arg)
{	
	struct thread_param * param = (struct thread_param *)arg;
    char command;
    bool Done = false;
    int start = 0;

    while(!Done){
		if (currMode == m1Mode){
			while(!FIFO_EMPTY(&left_fifo)){
				FIFO_REMOVE(&left_fifo, &command);
				switch(command){
					case 'i':
						if (leftPWMLevel != 100){
							leftPWMLevel += 5;
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d", leftPWMLevel);
						}
						else{
							printf("\ncurrent left PWM power = %d", leftPWMLevel);
						}

						break;
						
					case 'j':
						if (leftPWMLevel != 0){
							leftPWMLevel -= 5;
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d", leftPWMLevel);
						}
						else{
							printf("\ncurrent left PWM power = %d", leftPWMLevel);
						}
							
						break;
						
					case 'w':
						if (leftMode[0] == 0 && leftMode[1] == 0){
							// set direction to forward
							GPIO_CLR(param->gpio, 5);
							GPIO_SET(param->gpio, 6);
							leftMode[0] = 0;
							leftMode[1] = 1;
							// speed up to the value stored in leftPWMLevel
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d, forward", leftPWMLevel);
						}
						else if (leftMode[0] == 1 && leftMode[1] == 0){
							// current leftMode is at backward
							// slow dowm
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							// stop car for 0.1s
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							usleep(100*1000);
							// set direction to forward
							GPIO_CLR(param->gpio, 5);
							GPIO_SET(param->gpio, 6);
							leftMode[0] = 0;
							leftMode[1] = 1;
							// speed up to the value stored in leftPWMLevel
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d, forward", leftPWMLevel);
						}
						else if (leftMode[0] == 0 && leftMode[1] == 1){
							// forward again
							param->pwm->DAT1 = leftPWMLevel;
						}
						break;
						
					case 'x':
						if (leftMode[0] == 0 && leftMode[1] == 0){
							// set diection to backward
							GPIO_SET(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							leftMode[0] = 1;
							leftMode[1] = 0;
							// speed up to the value stored in leftPWMLevel
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d, backward", leftPWMLevel);
						}
						else if (leftMode[0] == 0 && leftMode[1] == 1){
							// current leftMove is forward
							// slow down first
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							// stop car for 0.1s
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							usleep(100*1000);
							// set direction to backward
							GPIO_SET(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							leftMode[0] = 1;
							leftMode[1] = 0;
							// speed up to the value stored in leftPWMLevel
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT1 = leftPWMLevel;
							printf("\ncurrent left PWM power = %d, backward", leftPWMLevel);
						}
						else if (leftMode[0] == 1 && leftMode[1] == 0){
							// backward again
							param->pwm->DAT1 = leftPWMLevel;
						}
						break;
						
					case 's': // stop
							if (leftMode[0] != 0 || leftMode[1] != 0){
								// not a stop mode
								//slow down to stop
								param->pwm->DAT1 = leftPWMLevel/2;
								usleep(100*1000);
								param->pwm->DAT1 = 0;
								usleep(100*1000);
								// set mode to stop
								GPIO_CLR(param->gpio, 5);
								GPIO_CLR(param->gpio, 6);
								leftMode[0] = 0;
								leftMode[1] = 0;
							}
							break;
						
					case 'a': // left turn
						if (leftMode[0] == 0 && leftMode[1] == 0){
							// stop turn
							// set direction to backward
							GPIO_SET(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							param->pwm->DAT1 = 85;
							usleep(stopTurnTime);
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							// set direction back to stop
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);	
						}
						else{ //on going turn
							param->pwm->DAT1 = 20;
							usleep(onMovingTurnTime);
							param->pwm->DAT1 = leftPWMLevel;
						}
						break;

					case 'd': // right turn
						if (leftMode[0] == 0 && leftMode[1] == 0){
							// stop turn
							// set direction to forward
							GPIO_CLR(param->gpio, 5);
							GPIO_SET(param->gpio, 6);
							param->pwm->DAT1 = 85;
							usleep(stopTurnTime);
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							//set direction back to stop
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
						}
						else{ //on going right turn
							param->pwm->DAT1 = 100;
							usleep(onMovingTurnTime);
							param->pwm->DAT1 = leftPWMLevel;
						}
						break;
					
					case 'q':
						Done = true;
						// slow down car, set to stop mode
						if (leftMode[0] != 0 || leftMode[1] != 0){
							// not a stop mode
							//slow down to stop
							param->pwm->DAT1 = leftPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							// set mode to stop
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							leftMode[0] = 0;
							leftMode[1] = 0;
						}
						else{
							//already stop mode, set again for sure
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							// set mode to stop
							GPIO_CLR(param->gpio, 5);
							GPIO_CLR(param->gpio, 6);
							leftMode[0] = 0;
							leftMode[1] = 0;
						}
						break;	
						
					default:
						usleep(50);
						break;
				}
			}
			param->pwm->DAT1 = leftPWMLevel;		
		}
		else if (currMode == m2Mode){
			// set the motor to full speed in this mode		
			while(!FIFO_EMPTY(&left_fifo)){
				FIFO_REMOVE(&left_fifo, &command);
				switch(command){
					case 's':
						// pause the car
						param->pwm->DAT1 = 0;
						usleep(100*1000);
						// set mode to stop
						GPIO_CLR(param->gpio, 5);
						GPIO_CLR(param->gpio, 6);
						leftMode[0] = 0;
						leftMode[1] = 0;
						start = 0;
						break;
					
					case 'w':
						// to start re-orient
						// note: every 90 degree rotation, the recorded data will have 20 degree off
						// add offset to correct the error
						start = 1;
						break;
						
					case 'q':
						Done = true;
						//slow down
						param->pwm->DAT1 = 0;
						usleep(100*1000);
						break;
						
					default:
						usleep(50);
						break;
				}
			}
			//float calibrateTotalAngel = totalAngel + totalAngel * 0.286;
			//int turnDegree = abs((int) calibrateTotalAngel);
			//printf("\nleft: turn degree = %d", turnDegree);
			if(start == 1){
				if (totalAngel > 3){
					//left turn, need to rotate the car to right
					//set direction to forward
					GPIO_CLR(param->gpio, 5);
					GPIO_SET(param->gpio, 6);
					leftMode[0] = 0;
					leftMode[1] = 1;
					param->pwm->DAT1 = 100;
					
				}
				else if (totalAngel < -3){
					// right turn, need to rotate the car to left
					// set direction to backward
					GPIO_SET(param->gpio, 5);
					GPIO_CLR(param->gpio, 6);
					leftMode[0] = 1;
					leftMode[1] = 0;
					param->pwm->DAT1 = 100;	
				}
				else{
					param->pwm->DAT1 = 0;
				}
			}
		}
	}
	return NULL;
 }



void *right_wheel_control(void *arg)
{
	struct thread_param * param = (struct thread_param *)arg;
    char command;
    bool Done = false;
	int start = 0;
    
    while(!Done){
		if (currMode == m1Mode){
			while(!FIFO_EMPTY(&right_fifo)){
				FIFO_REMOVE(&right_fifo, &command);
				switch(command){
					case 'i':
						if (rightPWMLevel != 100){
							rightPWMLevel += 5;
							param->pwm->DAT2 = rightPWMLevel;
							printf("\ncurrent right PWM power = %d", rightPWMLevel);
						}
						else{
							printf("\ncurrent right PWM power = %d", rightPWMLevel);
						}
						break;
						
					case 'j':
						if (rightPWMLevel != 0){
							rightPWMLevel -= 5;
							param->pwm->DAT2 = rightPWMLevel;
							printf("\ncurrent right PWM power = %d", rightPWMLevel);
						}
						else{
							printf("\ncurrent right PWM power = %d", rightPWMLevel);
						}
						break;
						
					case 'w':
						if (rightMode[0] == 0 && rightMode[1] == 0){
							// set direction to forward
							GPIO_CLR(param->gpio, 22);
							GPIO_SET(param->gpio, 23);
							rightMode[0] = 0;
							rightMode[1] = 1;
							// speed up to the value stored in rightPWMLevel
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT2 = rightPWMLevel - 7;
							printf("\ncurrent right PWM power = %d, forward", rightPWMLevel);
						}
						else if (rightMode[0] == 1 && rightMode[1] == 0){
							// current rightMode is at backward
							// slow dowm
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							// stop car for 0.1s
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							usleep(100*1000);
							// set direction to forward
							GPIO_CLR(param->gpio, 22);
							GPIO_SET(param->gpio, 23);
							rightMode[0] = 0;
							rightMode[1] = 1;
							// speed up to the value stored in rightPWMLevel
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT2 = rightPWMLevel - 7;
							printf("\ncurrent right PWM power = %d, forward", rightPWMLevel);
						}
						else if (rightMode[0] == 0 && rightMode[1] == 1){
							// forward again
							param->pwm->DAT2 = rightPWMLevel - 7;
						}
						break;
						
					case 'x':
						if (rightMode[0] == 0 && rightMode[1] == 0){
							// set diection to backward
							GPIO_SET(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							rightMode[0] = 1;
							rightMode[1] = 0;
							// speed up to the value stored in rightPWMLevel
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT2 = rightPWMLevel - 7;
							printf("\ncurrent right PWM power = %d, backward", rightPWMLevel);
						}
						else if (rightMode[0] == 0 && rightMode[1] == 1){
							// current rightMove is forward
							// slow down first
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							// stop car for 0.1s
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							usleep(100*1000);
							// set direction to backward
							GPIO_SET(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							rightMode[0] = 1;
							rightMode[1] = 0;
							// speed up to the value stored in rightPWMLevel
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000); //idle for 100ms to wait for moter to power up
							param->pwm->DAT2 = rightPWMLevel - 7;
							printf("\ncurrent right PWM power = %d, backward", rightPWMLevel);
						}
						else if (rightMode[0] == 1 && rightMode[1] == 0){
							// backward again
							param->pwm->DAT2 = rightPWMLevel - 7;
						}
						break;
						
					case 's': // stop
						if (rightMode[0] != 0 || rightMode[1] != 0){
							// not a stop mode
							//slow down to stop
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							// set mode to stop
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							rightMode[0] = 0;
							rightMode[1] = 0;
						}
						break;
						
					case 'a': // left turn
						if (rightMode[0] == 0 && rightMode[1] == 0){
							// stop turn
							// set direction to forward
							GPIO_CLR(param->gpio, 22);
							GPIO_SET(param->gpio, 23);
							param->pwm->DAT2 = 85;
							usleep(stopTurnTime);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							// set direction back to stop
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);	
						}
						else{ //on going turn
							param->pwm->DAT2 = 100;
							usleep(onMovingTurnTime);
							param->pwm->DAT2 = rightPWMLevel;
						}
						break;

					case 'd': // right turn
						if (rightMode[0] == 0 && rightMode[1] == 0){
							// stop turn
							// set direction to backward
							GPIO_SET(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							param->pwm->DAT2 = 85;
							usleep(stopTurnTime);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							//set direction back to stop
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
						}
						else{ //on going right turn
							param->pwm->DAT2 = 20;
							usleep(onMovingTurnTime);
							param->pwm->DAT2 = rightPWMLevel;
						}
						break;
					
					case 'q':
						Done = true;
						// slow down car, set to stop mode
						if (rightMode[0] != 0 || rightMode[1] != 0){
							// not a stop mode
							//slow down to stop
							param->pwm->DAT2 = rightPWMLevel/2;
							usleep(100*1000);
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							// set mode to stop
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							rightMode[0] = 0;
							rightMode[1] = 0;
						}
						else{
							//already stop mode, set again for sure
							param->pwm->DAT2= 0;
							usleep(100*1000);
							// set mode to stop
							GPIO_CLR(param->gpio, 22);
							GPIO_CLR(param->gpio, 23);
							rightMode[0] = 0;
							rightMode[1] = 0;
						}
						break;	
						
					default:
						usleep(50);
						break;
				}
			}
				param->pwm->DAT2 = rightPWMLevel - 7;	
		}	
	
		else if (currMode == m2Mode){
			while(!FIFO_EMPTY(&right_fifo)){
				FIFO_REMOVE(&right_fifo, &command);
				switch(command){
					case 's':
						// pause the car
						param->pwm->DAT2 = 0;
						usleep(100*1000);
						// set mode to stop
						GPIO_CLR(param->gpio, 22);
						GPIO_CLR(param->gpio, 23);
						rightMode[0] = 0;
						rightMode[1] = 0;
						start = 0;
						break;
						
					case 'w':
						// to start re-orient
						// note: every 90 degree rotation, the recorded data will have 20 degree off
						// add offset to correct the error
						start = 1;
						break;
							
					case 'q':
						Done = true;
						//slow down
						param->pwm->DAT2 = 0;
						usleep(100*1000);
						break;
							
					default:
						usleep(50);
						break;
				}
			}
			//float calibrateTotalAngel = totalAngel + totalAngel * 0.286;
			//nt turnDegree = abs((int) calibrateTotalAngel);
			//printf("\nright: turn degree = %d", turnDegree);
			if(start == 1){
				if (totalAngel > 3){
					//left turn, need to rotate the car to right
					// set direction to backward
					GPIO_SET(param->gpio, 22);
					GPIO_CLR(param->gpio, 23);
					rightMode[0] = 1;
					rightMode[1] = 0;
					param->pwm->DAT2 = 100;
				}
				else if (totalAngel < -3){
					// right turn, need to rotate the car to left
					// set direction to forward
					GPIO_CLR(param->gpio, 22);
					GPIO_SET(param->gpio, 23);
					rightMode[0] = 0;
					rightMode[1] = 1;
					param->pwm->DAT2 = 100;
				}
				else{
					param->pwm->DAT2 = 0;
				}
			}
		}
	}
	return NULL;
}


void *MPU9250_control(void *arg){
	struct MPU9250_param * param = (struct MPU9250_param *)arg;
    char command;
    bool Done = false;
	int m0ReadFlag = 0;
    int m1ReadFlag = 0;
	int m2ReadFlag = 0;
	
	while(!Done){
		// m0 mode: record data for five seconds
		if (currMode == m0Mode){			
			int count = 0;
			if(!FIFO_EMPTY(&MPU9250_fifo)){
				FIFO_REMOVE(&MPU9250_fifo, &command);	
				switch(command){
					case 'q':
						Done = true;
						break;
					
					case 'w': // afrer w key, start record
						m0ReadFlag = 1;
						break;
						
					case 's':
						m0ReadFlag = 0;
						break;
					
					case 'p':
						//print data
						int index = 0;
						printf("\nAx Ay Az Gx Gy Gz"); 
						while (index < printIndex){
							printf("\n%d  %d  %d  %d  %d  %d", accelBufferX[index], accelBufferY[index], accelBufferZ[index],
									gyroBufferX[index], gyroBufferY[index], gyroBufferZ[index]);
							index++;							  
						}
						break;
						
					default:
						usleep(10);
						break;
				}
			}
			while(count < 500 && m0ReadFlag == 1){
				/*
				if(!FIFO_EMPTY(&MPU9250_fifo)){
					FIFO_REMOVE(&MPU9250_fifo, &command);	
					if (command == 'q'){
						Done = true;
						break;
					}
				}*/
				
				//read IMU data
				read_accelerometer_gyroscope(&param->calibration_accelerometer, &param->calibration_gyroscope, param->bsc , param->fd_m0);
				usleep(10*1000); //sleep 10ms, 100/s
				count++;

			}
			m0ReadFlag = 0;
		}
		else if (currMode == m1Mode){
			if(!FIFO_EMPTY(&MPU9250_fifo)){
				FIFO_REMOVE(&MPU9250_fifo, &command);	
				switch(command){
					case 'q':
						Done = true;
						break;
					
					case 'w':
						m1ReadFlag = 1;
						break;
						
					case 's':
						m1ReadFlag = 0;
						break;
						
					case 'p':
						//print data
						int index = 0;
						printf("\nAx Ay Az Gx Gy Gz"); 
						while (index < printIndex){
							printf("\n%d  %d  %d  %d  %d  %d", accelBufferX[index], accelBufferY[index], accelBufferZ[index],
									gyroBufferX[index], gyroBufferY[index], gyroBufferZ[index]);	
							index++;						  
						}
						break;
						
					case 'n':
						//calculate average speed and total diatance
						float avgSpeed = totalDistance * 1.8/ (Time_Interval_S * m1m2DataCounter);
						
						printf("\nm1 mode: total traveled diatance is %.2f m, the average speed is %.2f m\\s^2", totalDistance * 1.8, avgSpeed);
						//clean up the valuables for next time use
						preAccelX = 0.0;
						preAccelY = 0.0;
						//totalSpeed = 0.0;
						totalDistance = 0.0;
						preSpeedX = 0.0;
						preSpeedY = 0.0;
						m1m2DataCounter = 0;
						break;
						
					default:
						usleep(10);
						break;
				}
			}
			// read IMU data
			if (m1ReadFlag == 1){
				read_accelerometer_gyroscope(&param->calibration_accelerometer, &param->calibration_gyroscope, param->bsc, param->fd_m1);
				usleep(10*1000); //sleep 10ms, 100/s
				m1m2DataCounter++;
			}
			
		}
		else if (currMode == m2Mode){
			if(!FIFO_EMPTY(&MPU9250_fifo)){
				FIFO_REMOVE(&MPU9250_fifo, &command);	
				switch(command){
					case 'q':
						Done = true;
						break;
					case 'r':
						m2ReadFlag = 1;
						printIndex = 0;
						break;
					
					case 'w':
						m2ReadFlag = 1;
						break;
						
					case 's':
						m2ReadFlag = 0;
						break;
						
					case 'p':	
						//print data
						int index = 0;
						printf("\nAx Ay Az Gx Gy Gz"); 
						while (index < printIndex){
							printf("\n%d  %d  %d  %d  %d  %d", accelBufferX[index], accelBufferY[index], accelBufferZ[index],
									gyroBufferX[index], gyroBufferY[index], gyroBufferZ[index]);	
							index++;						  
						}
						break;
						
					case 'n':
						//calculate average speed and total diatance
						float avgSpeed = totalDistance * 0.3 / (Time_Interval_S * m1m2DataCounter);
						
						printf("\nm2 mode: total traveled diatance is %.2f m, the average speed is %.2f m\\s^2", totalDistance * 0.3, avgSpeed);
						//clean up the valuables for next time use
						preAccelX = 0.0;
						preAccelY = 0.0;
						//totalSpeed = 0.0;
						totalDistance = 0.0;
						preSpeedX = 0.0;
						preSpeedY = 0.0;
						m1m2DataCounter = 0;
						break;	
					/*
					case 't'://dont know and dont have enough time to do t, but have some premade map.
						//print map
						printMap(printMapFlag);
						printMapFlag++;
						if(printMapFlag == 4){
							printMapFlag = 0;
						}*/
						
					default:
						usleep(10);
						break;
				}
			}
			// read IMU data
			if (m2ReadFlag == 1){
				read_accelerometer_gyroscope(&param->calibration_accelerometer, &param->calibration_gyroscope, param->bsc, param->fd_m2);
				usleep(10*1000); //sleep 10ms, 100/s
				m1m2DataCounter++;
			}
		}
	}
	return NULL;
}


int main( void )
{
  struct io_peripherals *io;
  
  FILE *fd_m0 = fopen("hw8m0data.txt", "w");
  FILE *fd_m1 = fopen("hw8m1data.txt", "w");
  FILE *fd_m2 = fopen("hw8m2data.txt", "w");
  pthread_t  leftThread;
  pthread_t  rightThread;
  pthread_t  MPU9250Thread;
  bool Done = false;
  struct thread_param  leftParam;
  struct thread_param  rightParam;
  struct MPU9250_param  MPU9250Param;
  //struct calibration_data calibration_accelerometer;
  //struct calibration_data calibration_gyroscope;
  
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    printf("This Program aims to control car driving by commands\n");
    printf("There are Three modes for use, 'm0': data recording for 5s, 'm1': manual driving mode; 'm2': self black line tracing mode\n");
	printf("Under the 'm0' mode, we have commands listed below\n");
	printf("(1) 'm1': change to m1 mode\n");
	printf("(2) 'm2': chnage to m2 mode\n");
	printf("(3) 'w' : start recording\n");
	printf("(4) 's' : stop\n");
	printf("(5) 'p' : print data\n");
    printf("Under the 'm1' mode, we have commands listed below\n");
    printf("(1) 's': Stop\n");
    printf("(2) 'w': Forward\n");
    printf("(3) 'x': Backward\n");
    printf("(4) 'i': Faster, 5%% PWM power increase for each 'i' key hit\n");
    printf("(5) 'j': Slower, 5%% PWM power decrease for each 'j' key hit\n");        
    printf("(6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition\n");    
    printf("(7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition\n");
    printf("(8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key\n");
    printf("(9) 'm2': change mode to m2\n");
    printf("(10) 'm0': change to m0 mode\n");
	printf("(11) 'p': Print data\n");
	printf("(12) 'n': Display total distance and average speed\n");
	printf("Under the 'm2' mode, we have commands listed below\n");
    printf("(1) 's': to pause the line tracing until next Forward command\n");
    printf("(2) 'w': to start re_orient direction\n");
    printf("(3) 'q': to quit all program\n");
    printf("(4) 'm1': change mode to m1\n");
	printf("(5) 'm0': change mode to m0\n");
	printf("(6) 'p': print data\n");
	printf("(7) 'n': Display total distance and average speed\n");
	printf("(8) 'r': Reset abd Start collecting/recording the IMU data\n");
    printf("Program must end with ‘q’ command and when the program ends.\n"); 

    
    enable_pwm_clock(io->cm, io->pwm);  /* Hardware pwm needs clock to work */
    
    // set the init status for GPIO 
    /* set the pin function to alternate function 0 for GPIO02 (I2C1, SDA) */
    /* set the pin function to alternate function 0 for GPIO03 (I2C1, SCL) */
    io->gpio->GPFSEL0.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
    io->gpio->GPFSEL0.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;
    
    /* set the pin function to alternate function 0 for GPIO12, PWM for LED on GPIO12 */
    /* set the pin function to alternate function 0 for GPIO13, PWM for LED on GPIO13 */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;
    
    /* set the pin function to OUTPUT for GPIO 22*/
    /* set the pin function to OUTPUT for GPIO 23*/
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    /* set the pin function to OUTPUT for GPIO 05*/
    /* set the pin function to OUTPUT for GPIO 06*/
    io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_OUTPUT;   //GPIO 5
    io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_OUTPUT;   //GPIO 6
    
    /* set the pin function to OUTPUT for GPIO 24*/
    /* set the pin function to OUTPUT for GPIO 25*/
    io->gpio->GPFSEL2.field.FSEL4 = GPFSEL_INPUT;   //GPIO 24
    io->gpio->GPFSEL2.field.FSEL5 = GPFSEL_INPUT;   //GPIO 25
    
    // set initial GPIO output state - OFF
    /*right direction control 00: stop, 01: forward, 10: backward*/
    GPIO_CLR(io->gpio, 22);
    GPIO_CLR(io->gpio, 23);
    /*left direction control 00: stop, 01: forward, 10: backward*/
    GPIO_CLR(io->gpio, 05);
    GPIO_CLR(io->gpio, 06);
    
    // init PWM and configure the PWM param
	pwm_setup(PWM_RANGE, io);
    
    /* configure the I2C interface */
    io->bsc->DIV.field.CDIV  = (PERIPHERAL_CLOCK*10)/400000;
    io->bsc->DEL.field.REDL  = 0x30;
    io->bsc->DEL.field.FEDL  = 0x30;
    io->bsc->CLKT.field.TOUT = 0x40;
    io->bsc->C.field.INTD    = 0;
    io->bsc->C.field.INTT    = 0;
    io->bsc->C.field.INTR    = 0;
    io->bsc->C.field.I2CEN   = 1;
    io->bsc->C.field.CLEAR   = 1;
    
    // init accelerometer and gyroscope
    initialize_accelerometer_and_gyroscope(&MPU9250Param.calibration_accelerometer, &MPU9250Param.calibration_gyroscope, io->bsc );

    // Init Parameter
    leftParam.gpio = io->gpio;
    leftParam.pwm = io->pwm;
    rightParam.gpio = io->gpio;
    rightParam.pwm = io->pwm;
    MPU9250Param.bsc = io->bsc;
    MPU9250Param.fd_m0 = fd_m0;
    MPU9250Param.fd_m1 = fd_m1;
    MPU9250Param.fd_m2 = fd_m2;
    
    
    // Create two threads leftThread and rightThread, and run them in parallel
    pthread_create(&leftThread, NULL, left_wheel_control, (void *)&leftParam);
    pthread_create(&rightThread, NULL, right_wheel_control, (void *)&rightParam);
    pthread_create(&MPU9250Thread, NULL, MPU9250_control, (void *)&MPU9250Param);
    
    
	while(!Done){
		switch(get_pressed_key()){
			case 'q':
				Done = true;
				FIFO_INSERT(&left_fifo, 'q');
				FIFO_INSERT(&right_fifo, 'q');
				FIFO_INSERT(&MPU9250_fifo, 'q');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'q';
				printf("\nHW8> 'q'");
				break;
			
			case 's':
				if (currMode == m0Mode){
					FIFO_INSERT(&MPU9250_fifo, 's');
				}
				else{
					FIFO_INSERT(&left_fifo, 's');
					FIFO_INSERT(&right_fifo, 's');
					FIFO_INSERT(&MPU9250_fifo, 's');
				}
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 's';
				printf("\nHW8> 's'");
				break;
			
			case 'w':
				if(currMode == m0Mode){
					FIFO_INSERT(&MPU9250_fifo, 'w');
				}
				else{
					FIFO_INSERT(&left_fifo, 'w');
					FIFO_INSERT(&right_fifo, 'w');
					FIFO_INSERT(&MPU9250_fifo, 'w');
				}
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'w';
				printf("\nHW8> 'w'");
				break;
			
			case 'x':
				FIFO_INSERT(&left_fifo, 'x');
				FIFO_INSERT(&right_fifo, 'x');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'x';
				printf("\nHW8> 'x'");
				break;
			
			case 'i':
				FIFO_INSERT(&left_fifo, 'i');
				FIFO_INSERT(&right_fifo, 'i');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'i';
				printf("\nHW8> 'i'");
				break;
			
			case 'j':
				FIFO_INSERT(&left_fifo, 'j');
				FIFO_INSERT(&right_fifo, 'j');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'j';
				printf("\nHW8> 'j'");
				break;
			
			case 'a':
				FIFO_INSERT(&left_fifo, 'a');
				FIFO_INSERT(&right_fifo, 'a');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'a';
				printf("\nHW8> 'a'");
				break;
				
			case 'd':
				FIFO_INSERT(&left_fifo, 'd');
				FIFO_INSERT(&right_fifo, 'd');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'd';
				printf("\nHW8> 'd'");
				break;
				
			case 'p':
				FIFO_INSERT(&MPU9250_fifo, 'p');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'p';
				printf("\nHW8> 'p'");
				break;
				
			case 'n':
				FIFO_INSERT(&MPU9250_fifo, 'n');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'n';
				printf("\nHW8> 'n'");
				break;
			/*
			case 't':
				FIFO_INSERT(&MPU9250_fifo, 't');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 't';
				printf("\nHW8> 't'");
				break;*/
			case 'r':
				FIFO_INSERT(&MPU9250_fifo, 'r');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'r';
				printf("\nHW8> 'r'");
				break;
			
			case 'm':
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'm';
				break;
				
			case '0':
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = '0';
				break;
				
			case '1':
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = '1';
				break;
				
			case '2':
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = '2';
				break;
			
			default:
				usleep(50);
				break;
		}
		
		if (commandBuffer[0] == 'm' && commandBuffer[1] == '1'){
			if (currMode == m1Mode){
				//printf("\nCurrent Mode is already m1 mode");
				continue;
			}
			else{
				printf("\nHW8> 'm1'");
			}
			currMode = m1Mode;
			printIndex = 0; //set to new mode, restart to collect data

			// clean up valuable for current mode to claculate total distance and average speed
			preAccelX = 0.0;
			preAccelY = 0.0;
			//totalSpeed = 0.0;
			totalDistance = 0.0;
			preSpeedX = 0.0;
			preSpeedY = 0.0;
			m1m2DataCounter = 0;
			totalAngel = 0.0;
		}
		else if (commandBuffer[0] == 'm' && commandBuffer[1] == '2'){
			if (currMode == m2Mode){
				//printf("\nCurrent Mode is already m2 mode");
				continue;
			}
			else{
				printf("\nHW8> 'm2'");
			}
			currMode = m2Mode;
			printIndex = 0; //set to new mode, restart to collect data

			// clean up valuable for current mode to claculate total distance and average speed
			preAccelX = 0.0;
			preAccelY = 0.0;
			//totalSpeed = 0.0;
			totalDistance = 0.0;
			preSpeedX = 0.0;
			preSpeedY = 0.0;
			m1m2DataCounter = 0;
			totalAngel = 0.0;
		}
		else if (commandBuffer[0] == 'm' && commandBuffer[1] == '0'){
			if (currMode == m0Mode){
				continue;
			}
			else{
				printf("\nHW8> 'm0'");
			}
			currMode = m0Mode;
			printIndex = 0; //set to new mode, restart to collect data
		}
		
		//read_accelerometer_gyroscope( &calibration_accelerometer, &calibration_gyroscope, io->bsc );
	}
	
	pthread_join(leftThread, NULL);
	pthread_join(rightThread, NULL);
	pthread_join(MPU9250Thread, NULL);
	printf("\nQUIT the program\n");
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL0.field.FSEL2 = GPFSEL_INPUT;
	io->gpio->GPFSEL0.field.FSEL3 = GPFSEL_INPUT;
	io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
	io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;
	io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
	io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
	io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_INPUT;
	io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_INPUT;
    fclose(fd_m0);
    fclose(fd_m1);
    fclose(fd_m2);
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}

