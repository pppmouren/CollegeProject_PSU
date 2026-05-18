/**************************************************
* File:  hw0c3pth1blin.c 
* Homework 4, pthread Sample 1 Program 
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a Homework 4, pthread Sample 1 Program, simple LED blinking. 
*  There are 4 sample programs to show, in step by step how to 
*    use pthread function.  Then there is 1 more sample program to show
*    how to write a good program with pthread function. 
*  This is the 1st sample program, simply showing LED blinking.
*    Note the basic GPIO parameter use.
*  This program:
*    Blink  Red    LED 1 at GPIO 12, blink at 5Hz rate
*    Blink  Green  LED 2 at GPIO 13, blink at 5Hz rate
*    QUIT this program by 'q' key hits
*    Any other keys are ignored
*    Works without <Enter> key
* 
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red    LED on GPIO 12 - with 220 Ohm resistor in series
*   Green  LED on GPIO 13 - with 220 Ohm resistor in series
*   Blue   LED on GPIO 22 - with 220 Ohm resistor in series
*   Orange LED on GPIO 23 - with 220 Ohm resistor in series
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
    
    /* set the pin function to OUTPUT for GPIO 22 - Red   LED */
    /* set the pin function to OUTPUT for GPIO 23 - Green LED */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input */
    io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04; NOT used here
    
    /* set initial output state - OFF */
    GPIO_CLR(io->gpio, 12);
    GPIO_CLR(io->gpio, 13);
    GPIO_CLR(io->gpio, 22);
    GPIO_CLR(io->gpio, 23);
    
    printf("\n Press 'q' to QUIT this program\n");
    printf(  " Any other key press will be ignored\n");
    
    while (!done)
    {
      GPIO_SET(io->gpio, 12);  // on
      GPIO_CLR(io->gpio, 13);  // off

      usleep(100*1000);    /* numbers in micro seconds (100msec.)  */

      GPIO_CLR(io->gpio, 12);  // off
      GPIO_SET(io->gpio, 13);  // on

      usleep(90*1000);    /* numbers in micro seconds (90+10=100msec.)  */
      
      switch (get_pressed_key())
      {
        case 'q':
          done = true;
          break;

        default:         /* it is Line Feed, New Line, or Enter Key character */
          usleep(10*1000); /* do nothing, numbers in micro seconds (100msec.) */
          break;
      }
    }
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;

    printf("\n 'q' key pressed, quiting loop\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
