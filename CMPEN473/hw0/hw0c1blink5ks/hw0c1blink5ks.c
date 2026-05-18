/**************************************************
* File:  hw0c1blink5ks.c 
* Homework 1 Sample Program 5
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a sample LED on/off program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. 'r' => Red   LED on/off, hit 'r' key to toggle Red   LED on/off
*    2. 'g' => Green LED on/off, hit 'g' key to toggle Green LED on/off
*    3. 'q' => QUIT program
*    4. any other keys are ignored
*    5. works without <Enter> key, like game playing
*    6. use 'include' to create keyboard function - as an example 
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
* Homework 1 Sample Program 5 turns on/off Red and Green LEDs by keyboard.
*   Program stops by hitting 'q' key.  
*   Any other key than 'r', 'g', and 'q' keys are ignored.
*   >>>>> works without <Enter> key, like game playing <<<<<
*   This program use 'include' to create keyboard function - as an example 
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
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input */
    io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04
    
    
    /* set initial output state - ON */
    GPIO_SET(io->gpio, 12);
    GPIO_SET(io->gpio, 13);
    
    printf("\n Press 'r' to toggle the Red   LED\n");
    printf(  " Press 'g' to toggle the Green LED\n");
    printf(  " Press 'q' to QUIT this program\n");
    printf(  " Any other key press will ignored\n");
    
    do
    {
      switch (get_pressed_key())
      {
        case 'r':
          if (GPIO_READ(io->gpio, 12) == 0)
          {
            GPIO_SET(io->gpio, 12);
          }
          else
          {
            GPIO_CLR(io->gpio, 12);
          }
          break;

        case 'g':
          if (GPIO_READ(io->gpio, 13) == 0)
          {
            GPIO_SET(io->gpio, 13);
          }
          else
          {
            GPIO_CLR(io->gpio, 13);
          }
          break;
          
        case 'q':
          done = true;
          break;

        default:               /* it is Line Feed, New Line, or Enter Key character */
          usleep(100*1000);    /* numbers in micro seconds (100msec.)  */
          break;
      }
    } while (!done);
    
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;

    printf("\n 'q' key pressed, quiting loop\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
