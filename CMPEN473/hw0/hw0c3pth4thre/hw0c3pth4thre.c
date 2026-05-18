/**************************************************
* File:  hw0c3pth4thre.c 
* Homework 4, pthread Sample 4 Program 
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a Homework 4, pthread Sample 4 Program, two LED blinking in 2 threads. 
*  There are 4 sample programs to show, in step by step how to 
*    use pthread function.  Then there is 1 more sample program to show
*    how to write a good program with pthread function. 
*  This is the 4th sample program, showing two threads for two LED blinking.
*    Note the basic pthread parameter, pthread_create, and pthread_join 
*  This program:
*    Blink  Red    LED 1 at GPIO 12, blink at 5Hz rate, for 40 times
*    Blink  Green  LED 2 at GPIO 13, blink at 5Hz rate, for 25 times
*    LEDs blink at same time but independently
*    Any key hits are ignored
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


struct blink_thread_param
{
  volatile struct gpio_register * gpio;   // GPIO port registers
  int                             count;  // Blink count
};


void *RedBlink(void * arg)
{
  struct blink_thread_param * param = (struct blink_thread_param *)arg;
  int i = 0;
  
//  while(i < 40)              // Blink Red LED 40 times
  while(i < param->count)      // Blink Red LED 'count' times
  {
    GPIO_SET(param->gpio,12);  // Red LED on, GPIO12
    usleep(100*1000);          // numbers in micro seconds (100msec.)
    GPIO_CLR(param->gpio,12);  // Red LED off, GPIO12
    usleep(100*1000);          // numbers in micro seconds (100msec.)
    ++i;
  }
  
  return NULL;
}


void *GreenBlink(void * arg)
{
  struct blink_thread_param * param = (struct blink_thread_param *)arg;
  int i = 0;
  
  while(i < param->count)      // Blink Green LED 'count' times
  {
    GPIO_SET(param->gpio,13);  // Green LED on, GPIO13
    usleep(100*1000);          // numbers in micro seconds (100msec.)
    GPIO_CLR(param->gpio,13);  // Green LED off, GPIO13
    usleep(100*1000);          // numbers in micro seconds (100msec.)
    ++i;
  }
  
  return NULL;
}


int main( void )
{
  struct io_peripherals *io;
  
  pthread_t  t1red;
  pthread_t  t2green;
  struct blink_thread_param  red_param;
  struct blink_thread_param  green_param;
  
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
    
    printf("\n Blink Red LED 40X, then blink Green LED 25X\n");
    
    red_param.gpio =    io->gpio;
    red_param.count =   40;         // Red   LED 40X blinking
    green_param.gpio =  io->gpio;
    green_param.count = 25;         // Green LED 25X blinking
    
    // Create two threads t1red and t2green, and run them in parallel
    pthread_create(&t1red,   NULL, RedBlink,   (void *)&red_param);
    pthread_create(&t2green, NULL, GreenBlink, (void *)&green_param);

    // Wait to finish both t1red and t2green threads
    pthread_join(t1red,   NULL);
    pthread_join(t2green, NULL);
    
    /* main task finished  */
    
    /* when finished, clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
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
