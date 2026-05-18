/**************************************************
* File:  hw5Lin.c 
* Homework 5, car driving
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 5, we will use command to control car driving
 (1) 's': Stop
 (2) 'w': Forward
 (3) 'x': Backward
 (4) 'i': Faster, 5% PWM power increase for each 'i' key hit
 (5) 'j': Slower, 5% PWM power decrease for each 'j' key hit
 (6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition
 (7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition
 (8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key
*Program must end with ‘q’ command and when the program ends.
* 
* GPIO Usage:
* 1. GPIO 12,12 used as PWM channel 1,2 for wheel speed
* 2. GPIO 22,23 used as output for right wheel direction, 00 for stop, 01 for forward, 10 or backward
* 3. GPIO 5,6 usd as output for left wheel direction, 00 for stop, 01 for forward, 10 for backward
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

uint32_t rightPWMLevel = 50;    /* right wheel PWM as duty cycle, set default to 50 */
uint32_t leftPWMLevel = 50;     /* left wheel PWM as duty cycle set default to 50*/
int leftMode[2] = {0,0};
int rightMode[2] = {0,0};


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
	return NULL;
 }



void *right_wheel_control(void *arg)
{
	struct thread_param * param = (struct thread_param *)arg;
    char command;
    bool Done = false;
    
    while(!Done){
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
			}
		}
		param->pwm->DAT2 = rightPWMLevel;		
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
    printf("(1) 's': Stop\n");
    printf("(2) 'w': Forward\n");
    printf("(3) 'x': Backward\n");
    printf("(4) 'i': Faster, 5%% PWM power increase for each 'i' key hit\n");
    printf("(5) 'j': Slower, 5%% PWM power decrease for each 'j' key hit\n");        
    printf("(6) 'a': Left, 15 degree turn for 'a' key hit, smooth transition\n");    
    printf("(7) 'd': Right, 15 degree turn for 'd' key hit, smooth transition\n");
    printf("(8) 'q': Quit, to quit all program proper(without ctrl+c, and without an Enter key\n");
    printf("Program must end with ‘q’ command and when the program ends, all LEDs must be off\n");        
    
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
    io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_OUTPUT;   //GPIO 23
    
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
				printf("\nHW5> 'q'");
				break;
			
			case 's':
				FIFO_INSERT(&left_fifo, 's');
				FIFO_INSERT(&right_fifo, 's');
				printf("\nHW5> 's'");
				break;
			
			case 'w':
				FIFO_INSERT(&left_fifo, 'w');
				FIFO_INSERT(&right_fifo, 'w');
				printf("\nHW5> 'w'");
				break;
			
			case 'x':
				FIFO_INSERT(&left_fifo, 'x');
				FIFO_INSERT(&right_fifo, 'x');
				printf("\nHW5> 'x'");
				break;
			
			case 'i':
				FIFO_INSERT(&left_fifo, 'i');
				FIFO_INSERT(&right_fifo, 'i');
				printf("\nHW5> 'i'");
				break;
			
			case 'j':
				FIFO_INSERT(&left_fifo, 'j');
				FIFO_INSERT(&right_fifo, 'j');
				printf("\nHW5> 'j'");
				break;
			
			case 'a':
				FIFO_INSERT(&left_fifo, 'a');
				FIFO_INSERT(&right_fifo, 'a');
				printf("\nHW5> 'a'");
				break;
				
			case 'd':
				FIFO_INSERT(&left_fifo, 'd');
				FIFO_INSERT(&right_fifo, 'd');
				printf("\nHW5> 'd'");
				break;
			
			default:
				usleep(50);
				break;
		}
	}
	
	pthread_join(leftThread, NULL);
	pthread_join(rightThread, NULL);
	
	printf("QUIT the program\n");
    
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
