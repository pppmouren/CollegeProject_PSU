/**************************************************
* File:  hw6Lin.c 
* Homework 6, manual car driving and self black line tracing
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 6, we will use command to control car driving
* either 'm1' mode for manual driving, or 'm2' mode for self black line tracing
* 1. "m1" mode:
 (1) 's': Stop
 (2) 'w': Forward
 (3) 'x': Backward
 (4) 'i': Faster, 5% PWM power increase for each 'i' key hit
 (5) 'j': Slower, 5% PWM power decrease for each 'j' key hit
 (6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition
 (7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition
 (8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key
 (9) 'm2': mode change to m2

* 2. "m2" mode:
 (1) 's': Stop to pause the line tracing until next forward command
 (2) 'w': to start the line tracing
 (3) 'q': to quit the program
 (4) 'm1': mode change to m1
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
* 1. always display on a terminal window the prompt ‘Hw6m1> ’ or ‘Hw6m2> ’ and display
*	 the user key command input as they enter (echo print)
* 2. m1 mode:
* 		2.1. For the ' w ' and ' x ' commands, be sure to have your car stop for 0.01 second 
* 			(ramp down, stop,reverse ramp up) if the car is currently in opposite motion. That
* 			is, avoid moving the car to go backward immediately if it is currently going forward
* 		2.2. One should consider the fact that PWM level less than 35% may not start the motor 
* 			(varies from one motor to another motor). Moreover, the available speed range may 
*			be 30% to 100%. If the left and right motor characteristics are different, write your 
* 			program to compensate - left and right motor speed balancing (may use look-up tables).
* 3. m2 mode:
* 		3.1. Car autonomously follows a black line on the white board
* 		3.2. car moves at full speed but it may move slower on the curbed line, never stopping 
* 			 or jerking in any case
* 		3.3. able to overcome small line discontinuity and misalignments, for the track is drawn 
* 			 on the multiple white boards
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

#define PWM_RANGE 100
#define FIFO_LENGTH 2048
#define stopTurnTime 150*1000
#define onMovingTurnTime 240*1000
#define m1Mode 1
#define m2Mode 2

uint32_t rightPWMLevel = 50;    /* right wheel PWM as duty cycle, set default to 50 */
uint32_t leftPWMLevel = 50;     /* left wheel PWM as duty cycle set default to 50*/
int leftMode[2] = {0,0};
int rightMode[2] = {0,0};
int currMode = m1Mode;
char commandBuffer[2] = {'N','N'};

//define two fifos, one for left, one for right
FIFO_TYPE(char, FIFO_LENGTH, MYFIFO);
struct MYFIFO left_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
struct MYFIFO right_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};

struct thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
  volatile struct pwm_register *pwm; // PWM port registers
};

// get pressed key without enter
int get_pressed_key(void){
  struct termios  original_attributes;
  struct termios  modified_attributes;
  long   oldf, newf;
  int    ch;

  /* Make input available immediately, without 'enter' key,       */
  /* no input processing, disable line editing, noncanonical mode */
  tcgetattr( STDIN_FILENO, &original_attributes );
  modified_attributes = original_attributes;
  modified_attributes.c_lflag &= ~(ICANON | ECHO);
  modified_attributes.c_cc[VMIN] = 1;
  modified_attributes.c_cc[VTIME] = 0;
  tcsetattr( STDIN_FILENO, TCSANOW, &modified_attributes );

  /* Save the mode of STDIN, then      <use fcntl>   */
  /*   change the mode to NONBLOCK, then get char,   */
  /*   and then restore the STDIN mode back.         */
  /* Key character show on the terminal (echo print) */
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  newf = oldf | O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, newf);
  
  ch = getchar();                       //get the character
  
  fcntl(STDIN_FILENO, F_SETFL, oldf);   //restore the STDIN mode back

  //reset the input to the orginal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &original_attributes );

  if (ch != -1) {                    // '0' <= valid key range <= 'z'
    if (ch > 47) {                   // ignore all other keys
      if (ch < 123) {
        } //printf( "\n ch= %d\n", ch);}
      else {ch = -1;}
    }
    else {ch = -1;}
  }

	//printf("ch = %d", ch);

  return ch;
}

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
			// set the motor to full speed in this mode
			//param->pwm->DAT1 = PWM_RANGE;
			
			while(!FIFO_EMPTY(&left_fifo)){
				FIFO_REMOVE(&left_fifo, &command);
				switch(command){
					case 's':
						// pause the line tracing until next forward command
						param->pwm->DAT1 = 0;
						usleep(100*1000);
						// set mode to stop
						GPIO_CLR(param->gpio, 5);
						GPIO_CLR(param->gpio, 6);
						leftMode[0] = 0;
						leftMode[1] = 0;
						break;
					
					case 'w':
						// to start line tracing
						if(leftMode[0] == 0 && leftMode[1] == 1){
							param->pwm->DAT2 = 50;
						}
						else if (leftMode[0] == 0 && leftMode[1] == 0){
							param->pwm->DAT1 = 25;
							usleep(100*1000);
							param->pwm->DAT1 = 50;
						}
						else if (leftMode[0] == 1 && leftMode[1] == 0){
							//slow down first
							param->pwm->DAT1 = 0;
							usleep(100*1000);
							//set direction to forward
							GPIO_CLR(param->gpio, 5);
							GPIO_SET(param->gpio, 6);
							//speed up
							param->pwm->DAT1 = 25;
							usleep(100*1000);
							param->pwm->DAT1 = 50;
						}	
						// set direction to forward
						GPIO_CLR(param->gpio, 5);
						GPIO_SET(param->gpio, 6);
						leftMode[0] = 0;
						leftMode[1] = 1;
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
			
			// if both sensors detected black line
			if (GPIO_READ(param->gpio, 24) != 0 && GPIO_READ(param->gpio,25) != 0){
				// turn right
				param->pwm->DAT1 = 100;
				usleep(onMovingTurnTime);
			} 
			// if left not detect black, but right detect black line
			else if (GPIO_READ(param->gpio, 24) ==0 && GPIO_READ(param->gpio,25) != 0){
				// turn right
				param->pwm->DAT1 = 100;
				usleep(onMovingTurnTime);
			}
			//if left detected black line, but right not detect black line
			else if (GPIO_READ(param->gpio,24) != 0 && GPIO_READ(param->gpio,25) == 0){
				// turn left
				param->pwm->DAT1 = 0;
				usleep(onMovingTurnTime);
			}
			// if both sensors not detect black line
			else{
				// go straight
				param->pwm->DAT1 = 50;
				//usleep(10*1000);
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
							param->pwm->DAT2 = rightPWMLevel;
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
							param->pwm->DAT2 = rightPWMLevel;
							printf("\ncurrent right PWM power = %d, forward", rightPWMLevel);
						}
						else if (rightMode[0] == 0 && rightMode[1] == 1){
							// forward again
							param->pwm->DAT2 = rightPWMLevel;;
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
							param->pwm->DAT2 = rightPWMLevel;
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
							param->pwm->DAT2 = rightPWMLevel;
							printf("\ncurrent right PWM power = %d, backward", rightPWMLevel);
						}
						else if (rightMode[0] == 1 && rightMode[1] == 0){
							// backward again
							param->pwm->DAT2 = rightPWMLevel;;
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
				param->pwm->DAT2 = rightPWMLevel;	
		}	
	
		else if (currMode == m2Mode){
			while(!FIFO_EMPTY(&right_fifo)){
				FIFO_REMOVE(&right_fifo, &command);
				switch(command){
					case 's':
						// pause the line tracing until next forward command
						param->pwm->DAT2 = 0;
						usleep(100*1000);
						// set mode to stop
						GPIO_CLR(param->gpio, 22);
						GPIO_CLR(param->gpio, 23);
						rightMode[0] = 0;
						rightMode[1] = 0;
						break;
						
					case 'w':
						// to start line tracing
						if (rightMode[0] == 0 && rightMode[1] == 1){
							param->pwm->DAT2 = 50;
						}
						else if (rightMode[0] == 0 && rightMode[1] == 0){
							param->pwm->DAT2 = 25;
							usleep(100*1000);
							param->pwm->DAT2 = 50;
						}
						else if (rightMode[0] == 1 && rightMode[1] == 0){
							//slow down first
							param->pwm->DAT2 = 0;
							usleep(100*1000);
							//set direction to forward
							GPIO_CLR(param->gpio, 22);
							GPIO_SET(param->gpio, 23);
							//speed up
							param->pwm->DAT2 = 25;
							usleep(100*1000);
							param->pwm->DAT2 = 50;
						}
						// set direction to forward
						GPIO_CLR(param->gpio, 22);
						GPIO_SET(param->gpio, 23);
						rightMode[0] = 0;
						rightMode[1] = 1;
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
			// if both sensors detected black line
			if (GPIO_READ(param->gpio, 24) != 0 && GPIO_READ(param->gpio,25) != 0){
				// turn right
				param->pwm->DAT2 = 0;
				usleep(onMovingTurnTime);
			} 
			// if left not detect black, but right detect black line
			else if (GPIO_READ(param->gpio, 24) ==0 && GPIO_READ(param->gpio,25) != 0){
				// turn right
				param->pwm->DAT2 = 0;
				usleep(onMovingTurnTime);
			}
			//if left detected black line, but right not detect black line
			else if (GPIO_READ(param->gpio,24) != 0 && GPIO_READ(param->gpio,25) == 0){
				// turn left
				param->pwm->DAT2 = 100;
				usleep(onMovingTurnTime);
			}
			// if both sensors not detect black line
			else{
				// go straight
				param->pwm->DAT2 = 50;
				//usleep(10*1000);
			}	
		}
	}
	return NULL;
}


int main( void )
{
  struct io_peripherals *io;
  
  pthread_t  leftThread;
  pthread_t  rightThread;
  bool Done = false;
  struct thread_param  leftParam;
  struct thread_param  rightParam;
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    printf("This Program aims to control car driving by commands\n");
    printf("There are two modes for use, 'm1': manual driving mode; 'm2': self black line tracing mode\n");
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
    printf("(1) 's': to pause the line tracing until next Forward command\n");
    printf("(2) 'w': to start the line tracing\n");
    printf("(3) 'q': to quit all program\n");
    printf("(4) 'm1': change mode to m1\n");
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
    
    // Create two threads leftThread and rightThread, and run them in parallel
    pthread_create(&leftThread, NULL, left_wheel_control, (void *)&leftParam);
    pthread_create(&rightThread, NULL, right_wheel_control, (void *)&rightParam);
    
	while(!Done){
		switch(get_pressed_key()){
			case 'q':
				Done = true;
				FIFO_INSERT(&left_fifo, 'q');
				FIFO_INSERT(&right_fifo, 'q');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'q';
				printf("\nHW6> 'q'");
				break;
			
			case 's':
				FIFO_INSERT(&left_fifo, 's');
				FIFO_INSERT(&right_fifo, 's');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 's';
				printf("\nHW6> 's'");
				break;
			
			case 'w':
				FIFO_INSERT(&left_fifo, 'w');
				FIFO_INSERT(&right_fifo, 'w');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'w';
				printf("\nHW6> 'w'");
				break;
			
			case 'x':
				FIFO_INSERT(&left_fifo, 'x');
				FIFO_INSERT(&right_fifo, 'x');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'x';
				printf("\nHW6> 'x'");
				break;
			
			case 'i':
				FIFO_INSERT(&left_fifo, 'i');
				FIFO_INSERT(&right_fifo, 'i');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'i';
				printf("\nHW6> 'i'");
				break;
			
			case 'j':
				FIFO_INSERT(&left_fifo, 'j');
				FIFO_INSERT(&right_fifo, 'j');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'j';
				printf("\nHW6> 'j'");
				break;
			
			case 'a':
				FIFO_INSERT(&left_fifo, 'a');
				FIFO_INSERT(&right_fifo, 'a');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'a';
				printf("\nHW6> 'a'");
				break;
				
			case 'd':
				FIFO_INSERT(&left_fifo, 'd');
				FIFO_INSERT(&right_fifo, 'd');
				commandBuffer[0] = commandBuffer[1];
				commandBuffer[1] = 'd';
				printf("\nHW6> 'd'");
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
				printf("\nHW6> 'm1'");
			}
			currMode = m1Mode;
		}
		else if (commandBuffer[0] == 'm' && commandBuffer[1] == '2'){
			if (currMode == m2Mode){
				//printf("\nCurrent Mode is already m2 mode");
				continue;
			}
			else{
				printf("\nHW6> 'm2'");
			}
			currMode = m2Mode;
		}
	}
	
	pthread_join(leftThread, NULL);
	pthread_join(rightThread, NULL);
	
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

