/**************************************************
* File:  hw3Lin2.c 
* Homework 3 Program Part 2
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* 
* This is a LED toggle program
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. 'r' => Red   LED on/off, hit 'r' key to toggle Red   LED on/off
*    2. 'g' => Green LED on/off, hit 'g' key to toggle Green LED on/off
* 	 3. 'b' => Blur  LED on/off, hit 'b' key to toggle Blue  LED on/off
* 	 4. 'o' => Orange LED on/off, hit 'o' key to toggle Orange LED on/off
*    3. only hit 'q' to quit the program
*    4. works without <Enter> key, like game playing
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 22 - with 330 Ohm resistor, Red LED, Green LED, and 220 Ohm in series
*   Green LED on GPIO 23 - with 220 Ohm resistor, Blue LED, Orange LED, and 220 Ohm in series
* 
* RPi4 GPIO pin configuration - OUTPUT and INPUT
*   OUTPUT: Logic high => 3.3V at the pin, up to 10mA current source
*           Logic low  => 0.0V at the pin, up to 10mA current sink
* 
*   >>>>> works without <Enter> key, like game playing <<<<<
* 
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
  bool  done = false;
  int status[4] = {0,0,0,0};
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    
    /* INIT set the pin function to INPUT for GPIO22 */
    /* INIT set the pin function to INPUT for GPIO23 */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
    
	/* set the pin function to INPUT for GPIO 04 - for a switch input (not used here)*/
    //io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04
    
    
    /* set initial output state - OFF */
    GPIO_CLR(io->gpio, 22);
    GPIO_CLR(io->gpio, 23);
    
    printf("\n Press 'r' to toggle the Red   LED\n");
    printf(  " Press 'g' to toggle the Green LED\n");
    printf(  " Press 'b' to toggle the Blue  LED\n");
    printf(  " Press 'o' to toggle the Yellow LED\n");
    printf(  " Press 'q' will exit\n");

	while(!done){
		
		switch(get_pressed_key()){
			case 'q':
				done = true;
				break;
			
			case 'r':
				status[0] = 1 - status[0];
				break;
			
			case 'g':
				status[1] = 1 - status[1];
				break;
			
			case 'b':
				status[2] = 1- status[2];
				break;
			
			case 'o':
				status[3] = 1 - status[3];
				break;
			
			default:
				break;
		}
		// control red and green 
		if (status[0] == 0){
			if (status[1] == 1){
				// red off green light up
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
				GPIO_SET(io->gpio,22);
			}
			else{
				// both red and green are off
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22
			}
		}
		else{
			if (status[1] == 1){
				// red and green are on
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
				//printf("red light up\n");
				GPIO_CLR(io->gpio,22);
				usleep(1);
				//printf("green light up\n");
				GPIO_SET(io->gpio,22);
			}
			else{
				// red on green off
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
				GPIO_CLR(io->gpio,22);
			}
		}
		
		//control blue and orange
		if (status[2] == 0){
			if (status[3] == 1){
				//blur if off and orange is on
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
				GPIO_SET(io->gpio,23);
			}
			else{
				// blue and orange are off
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
			}
		}
		else{
			if (status[3] == 1){
				// blue and orange are on
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
				GPIO_CLR(io->gpio,23);
				usleep(1);
				GPIO_SET(io->gpio,23);
			}
			else{
				// blue is on and orange is off
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
				GPIO_CLR(io->gpio,23);
			}
		}
		
	}
		
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;

    printf("\n Key hit is 'q', quiting loop\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
