/**************************************************
* File:  hw4Lin2.c 
* Homework 4, Light dimming pthread Program part 2
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a Homework 4, we will use command to control the LED lights as follows
 (1) ‘i’ command – increase Red LED light level by 5%,
 (2) ‘j’ command – decrease Red LED light level by 5%,
 (3) ‘r’ command – set Red LED light level to 0%,
 (4) ‘h’ command – set Red LED light level to 50%,
 (5) ‘m’ command – set Red LED light level to 100%,
 (6) ‘w’ command – turn-on Orange LED for 2 seconds,
 (7) ‘x’ command – turn-on Blue LED for 2 seconds,
 (8) ‘s’ command – turn-on both Orange LED and Blue LED for 3 seconds,
 (9) ‘q’ command – quit program.
*Program must end with ‘q’ command and when the program ends, all LEDs must be off.
* 
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red    LED on GPIO 22 - with 330 Ohm resistor in series
*   Green  LED on GPIO 22 - with 220 Ohm resistor in series
*   Blue   LED on GPIO 23 - with 220 Ohm resistor in series
*   Orange LED on GPIO 23 - with 220 Ohm resistor in series
* 	Red and Green are connected in series and GPIO 22 is connected in between
* 	Blue and Orange are connected in series and GPIO 23 is connected in between
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

#define FIFO_LENGTH 2048

int red_greeb_status = 0;

FIFO_TYPE(int, FIFO_LENGTH, MyFIFO);

struct MyFIFO my_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};

struct blink_thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
};

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
	int currentOnLevel = 0;
	int maxCount = 100;
    
    param->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    
    while(1){
		switch(red_greeb_status){
			// increase RED LED light level by 5%
			case 1:
				if (currentOnLevel != 100){
					currentOnLevel += 5;
					printf("currentOnLevel = %d\n", currentOnLevel);
				}
				else{
					printf("RED LED light level is already 100%%\n");
				}
				red_greeb_status = 0;
				break;
				
			// decrease RED LED light level by 5%
			case 2:
				if (currentOnLevel != 0){
					currentOnLevel -= 5;
					printf("currentOnLevel = %d\n", currentOnLevel);
				}
				else{
					printf("RED LED light level is already 0%%");
				}
				red_greeb_status = 0;
				break;
			
			// set RED LED light level to 0%	
			case 3:
				currentOnLevel = 0;
				printf("currentOnLevel = %d\n", currentOnLevel);
				red_greeb_status = 0;
				break;
			
			// set RED LED light level to 50%	
			case 4:
				currentOnLevel = 50;
				printf("currentOnLevel = %d\n", currentOnLevel);
				red_greeb_status = 0;
				break;
			
			//set RED LED light level to 100%
			case 5:
				currentOnLevel = 100;
				printf("currentOnLevel = %d\n", currentOnLevel);
				red_greeb_status = 0;
				break;
			
			default:
				usleep(10);
				break;
		}
		
		//Light RED LED based on current dimming level
		GPIO_CLR(param->gpio, 22);
		DimLevelUnit(currentOnLevel);
		GPIO_SET(param->gpio, 22);
		DimLevelUnit(maxCount - currentOnLevel);
    }
}


void *BlueOrangeBlink(void *arg)
{
    struct blink_thread_param * param = (struct blink_thread_param *)arg;
    int currentStatus = 0;
    
    while(1){
		while(!FIFO_EMPTY(&my_fifo)){
			FIFO_REMOVE(&my_fifo, &currentStatus);
			switch(currentStatus){
				// turn on orange led for 2 seconds
				case 6:
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
					GPIO_SET(param->gpio, 23);
					sleep(2);
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
					break;
					
				// turn on Blue LED for 2 seconds
				case 7:
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
					GPIO_CLR(param->gpio, 23);
					sleep(2);
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
					break;
				
				// turn on both Orange LED and Blue LED for 3 seconds
				case 8:
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
					int totalCount = 180; //one for loop will take 20ms, 3s need 150 loops
					for(int i = 0; i < totalCount; i++){
						GPIO_CLR(param->gpio, 23);
						DimLevelUnit(50);
						GPIO_SET(param->gpio, 23);
						DimLevelUnit(50);
					} 
					param->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
					break;
				
				default:
					usleep(10);
					break;
			}	
		}
	}
}


int main( void )
{
  struct io_peripherals *io;
  
  pthread_t  t1R_G;
  pthread_t  t2B_O;
  bool Done = false;
  struct blink_thread_param  redGreenParam;
  struct blink_thread_param  blueOrangeParam;
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    printf("This Program aims to control the LED lights by user command as follows\n");
    printf("(1) ‘i’ command – increase Red LED light level by 5%%\n");
    printf("(2) ‘j’ command – decrease Red LED light level by 5%%\n");
    printf("(3) ‘r’ command – set Red LED light level to 0%%\n");
    printf("(4) ‘h’ command – set Red LED light level to 50%%\n");
    printf("(5) ‘m’ command – set Red LED light level to 100%%\n");        
    printf("(6) ‘w’ command – turn-on Orange LED for 2 seconds\n");    
    printf("(7) ‘x’ command – turn-on Blue LED for 2 seconds\n");
    printf("(8) ‘s’ command – turn-on both Orange LED and Blue LED for 3 seconds\n");
    printf("(9) ‘q’ command – quit program\n");
    printf("Program must end with ‘q’ command and when the program ends, all LEDs must be off\n");        
    /* set the pin function to INPUT for GPIO 22 - Red and Green  LED */
    /* set the pin function to INPUT for GPIO 23 - Blue and Orange LED */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
    
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
			
			case 'i':
				red_greeb_status = 1;
				break;
			
			case 'j':
				red_greeb_status = 2;
				break;
			
			case 'r':
				red_greeb_status = 3;
				break;
			
			case 'h':
				red_greeb_status = 4;
				break;
			
			case 'm':
				red_greeb_status = 5;
				break;
			
			case 'w':
				FIFO_INSERT(&my_fifo, 6);
				break;
				
			case 'x':
				FIFO_INSERT(&my_fifo, 7);
				break;
			
			case 's':
				FIFO_INSERT(&my_fifo, 8);
				break;
			
			default:
				usleep(50);
				break;
		}
	}
	
	printf("QUIT the program\n");
    
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
