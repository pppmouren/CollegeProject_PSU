/**************************************************
* File:  hw0c2dim1s.c 
* Homework 2 Sample Program 1
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a sample LED dimming (pwm) program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* 
*  >>> PWM (dimming) principle <<<
*  Total output power controlled by the RATIO of 
*  ON-time and OFF-time, and cycle repeated fast.
* 
*  This program dim Green LED, 5% light level.
*  - Turn-on  Green LED for 100us* 5= 500us and
*    Turn-off Green LED for 100us*95=9500us  => total 10000us = 10ms => 100/s
*    Program ends with switch press, push switch connected at GPIO 04
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
* Homework 2 Sample Program 1 dim Green LED, on GPIO 13, at 5% light level.
*   Program stops when push-switch is pressed (GPIO 04).  
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
#include "keypress.h"


void DimLevelUnit(int Level, struct io_peripherals *io)
{
      int  ONcount, OFFcount;

      ONcount  =     Level;   // Total output power controlled by the RATIO of 
      OFFcount = 100-Level;   // ON-time and OFF-time, and cycle repeated fast.

      /* create the output pin signal duty cycle, same as Level */
      GPIO_SET(io->gpio, 13 );   /* Turn ON  LED at GPIO 13 */

      while (ONcount > 0)
      {
        usleep( 100 );
        ONcount = ONcount -1;
      }

      GPIO_CLR(io->gpio, 13 );  /* Turn OFF LED at GPIO 13 */

      while (OFFcount > 0)
      {
        usleep( 100 );
        OFFcount = OFFcount -1;
      }
}


int main( void )
{
  struct io_peripherals *io;
  bool  done = false;
  
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
    io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;    //GPIO 04
    
    
    /* set initial output state - OFF */
    GPIO_CLR(io->gpio, 12);
    GPIO_CLR(io->gpio, 13);
    
    printf("\n Press SWITCH to quit\n");
    
    
    while (GPIO_READ(io->gpio, 04) != 0)
     /* Do NOT use (GPIO_READ(io->gpio, 04) == 1) test case because   */
     /* function    GPIO_READ(io->gpio, 04) will return               */
     /* any of 1, 2, 4, 8, 16, ...   values                           */

    {
      for(int i=0; i<10; ++i)
      {
        DimLevelUnit(5, io);  /* dim green LED to 5% light level, and repeat */
      }
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
