/**************************************************
* File:  hw0c1blink2f.c 
* Homework 1 Sample Program 2
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a sample LED blinking program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. Blink Red   LED connected at GPIO 12 pin 
*    2. Blink Green LED connected at GPIO 13 pin 
*    3. Blinking rate set to 0.2 seconds with usleep() function
*    4. Program ends with switch press, push switch connected at GPIO 04
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 12 - with 220 Ohm resistor in series
*   Green LED on GPIO 13 - with 220 Ohm resistor in series
*   Push-switch connected on GPIO 04 - with 10 KOhm pull-up resistor
* 
* RPi4 GPIO pin configuration - OUTPUT and INPUT
*   OUTPUT: Logic high => 3.3V at the pin, up to 10mA current source
*           Logic low  => 0.0V at the pin, up to 10mA current sink
*   INPUT:  Use 10 KOhm pull-up resistor at GPIO 04 pin, and 
*           push-switch at GPIO 04 pin => 
*              if switch is     pressed, GPIO 04 pin reads 0.0V (logic Low)
*              if switch is not pressed, GPIO 04 pin reads 3.3V (logic High)
* 
* Homework 1 Sample Program 2 blinks Red and Green LEDs alternately 
*   at 0.2 second rate, if the push-switch is not pressed.  Program
*   stops if the push-switch is pressed.
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
    
    /* set the pin function to OUTPUT for GPIO 12 - Red   LED */
    /* set the pin function to OUTPUT for GPIO 13 - Green LED */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 12
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 13
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input */
    io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04
    
    
    /* set initial output state - ON */
    GPIO_SET(io->gpio, 12);
    GPIO_SET(io->gpio, 13);
  
    printf("\n Press SWITCH to quit\n");

    while (GPIO_READ(io->gpio, 04) != 0)
     /* Do NOT use (GPIO_READ(io->gpio, 04) == 1) test case because   */
     /* function    GPIO_READ(io->gpio, 04) will return               */
     /* any of 1, 2, 4, 8, 16, ...   values                           */

    {
      GPIO_SET(io->gpio, 12);     // Turn on  Red   LED
      GPIO_CLR(io->gpio, 13);     // Turn off Green LED
      usleep(100*1000);           // sleep for 100 msec.
      GPIO_CLR(io->gpio, 12);     // Turn off Red   LED
      GPIO_SET(io->gpio, 13);     // Turn on  Green LED
      usleep(100*1000);           // sleep for 100 msec.
    }
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;

    printf("\n Now SWITCH pressed, quiting loop\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
