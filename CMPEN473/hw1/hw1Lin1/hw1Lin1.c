/**************************************************
* File:  hw1Lin1.c 
* Homework 1 Program 1
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* Date: 01/15/2024
* 
* This is a LED blinking program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. Red LED connected at GPIO 12 pin, Green LED connected at GPIO 13 Pin
* 		Blue LED connected at GPIO 22 pin, Orange LED connected at GPIO 23 pin
*    2. Blinking LEDs in sequence, Red, Green, Blue, Orange and then repeat
*    3. Program runs indefinitely, until 'control c' key hit
*    4. Use sleep() function for sleep ingeter seconds
* 	 5. Use usleep() function for faster rate
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 12 - with 33O Ohm resistor in series
*   Green LED on GPIO 13 - with 330 Ohm resistor in series
* 	Blue  LED on GPIO 22 - with 330 Ohm resistor in series
* 	Orange LED on GPIO 23 - with 330 Ohm resistor in series
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

int main( void )
{
  struct io_peripherals *io;

  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    
    /* set the pin function to OUTPUT for GPIO12 */
    /* set the pin function to OUTPUT for GPIO13 */
    /* set the pin function to OUTPUT for GPIO22 */
    /* set the pin function to OUTPUT for GPIO23 */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 12
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_OUTPUT; 	 //GPIO 13
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    printf("\n Hit 'ctl c' to quit \n");
    
    // init all LEDs to Off 
    GPIO_CLR(io->gpio, 12);
    GPIO_CLR(io->gpio, 13);
    GPIO_CLR(io->gpio, 22);
    GPIO_CLR(io->gpio, 23);
    
    while (1)   // Loop forever
    { 
	  //uncommand sleep() will blink LEDs in 2s
	  //flash the Red LED for 0.2s
      GPIO_SET(io->gpio, 12);
      //sleep(1);
      usleep(100*1000);
      GPIO_CLR(io->gpio, 12);
      //sleep(1);
      usleep(100*1000);
      
      //flash the Green LED for 0.2s
      GPIO_SET(io->gpio, 13);
      //sleep(1);
      usleep(100*1000);
      GPIO_CLR(io->gpio, 13);
      //sleep(1);
      usleep(100*1000);
      
      //flash the Blue LED for 0.2s
      GPIO_SET(io->gpio, 22);
      //sleep(1);
      usleep(100*1000);
      GPIO_CLR(io->gpio, 22);
      //sleep(1);
      usleep(100*1000);
      
      //flash the Orange LED for 0.2s
      GPIO_SET(io->gpio, 23);
      //sleep(1);
      usleep(100*1000);
      GPIO_CLR(io->gpio, 23);
      //sleep(1);
      usleep(100*1000);
    }

  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n main function done\n");   // this line never executed - ctl c

  return 0;
}
