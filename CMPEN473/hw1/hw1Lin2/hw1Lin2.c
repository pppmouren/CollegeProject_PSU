/**************************************************
* File:  hw1Lin2.c 
* Homework 1 Program 2
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a sample LED on/off program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. 'r' => Red   LED on/off, hit 'r' key to toggle Red   LED on/off
*    2. 'g' => Green LED on/off, hit 'g' key to toggle Green LED on/off
* 	 3. 'b' => Blue  LED on/off, hit 'b' key to toggle Blue  LED on/off
* 	 4. 'o' => Orange LED on/off, hit 'o' key to toggle Orange LED on/off
*    5. hit any other key to quit
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 12 - with 33O Ohm resistor in series
*   Green LED on GPIO 13 - with 330 Ohm resistor in series
* 	Blue  LED on GPIO 22 - with 330 Ohm resistor in series
* 	Orange LED on GPIO 23 - with 330 Ohm resistor in series
* 
* RPi4 GPIO pin configuration - OUTPUT and INPUT
*   OUTPUT: Logic high => 3.3V at the pin, up to 10mA current source
*           Logic low  => 0.0V at the pin, up to 10mA current sink
* 
* Homework 1 Program 2 turns on/off Red, Green, Blue, Orange LEDs by keyboard.
*   Program stops by hitting any key other than 'r', 'g', 'b', 'o' keys.
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



int get_pressed_key(void)
{
  int ch;

  system("stty -icanon"); //disable buffer
  ch = getchar();
  
 
  return ch;
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
    
    /* set the pin function to OUTPUT for GPIO12 */
    /* set the pin function to OUTPUT for GPIO13 */
    /* set the pin function to OUTPUT for GPIO22 */
    /* set the pin function to OUTPUT for GPIO23 */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 12
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_OUTPUT; 	 //GPIO 13
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input (not used here)*/
    //io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04
    
    
    /* set initial output state - ON */
    GPIO_SET(io->gpio, 12);
    GPIO_SET(io->gpio, 13);
    GPIO_SET(io->gpio, 22);
    GPIO_SET(io->gpio, 23);
    
    printf("\nAll LEDs are initialized to turn on\n");
    printf("Press 'r' to toggle the Red LED\n");
    printf("Press 'g' to toggle the Green LED\n");
    printf("Press 'b' to toggle the Blue LED\n");
    printf("Press 'o' to toggle the Orange LED\n");
    printf("Press any other key will exit\n");
    
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
          
        case 'b':
          if (GPIO_READ(io->gpio, 22) == 0)
          {
            GPIO_SET(io->gpio, 22);
          }
          else
          {
            GPIO_CLR(io->gpio, 22);
          }
          break;
          
        case 'o':
          if (GPIO_READ(io->gpio, 23) == 0)
          {
            GPIO_SET(io->gpio, 23);
          }
          else
          {
            GPIO_CLR(io->gpio, 23);
          }
          break;

		 //case 10:               /* it is Line Feed, New Line, or Enter Key character */
         // usleep(100*1000);    /* numbers in micro seconds (100msec.)  */
         // break; 

        default:
          done = true;
          break;
      }
    } while (!done);
    
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;

    printf("\n Key hit is not 'r', 'g', 'b', or 'o' key, quiting loop\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}
