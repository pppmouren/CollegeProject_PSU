/**************************************************
* File:  hw4Lin1.c 
* Homework 4, Light dimming pthread Program 
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 4, four LED blinking in 2 threads. 
*    Note the basic pthread parameter, pthread_create, and pthread_join 
*  This program:
*    Blink  Red    LED at GPIO 22, from 0% light level to 12% light level in 2 seconds
*    Blink  Green  LED at GPIO 22, from 100% light level to 88% light level in 2 seconds
* 	 Blink  Blue   LED at GPIO 23, from 3% light level to 23% light level in 1.5 seconds
* 	 Blink  Orange LED at GPIO 23, from 97% light level to 77% 	light level in 1.5 seconds
*    Blink  Red    LED at GPIO 22, from 12% light level to 0% light level in 2 seconds
*    Blink  Green  LED at GPIO 22, from 88% light level to 100% light level in 2 seconds
* 	 Blink  Blue   LED at GPIO 23, from 23% light level to 3% light level in 1.5 seconds
* 	 Blink  Orange LED at GPIO 23, from 77% light level to 97% 	light level in 1.5 seconds
* 	 Repeat the pattern forever
* 	 Using 'q' key to terminate the program
*    LEDs blink at same time but independently
* 
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red    LED on GPIO 22 - with 330 Ohm resistor in series
*   Green  LED on GPIO 22 - with 220 Ohm resistor in series
*   Blue   LED on GPIO 23 - with 220 Ohm resistor in series
*   Orange LED on GPIO 23 - with 220 Ohm resistor in series
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


int dimPeriodCount_12 = 15; // 0% to 12% dimming for 2s, each dim level will take 153ms, which will have 15 complete cycles
int dimPeriodCount_20 = 8; // 3% to 23% dimming for 1.5s, each dim level will take 71.4ms, which will ahve 7 complete cycles                                                                                                                                                                           
bool Done = false;

struct blink_thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
};

/*
 Function: DimLevelUnit is the most low level dim part which sleep 100us * count, the total count is 100 at max. one cycle is 10ms
*/
void DimLevelUnit(int count){
	int i = 0;
	while(i<count){
		usleep(100); //sleep 100us
		i++;
	}
}


void *RedGreenBlink(void *arg)
{
	struct blink_thread_param * param = (struct blink_thread_param *)arg;
	int maxCount = 100; 
	int initDimLevel = 0;
	int maxDimLevel = 12;
	int currentOnLevel = initDimLevel;
	
	while(1){
		// Increase Dim Level for Red from 0% to 12%
		while(currentOnLevel <= maxDimLevel){
			//Each Dim Level we need to Loop 15 times to make the Total Dim Period is 2s
			for(int i=0; i<dimPeriodCount_12; i++){
				if(currentOnLevel != 0){
					GPIO_CLR(param->gpio, 22); //turn on red, turn off green
					DimLevelUnit(currentOnLevel);
				}
				GPIO_SET(param->gpio,22);
				DimLevelUnit(maxCount - currentOnLevel);
			}
			currentOnLevel++;
		}
		currentOnLevel--; //Note: CurrentOnlevel = 13 at this time;
		currentOnLevel--;
		// Decrease Dim Level for Red from 11% to 0%, and go back 
		while(currentOnLevel >= initDimLevel + 1){
			for(int j=0; j<dimPeriodCount_12; j++){
				GPIO_CLR(param->gpio,22); //turn on red, turn off green
				DimLevelUnit(currentOnLevel);
				GPIO_SET(param->gpio,22);
				DimLevelUnit(maxCount - currentOnLevel);
			}
			currentOnLevel--;
		}
	}
}


void *BlueOrangeBlink(void *arg)
{
	struct blink_thread_param * param = (struct blink_thread_param *)arg;
	int maxCount = 100;
	int initDimLevel = 3;
	int maxDimLevel = 23;
	int currentOnLevel = initDimLevel;
	
	while(1){
		// Increase Dim Level of Blue LED from 3% to 23%
		while(currentOnLevel <= maxDimLevel){
			for(int i=0; i < dimPeriodCount_20; i++){
				GPIO_CLR(param->gpio,23); // turn on blue and turn off orange
				DimLevelUnit(currentOnLevel);
				GPIO_SET(param->gpio,23); // turn off blue and turn on orange
				DimLevelUnit(maxCount - currentOnLevel);
			}
			currentOnLevel++;
		}
		currentOnLevel--; //Note: currentOnLevel = 24 at this time
		currentOnLevel--;
		// Decrease Dim Level of Blue from 22% to 3%
		while(currentOnLevel >= initDimLevel + 1){
			for(int j=0; j < dimPeriodCount_20; j++){
				GPIO_CLR(param->gpio,23); // turn on blue and turn off orange
				DimLevelUnit(currentOnLevel);
				GPIO_SET(param->gpio,23); // turn off blue and turn on orange
				DimLevelUnit(maxCount - currentOnLevel);
			}
			currentOnLevel--;
		}
	}
}

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
  // this is keyboard reading without 'Enter' key and in
  // non-blocking mode.  This function returns single key
  // value (ASCII code), and returns -1 if no key was hit
}

int main( void )
{
  struct io_peripherals *io;
  
  pthread_t  t1R_G;
  pthread_t  t2B_O;
  struct blink_thread_param  redGreenParam;
  struct blink_thread_param  blueOrangeParam;
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    printf("Blink  Red    LED at GPIO 22, from 0%% light level to 12%% light level in 2 seconds\n");
	printf("Blink  Green  LED at GPIO 22, from 100%% light level to 88%% light level in 2 seconds\n");
	printf("Blink  Blue   LED at GPIO 23, from 3%% light level to 23%% light level in 1.5 seconds\n");
 	printf("Blink  Orange LED at GPIO 23, from 97%% light level to 77%% light level in 1.5 seconds\n");
    printf("Blink  Red    LED at GPIO 22, from 12%% light level to 0%% light level in 2 seconds\n");
    printf("Blink  Green  LED at GPIO 22, from 88%% light level to 100%% light level in 2 seconds\n");
 	printf("Blink  Blue   LED at GPIO 23, from 23%% light level to 3%% light level in 1.5 seconds\n");
 	printf("Blink  Orange LED at GPIO 23, from 77%% light level to 97%% light level in 1.5 seconds\n");
 	printf("Repeat the pattern forever\n");
 	printf("Using 'q' key to terminate the program\n");
    
    
    /* set the pin function to OUTPUT for GPIO 22 - Red and Green  LED */
    /* set the pin function to OUTPUT for GPIO 23 - Blue and Orange LED */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input */
    //io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04; NOT used here    
    
    // Init Parameter
    redGreenParam.gpio = io->gpio;
    blueOrangeParam.gpio = io->gpio;
    
    // Create two threads t1red and t2green, and run them in parallel
    pthread_create(&t1R_G, NULL, RedGreenBlink,   (void *)&redGreenParam);
    pthread_create(&t2B_O, NULL, BlueOrangeBlink, (void *)&blueOrangeParam);
    
    while(!Done){
		switch(get_pressed_key()){
			case 'q':
				Done = true;
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;
				pthread_kill(t1R_G, SIGTERM);
				pthread_kill(t2B_O, SIGTERM);
				// Wait to finish both t1red and t2green threads
				pthread_join(t1R_G, NULL);
				pthread_join(t2B_O, NULL);
				break;
			
			default:
				usleep(50);
				break;
		}
	}
	
	printf("QUIT the program\n");
    
    /* main task finished  */
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
