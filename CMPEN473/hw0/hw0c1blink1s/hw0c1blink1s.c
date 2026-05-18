/**************************************************
* File:  hw0c1blink1s.c 
* Homework 1 Sample Program 1
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a sample LED blinking program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. Blink Red LED connected at GPIO 12 pin 
*    2. Blinking rate set to 2 seconds with sleep() function
*    3. Program runs indefinitely, until 'control c' key hit
*    4. Use usleep() function if faster rate is desired
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 12 - with 220 Ohm resistor in series
*   Green LED on GPIO 23 - not used in this program
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
    /* set the pin function to OUTPUT for GPIO23 */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 12
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    printf("\n Hit 'ctl c' to quit \n");
    
    while (1)   // Loop forever
    {
      GPIO_SET(io->gpio, 12);
      
      sleep(1);
      
      GPIO_CLR(io->gpio, 12);
      
      sleep(1);
    }

  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n main function done\n");   // this line never executed - ctl c

  return 0;
}
