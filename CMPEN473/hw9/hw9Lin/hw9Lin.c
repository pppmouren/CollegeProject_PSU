/**************************************************
* File:  hw9Lin.c 
* Homework 9, manual car driving, and camera base red laser tracig
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 9, we will use command to control car driving
* Two Modes: 'm1' mode for manual driving, 'm2' mode for red laser light tracing 
 
* 1. "m1" mode:
 (1) 's': Stop, and stop IMU data collecting
 (2) 'w': Forward, and also start IMU data collecting
 (3) 'x': Backward
 (4) 'i': Faster, 5% PWM power increase for each 'i' key hit
 (5) 'j': Slower, 5% PWM power decrease for each 'j' key hit
 (6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition
 (7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition
 (8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key
 (9) 'm2': mode change to m2

* 2. "m2" mode:
 (1) 'q': to quit the program
 (2) 'm1': mode change to m1
 (When m2 mode is entered, start moving the RoboCar to follow a red laser light point if there is
one on the camera image. If there is no red laser light point on the camera image, then stop
the RoboCar and wait for a laser light point)
* 
*Program must end with ‘q’ command and when the program ends.
* 
* GPIO Usage:
* 1. GPIO 12,12 used as PWM channel 1,2 for wheel speed
* 2. GPIO 22,23 used as output for right wheel direction, 00 for stop, 01 for forward, 10 or backward
* 3. GPIO 5,6 usd as output for left wheel direction, 00 for stop, 01 for forward, 10 for backward
* 4. GPIO 24, 25 used as input port to receive the information from the IR sensors. 24 for left sensor, 25 for right
* 		(when the input pin is low, indicator light is on, obsticle detected, NO BLACK LINE!)
* 		(when the input pin is high, indicator light is off, no obsticle detected, BLACK LINE!)
* 
* QUICK NOTE:
* 1. always display on a terminal window the prompt ‘Hw9>' and display
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
* 		3.1. In m2 mode, stop the RoboCar when the red laser light point is at the center of the camera
*		 	 image, or no laser light point shows on the camera image. That is, RoboCar moves in order
*   		 to place the red laser light point to the center of the camera image.
* 		3.2. In m2 mode, mode can be changed by typing m1 command while RoboCar is at stop state.
* 		3.3. RoboCar moves at full speed but it may move slower when turning, but not stopping;
* 		3.4. RoboCar stops when there is no red laser light point or the red laser light point is 
* 			 at the center of the camera image.
* 
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
#include <linux/videodev2.h>
#include <time.h>
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
#include "pixel_format_RGB.h"
#include "video_interface.h"
#include "scale_image_data.h"
#include "draw_bitmap.h"

#define PWM_RANGE 100
#define FIFO_LENGTH 2048
#define stopTurnTime 150*1000
#define onMovingTurnTime 240*1000
#define m2TurnTime 5*1000
#define m1Mode 1
#define m2Mode 2
#define imageScaleFactor 8
#define imageHeightDownLimit 20
#define imageHeightUpLimit 39
#define imageWidthDownLimit 27
#define imageWidthUpLimit 52

uint32_t rightPWMLevel = 50;    /* right wheel PWM as duty cycle, set default to 50 */
uint32_t leftPWMLevel = 50;     /* lefwt wheel PWM as duty cycle set default to 50*/
int leftMode[2] = {0,0};
int rightMode[2] = {0,0};
int currMode = m1Mode;
char commandBuffer[2] = {'N','N'};
size_t laserLocation[2];

//define two fifos, one for left, one for right
FIFO_TYPE(char, FIFO_LENGTH, MYFIFO);
struct MYFIFO left_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
struct MYFIFO right_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
struct MYFIFO image_process_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};

struct thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
  volatile struct pwm_register *pwm; // PWM port registers
};

struct image_thread_param
{
    struct video_interface_handle_t * handle;
    struct image_t                    image;
    unsigned char *                   scaled_data;
    struct pixel_format_RGB *         scaled_RGB_data;
    unsigned int                      scaled_height;
    unsigned int                      scaled_width;  
    int argc;
    char **argv;		
};

void *left_wheel_control(void *arg)
{	
    struct thread_param * param = (struct thread_param *)arg;
    char command;
    bool Done = false;
    
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
	    //keep looking at the laserLocation, make sure the laserLoacation index is around the center of the image
	    while(!FIFO_EMPTY(&left_fifo)){
		FIFO_REMOVE(&left_fifo, &command);
		switch(command){
		    case 'q':
			Done = true;
			break;
		    
		    default:
			usleep(50);
			break;
		}
	    }
	    // the laser at the senter region
	    if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		param->pwm->DAT1 = 0;
		GPIO_CLR(param->gpio, 5);
		GPIO_CLR(param->gpio, 6);
	    }
	    // the laser is at the up right section
	    else if (laserLocation[0] < imageHeightDownLimit && laserLocation[1] > imageWidthUpLimit){
		//car should moving forward-right
		GPIO_CLR(param->gpio, 5);
		GPIO_SET(param->gpio, 6);
		param->pwm->DAT1 = 70;
	    }
	    // the laser is at the up-left section
	    else if (laserLocation[0] < imageHeightDownLimit && laserLocation[1] < imageWidthDownLimit){
		//car should move forward-left
		GPIO_CLR(param->gpio, 5);
		GPIO_SET(param->gpio, 6);
		param->pwm->DAT1 = 40;
	    }
	    // the laser is at the down-left section
	    else if (laserLocation[0] > imageHeightUpLimit && laserLocation[1] < imageWidthDownLimit){
		// car should move backward-right
		GPIO_SET(param->gpio, 5);
		GPIO_CLR(param->gpio, 6);
		param->pwm->DAT1 = 70;
	    }
	    // the laser is at the down-right section
	    else if (laserLocation[0] > imageHeightUpLimit && laserLocation[1] > imageWidthUpLimit){
		// car should move backward-left
		GPIO_SET(param->gpio, 5);
		GPIO_CLR(param->gpio, 6);
		param->pwm->DAT1 = 40;
	    }
	    // the laser is at the center up region
	    else if (laserLocation[0] < imageHeightDownLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		//car should move forward
		GPIO_CLR(param->gpio, 5);
		GPIO_SET(param->gpio, 6);
		param->pwm->DAT1 = 70;
	    }
	    // the laser is at the center downward region
	    else if (laserLocation[0] > imageHeightUpLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		//car should move backward
		GPIO_SET(param->gpio, 5);
		GPIO_CLR(param->gpio, 6);
		param->pwm->DAT1 = 70;
	    }
	    // the laser is at the center left region
	    else if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && laserLocation[1] < imageWidthDownLimit){
		//car should rotate counterclockwise
		GPIO_SET(param->gpio, 5);
		GPIO_CLR(param->gpio, 6);
		param->pwm->DAT1 = 70;
	    }
	    // the laser is at the center right region
	    else if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && laserLocation[1] > imageWidthUpLimit){
		// car should rotate clockwise
		GPIO_CLR(param->gpio, 5);
		GPIO_SET(param->gpio, 6);
		param->pwm->DAT1 = 70;
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
	    //keep looking at the laserLocation, make sure the laserLoacation index is around the center of the image
	    while(!FIFO_EMPTY(&right_fifo)){
		FIFO_REMOVE(&right_fifo, &command);
		switch(command){
		    case 'q':
			Done = true;
			break;
		    
		    default:
			usleep(50);
			break;
		}
	    }
	    // the laser at the senter region
	    if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		param->pwm->DAT2 = 0;
		GPIO_CLR(param->gpio, 22);
		GPIO_CLR(param->gpio, 23);
	    }
	    // the laser is at the up right section
	    else if (laserLocation[0] < imageHeightDownLimit && laserLocation[1] > imageWidthUpLimit){
		//car should moving forward-right
		GPIO_CLR(param->gpio, 22);
		GPIO_SET(param->gpio, 23);
		param->pwm->DAT2 = 40;
	    }
	    // the laser is at the up-left section
	    else if (laserLocation[0] < imageHeightDownLimit && laserLocation[1] < imageWidthDownLimit){
		//car should move forward-left
		GPIO_CLR(param->gpio, 22);
		GPIO_SET(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	    // the laser is at the down-left section
	    else if (laserLocation[0] > imageHeightUpLimit && laserLocation[1] < imageWidthDownLimit){
		// car should move backward-right
		GPIO_SET(param->gpio, 22);
		GPIO_CLR(param->gpio, 23);
		param->pwm->DAT2 = 40;
	    }
	    // the laser is at the down-right section
	    else if (laserLocation[0] > imageHeightUpLimit && laserLocation[1] > imageWidthUpLimit){
		// car should move backward-left
		GPIO_SET(param->gpio, 22);
		GPIO_CLR(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	    // the laser is at the center up region
	    else if (laserLocation[0] < imageHeightDownLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		//car should move forward
		GPIO_CLR(param->gpio, 22);
		GPIO_SET(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	    // the laser is at the center downward region
	    else if (laserLocation[0] > imageHeightUpLimit && imageWidthDownLimit <= laserLocation[1] && laserLocation[1] <= imageWidthUpLimit){
		//car should move backward
		GPIO_SET(param->gpio, 22);
		GPIO_CLR(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	    // the laser is at the center left region
	    else if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && laserLocation[1] < imageWidthDownLimit){
		//car should rotate counterclockwise
		GPIO_CLR(param->gpio, 22);
		GPIO_SET(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	    // the laser is at the center right region
	    else if (imageHeightDownLimit <= laserLocation[0] && laserLocation[0] <= imageHeightUpLimit && laserLocation[1] > imageWidthUpLimit){
		// car should rotate clockwise
		GPIO_SET(param->gpio, 22);
		GPIO_CLR(param->gpio, 23);
		param->pwm->DAT2 = 70;
	    }
	}
    }
    return NULL;
}

void *image_process_control(void *arg)
{
    struct image_thread_param * param = (struct image_thread_param *)arg;
    bool Done = false;
    char command;
    int count = 0;
    
    // init the parameter, capture the image, process image to get the laser point idex
    param->handle = video_interface_open("/dev/video0");
    video_interface_print_modes(param->handle);
    if (video_interface_set_mode_auto(param->handle))
    {
	printf("configured resolution: %zux%zu\n", param->handle->configured_width, param->handle->configured_height);
	// set up the buffer for scaled data
	param->scaled_width  = param->handle->configured_width/imageScaleFactor;
	param->scaled_height = param->handle->configured_height/imageScaleFactor;
	param->scaled_data     = (unsigned char *)malloc( sizeof(param->image)/(imageScaleFactor*imageScaleFactor) );
	param->scaled_RGB_data = (struct pixel_format_RGB *)param->scaled_data;
	
	// create the window to show the bitmap
	//draw_bitmap_create_window(param->argc, param->argv, param->scaled_width, param->scaled_height);

	while (!Done)
	{
	    while(!FIFO_EMPTY(&image_process_fifo)){
		FIFO_REMOVE(&image_process_fifo, &command);
		switch(command){
		    case 'q':
			Done = true;
			break;
		    
		    default:
			usleep(50);
			break;
		}
	    }
	    // capture an image
	    if (video_interface_get_image(param->handle, &param->image))
	    {
		// scale the image to a more agreeable size
		scale_image_data(
		    (struct pixel_format_RGB *)&param->image,
		    param->handle->configured_height,
		    param->handle->configured_width,
		    param->scaled_RGB_data,
		    imageScaleFactor,
		    imageScaleFactor );      
		    
		#if 0
		// set the color manually
		// since scale_data is mallocaed in memory, then scaled_PGB_data will be a linear structure
		//adjust the scaled_height to fill up the image with yellow color
		for (size_t i = 0; i < param->scaled_width * param->scaled_height; i++)
		{
		    // row-oriented data
		    param->scaled_RGB_data[i].R = 255;
		    param->scaled_RGB_data[i].G = 255;
		    param->scaled_RGB_data[i].B = 0;
		}
		#endif

		// Modify the image data
		int redLayer[param->scaled_height][param->scaled_width];
		int greenLayer[param->scaled_height][param->scaled_width];
		int blueLayer[param->scaled_height][param->scaled_width];
		//fill up res, green, blue layer buffer
		for (size_t i = 0; i < param->scaled_width * param->scaled_height; i++)
		{
		    size_t row = i / param->scaled_width;
		    size_t col = i % param->scaled_width;
		    redLayer[row][col] = param->scaled_RGB_data[i].R;
		    greenLayer[row][col] = param->scaled_RGB_data[i].G;
		    blueLayer[row][col] = param->scaled_RGB_data[i].B;
		}
		
		int flag = 0;
		// get the laser point index
		for (size_t row = 0; row < param->scaled_height; row++){
		    for (size_t col = 0; col < param->scaled_width; col++){
			if (redLayer[row][col] >= 254 && greenLayer[row][col] >= 254 && blueLayer[row][col] >= 254){
			    laserLocation[0] = row;
			    laserLocation[1] = col;
			    flag = 1;
			    count = 0;
			    //printf("laserLocation: [%d, %d]\n", row, col);
			    break;
			}
		    }
		}
		if (flag == 0){
		    count ++;
		    if (count >= 15){
			//if dont detect layer point for consecutive 7 times, then stop the car
			laserLocation[0] = 30;
			laserLocation[1] = 40;
		    }
		    //printf("NO LASER FOUND, COUNT = %d\n", count);
		}

		
		//display RGB layer data
		#if 0
		int red_count = 0;
		int blue_count = 0;
		int green_count = 0;
		printf("RED LAYER \n");
		for (size_t i = 0; i < param->scaled_height; i++)
		{
		    for (size_t j = 0; j < param->scaled_width; j++)
		    {
			printf("%3d ", redLayer[i][j]);
			if(redLayer[i][j] >=250){
			    red_count++;
			}
		    }
		    printf("\n");
		}
		printf("red >= 250: %d\n", red_count);
		printf("\n");
		printf("GREEN Layer\n");
		for (size_t i = 0; i < param->scaled_height; i++)
		{
		    for (size_t j = 0; j < param->scaled_width; j++)
		    {
			printf("%3d ", greenLayer[i][j]);
			if(greenLayer[i][j] >=250){
			    green_count++;
			}
		    }
		    printf("\n");
		}
		printf("green >= 250: %d\n", green_count);
		printf("\n");
		printf("BLUE Layer\n");
		for (size_t i = 0; i < param->scaled_height; i++)
		{
		    for (size_t j = 0; j < param->scaled_width; j++)
		    {
			printf("%3d ", blueLayer[i][j]);
			if(blueLayer[i][j] >=250){
			    blue_count++;
			}
		    }
		    printf("\n");
		}
		printf("blue >= 250: %d\n", blue_count);
		printf("\n");
		
		red_count = 0;
		blue_count = 0;
		green_count = 0;
		sleep(2);
		#endif
		
		// display what the camera sees
		//draw_bitmap_display(param->scaled_RGB_data);
	    }
	    else
	    {
		printf( "did not get an image\n" );
	    }
	}
    }
    else
    {
	printf( "failed to configure\n" );
    }

    // clean up
    //draw_bitmap_close_window();
    video_interface_close(param->handle);
    free(param->scaled_data);
    
    return NULL;
}


int main(int argc, char * argv[] )
{
    struct io_peripherals *io;

    pthread_t  leftThread;
    pthread_t  rightThread;
    pthread_t  imageThread;
    bool Done = false;
    struct thread_param  leftParam;
    struct thread_param  rightParam;
    struct image_thread_param imageParam;

    io = import_registers();
    if (io != NULL)
    {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    printf("This Program aims to control car driving by commands\n");
    printf("There are two modes for use, 'm1': manual driving mode; 'm2': red laser light tracing\n");
    printf("Under the 'm1' mode, we have commands listed below\n");
    printf("(1) 's': Stop\n");
    printf("(2) 'w': Forward\n");
    printf("(3) 'x': Backward\n");
    printf("(4) 'i': Faster, 5%% PWM power increase for each 'i' key hit\n");
    printf("(5) 'j': Slower, 5%% PWM power decrease for each 'j' key hit\n");        
    printf("(6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition\n");    
    printf("(7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition\n");
    printf("(8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key\n");
    printf("(9) 'm2': change mode to m2");
    printf("Under the 'm2' mode, we have commands listed below\n");
    printf("(1) 'q': to quit all program\n");
    printf("(2) 'm1': change mode to m1\n");
    printf("Program must end with ‘q’ command and when the program ends.\n");        

    enable_pwm_clock(io->cm, io->pwm);  /* Hardware pwm needs clock to work */

    // set the init status for GPIO 
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
    io->pwm->RNG1 = PWM_RANGE;     /* the range value, 100 level steps */
    io->pwm->RNG2 = PWM_RANGE;     /* the range value, 100 level steps */
    io->pwm->DAT1 = 0;             /* initial beginning level=0% */
    io->pwm->DAT2 = 0;             /* initial beginning level=0% */
    io->pwm->CTL.field.MODE1 = 0;  /* PWM mode */
    io->pwm->CTL.field.MODE2 = 0;  /* PWM mode */
    io->pwm->CTL.field.RPTL1 = 1;  /* not using FIFO, but repeat the last byte anyway */
    io->pwm->CTL.field.RPTL2 = 1;  /* not using FIFO, but repeat the last byte anyway */
    io->pwm->CTL.field.SBIT1 = 0;  /* idle low */
    io->pwm->CTL.field.SBIT2 = 0;  /* idle low */
    io->pwm->CTL.field.POLA1 = 0;  /* non-inverted polarity */
    io->pwm->CTL.field.POLA2 = 0;  /* non-inverted polarity */
    io->pwm->CTL.field.USEF1 = 0;  /* do not use FIFO */
    io->pwm->CTL.field.USEF2 = 0;  /* do not use FIFO */
    io->pwm->CTL.field.MSEN1 = 1;  /* use M/S algorithm, level=pwm->DAT1/PWM_RANGE */
    io->pwm->CTL.field.MSEN2 = 1;  /* use M/S algorithm, level=pwm->DAT2/PWM_RANGE */
    io->pwm->CTL.field.CLRF1 = 1;  /* clear the FIFO, even though it is not used */
    io->pwm->CTL.field.PWEN1 = 1;  /* enable the PWM channel */
    io->pwm->CTL.field.PWEN2 = 1;  /* enable the PWM channel */

    // Init Parameter
    leftParam.gpio = io->gpio;
    leftParam.pwm = io->pwm;
    rightParam.gpio = io->gpio;
    rightParam.pwm = io->pwm;
    imageParam.argc = argc;
    imageParam.argv = argv;

    // Create two threads leftThread and rightThread, and run them in parallel
    pthread_create(&leftThread, NULL, left_wheel_control, (void *)&leftParam);
    pthread_create(&rightThread, NULL, right_wheel_control, (void *)&rightParam);
    pthread_create(&imageThread, NULL, image_process_control, (void *)&imageParam);
    
    while(!Done){
	switch(get_pressed_key()){
	    case 'q':
		Done = true;
		FIFO_INSERT(&left_fifo, 'q');
		FIFO_INSERT(&right_fifo, 'q');
		FIFO_INSERT(&image_process_fifo, 'q');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'q';
		printf("\nHW9> 'q'");
		break;
	    
	    case 's':
		FIFO_INSERT(&left_fifo, 's');
		FIFO_INSERT(&right_fifo, 's');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 's';
		printf("\nHW9> 's'");
		break;
	    
	    case 'w':
		FIFO_INSERT(&left_fifo, 'w');
		FIFO_INSERT(&right_fifo, 'w');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'w';
		printf("\nHW9> 'w'");
		break;
	    
	    case 'x':
		FIFO_INSERT(&left_fifo, 'x');
		FIFO_INSERT(&right_fifo, 'x');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'x';
		printf("\nHW9> 'x'");
		break;
	    
	    case 'i':
		FIFO_INSERT(&left_fifo, 'i');
		FIFO_INSERT(&right_fifo, 'i');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'i';
		printf("\nHW9> 'i'");
		break;
	    
	    case 'j':
		FIFO_INSERT(&left_fifo, 'j');
		FIFO_INSERT(&right_fifo, 'j');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'j';
		printf("\nHW9> 'j'");
		break;
	    
	    case 'a':
		FIFO_INSERT(&left_fifo, 'a');
		FIFO_INSERT(&right_fifo, 'a');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'a';
		printf("\nHW9> 'a'");
		break;
		    
	    case 'd':
		FIFO_INSERT(&left_fifo, 'd');
		FIFO_INSERT(&right_fifo, 'd');
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'd';
		printf("\nHW9> 'd'");
		break;
	    
	    case 'm':
		commandBuffer[0] = commandBuffer[1];
		commandBuffer[1] = 'm';
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
		printf("\nHW9> 'm1'");
	    }
	    currMode = m1Mode;
	}
	else if (commandBuffer[0] == 'm' && commandBuffer[1] == '2'){
	    if (currMode == m2Mode){
		//printf("\nCurrent Mode is already m2 mode");
		continue;
	    }
	    else{
		printf("\nHW9> 'm2'");
	    }
	    currMode = m2Mode;
	    laserLocation[0] = 30;
	    laserLocation[1] = 40;
	}
    }
    
    pthread_join(leftThread, NULL);
    pthread_join(rightThread, NULL);
    pthread_join(imageThread, NULL);
    
    printf("\nQUIT the program\n");

/* when finished, clean the GPIO pins */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
    io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_INPUT;
    io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_INPUT;

  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
