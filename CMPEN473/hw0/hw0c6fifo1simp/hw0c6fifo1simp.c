/**************************************************
* File:  hw0c6fifo1simp.c
* Homework 5, pthread and FIFO queue Sample 1 Program 
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is Homework 5, pthread and FIFO queue Sample 1 Program 
*   - Multi-tasking with pthread, two thread example
*   - FIFO queue data passing example, 
*   -   keyboard input thread to Red LED ON/OFF thread
*   - Mutex critical section locking example, 
*   -   when accessing common data (FIFO queue)
*   - Real-Time (like Interrupt-Driven) embedded programming example
*   - Key board reading program example, without enter key, non-blocking 
*   - 
*   - Create two functions/threads: KeyRead and RedLED
*   -   KeyRead: scan key press every 10ms, 100 times/sec
*   -            pressed key value passed to RedLED function through FIFO queue
*   -   RedLED:  control Red LED ON/OFF based on the key press, 
*   -            pressed key value received through FIFO queue,
*   -            queued key value checked every 10ms, 100 times/sec
*   - Ending program by 'q' key hit
* 
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red    LED on GPIO 12 - with 220 Ohm resistor in series, GPIO function
*   Green  LED on GPIO 13 - with 220 Ohm resistor in series, GPIO function, not used
*   Blue   LED on GPIO 22 - with 220 Ohm resistor in series, GPIO function, not used
*   Orange LED on GPIO 23 - with 220 Ohm resistor in series, GPIO function, not used
*   Push-switch connected on GPIO 04 - with 10 KOhm pull-up resistor,       not used
*     GPIO function
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
#include "pwmsetup.h"


#define FIFO_LENGTH     1024
#define THREE_QUARTERS  (FIFO_LENGTH*3/4)

/* fifo entries are key presses, called 'command' (cmd):
 *    'n' to turn-on  the Red LED light, 
 *    'f' to turn-off the Red LED light, and
 *    'q' for quit program                               */

FIFO_TYPE(uint8_t, FIFO_LENGTH, fifo_t);

struct key_thread_param
{
  const char                    * name;
  volatile struct gpio_register * gpio;
  struct fifo_t                 * key_fifo;
};


void *KeyRead(void * arg)
{
  struct key_thread_param * param = (struct key_thread_param *)arg;
  uint8_t  cmd =      0;
  int      keyhit =   0;
  bool     done =     false;
  struct   timespec   timer_state; 
           // used to wake up every 10ms with wait_period() function, 
           // similar to interrupt occuring every 10ms 
  
  // start 10ms timed wait (like set interrupt)
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* unsigned integer 10 => 10ms */
  
  while (!(done))
  {
    keyhit = get_pressed_key();  // read once every 10ms
    if ( keyhit != -1)
    {
      switch (keyhit)
      {
        case 110:  // 'n' for turn-ON
        {
          cmd = 110;
          if (!(FIFO_FULL(param->key_fifo)))
          {FIFO_INSERT(param->key_fifo, cmd );}
          else {printf( "fifo queue full\n" );}
        }
        break;
        
        case 102:  // 'f' for turn-OFF
        {
          cmd = 102;
          if (!(FIFO_FULL(param->key_fifo)))
          {FIFO_INSERT(param->key_fifo, cmd );}
          else {printf( "fifo queue full\n" );}
        }
        break;
        
        case 113:  // 'q' for quit
        {
          cmd = 113;
          if (!(FIFO_FULL(param->key_fifo)))
          {FIFO_INSERT(param->key_fifo, cmd );}
          else {printf( "fifo queue full\n" );}
          done = true;
        }
        break;
        
        default:
        {
          cmd = keyhit;  // other key hit
          if (!(FIFO_FULL(param->key_fifo)))
          {FIFO_INSERT(param->key_fifo, cmd );}
          else {printf( "fifo queue full\n" );}
        }
        
      }
    }
    
    wait_period( &timer_state, 10u ); /* 10ms */
    
  }
  
  printf( "KeyRead function done\n" );
  
  return NULL;
  
}


void *RedLED(void * arg)
{
  struct key_thread_param * param = (struct key_thread_param *)arg;
  uint8_t  cmd =      0;
  bool     done =     false;
  struct   timespec   timer_state; 
           // used to wake up every 10ms with wait_period() function, 
           // similar to interrupt occuring every 10ms
  
  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* unsigned integer 10 => 10ms */
  
  while (!(done))
  {
    if (!(FIFO_EMPTY(param->key_fifo)))
    {
      FIFO_REMOVE(param->key_fifo, &cmd );  // read once every 10ms
      printf( " RK= %d  %c\n", cmd, cmd);
      switch (cmd)
      {
        case 110:  // 'n' for turn-ON
        {
          GPIO_SET(param->gpio, 12);
          printf( " LED On\n");
        }
        break;
        
        case 102:  // 'f' for turn-OFF
        {
          GPIO_CLR(param->gpio, 12);
          printf( " LED Off\n");
        }
        break;
        
        case 113:  // 'q' for quit the program
        {
          done = true;
          printf( " quit cmd\n");
        }
        break;
        
        default:
        {
          printf( " wrong cmd\n");
        }
        
      }
    }
    
    wait_period( &timer_state, 10u ); /* 10ms */
    
  }
  
  printf( "RedLED function done\n" );
  
  return NULL;
  
}


int main( void )
{
  struct io_peripherals *io;
  
  pthread_t t1red;
  pthread_t t2key;
  struct fifo_t             key_fifo    = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
  struct key_thread_param   key_param   = {"key", NULL, &key_fifo};

  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );

    /* set the pin function to OUTPUT for GPIO12 - red   LED light   */
    /* set the pin function to OUTPUT for GPIO13 - green LED light   */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_OUTPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_OUTPUT;
    
    key_param.gpio = io->gpio;
    
    printf("\n Hit 'n' for RedLED = ON \n");
    printf(  " Hit 'f' for RedLED = OFF \n");
    printf(  " Hit 'q' to end this program. \n");
    
    /* set initial output state - RedLED=ON, GreenLED=OFF */
    GPIO_SET(io->gpio, 12);
    GPIO_CLR(io->gpio, 13);


    // Create two threads t1red and t2key, and run them in parallel
    pthread_create(&t1red, NULL, RedLED,  (void *)&key_param);
    pthread_create(&t2key, NULL, KeyRead, (void *)&key_param);

    // Wait to finish both t1red and t2key threads
    pthread_join(t1red, NULL);
    pthread_join(t2key, NULL);


    /* main task finished  */

    /* clean the GPIO pins */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;

  }
  else
  {
    ; /* warning message already issued */
  }
  
  printf( "main function done\n" );
  
  return 0;
  
  /* This is Real-Time (like Interrupt-Driven) embedded programming example and
   * Multi-tasking with pthread programming example, and 
   * FIFO queue data passing programming example.  */
}

