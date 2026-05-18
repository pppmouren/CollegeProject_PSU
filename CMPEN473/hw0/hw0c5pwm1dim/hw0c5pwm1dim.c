/**************************************************
* File:  hw0c5pwm1dim.c
* Homework 5, pwm Sample 1 Program 
* By Kyusun Choi
* CMPEN 473, Spring 2023, Penn State University
* 
* Revision V2.2 On 01/10/2023
* Revision V2.1 On 01/14/2022
* Revision V1.0 On 02/04/2018
* 
* This is a Homework 5, pwm Sample 1 Program, 
*  Simple LED dimming, use hardware pwm function. 
*  This is a simple two LED dimming program, showing how to use the 
*    hardware pwm function
*  This program:
*    Dim Red   LED 1 at GPIO 12, 15% light level, use hardware pwm function
*    Dim Green LED 2 at GPIO 13,  4% light level, use hardware pwm function
*    This program sets the hardware pwm function and quits
*    The pwm hardware maintains the dimming level even after the program ends.
* 
*  CPU not used for dimming, due to hardware pwm, use htop to see
* 
* For Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red    LED on GPIO 12 - with 220 Ohm resistor in series, pwm function
*   Green  LED on GPIO 13 - with 220 Ohm resistor in series, pwm function
*   Blue   LED on GPIO 22 - with 220 Ohm resistor in series, GPIO function
*   Orange LED on GPIO 23 - with 220 Ohm resistor in series, GPIO function
*   Push-switch connected on GPIO 04 - with 10 KOhm pull-up resistor, GPIO function
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


/* setting the RPi4 hardware PWM range, power level=pwm.DAT/PWM_RANGE   */
/* Minimum range is 32, typical range is 100, stepping by 1% => pwm.DAT */
/* If 0.1% stepping needed for control, use range of 1000               */
/* Example: pwm.DAT1/PWM_RANGE = 789/1000 = 78.9% level                 */
#define PWM_RANGE 100


int main( void )
{
  struct io_peripherals *io;
  
  uint32_t DLevelRed;    /* Red   dimming level as duty cycle */
  uint32_t DLevelGreen;  /* Green dimming level as duty cycle */
  
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    
    enable_pwm_clock(io->cm, io->pwm);  /* Hardware pwm needs clock to work */
    
    /* set the pin function to alternate function 0 for GPIO12, PWM for LED on GPIO12 */
    /* set the pin function to alternate function 0 for GPIO13, PWM for LED on GPIO13 */
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;
    
    /* set the pin function to OUTPUT for GPIO 22 - Red   LED */
    /* set the pin function to OUTPUT for GPIO 23 - Green LED */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input */
    io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04; NOT used 
    
    /* set initial GPIO output state - OFF */
    GPIO_CLR(io->gpio, 22);
    GPIO_CLR(io->gpio, 23);
    
    /* configure the PWM channels */
    io->pwm->RNG1 = PWM_RANGE;     /* the range value, 100 level steps */
    io->pwm->RNG2 = PWM_RANGE;     /* the range value, 100 level steps */
    io->pwm->DAT1 = 1;             /* initial beginning level=1/100=1% */
    io->pwm->DAT2 = 1;             /* initial beginning level=1/100=1% */
    io->pwm->CTL.field.MODE1 = 0;  /* PWM mode */
    io->pwm->CTL.field.MODE2 = 0;  /* PWM mode */
    io->pwm->CTL.field.RPTL1 = 1;  /* not using FIFO, but repeat the last byte anyway */
    io->pwm->CTL.field.RPTL2 = 1;  /* not using FIFO, but repeat the last byte anyway */
    io->pwm->CTL.field.SBIT1 = 0;  /* idle low */
    io->pwm->CTL.field.SBIT2 = 0;  /* idle low */
    io->pwm->CTL.field.POLA1 = 0;  /* non-inverted polarity */
    io->pwm->CTL.field.POLA2 = 0;  /* non-inverted polarity */
    io->pwm->CTL.field.USEF1 = 0;  /* do not use FIFO */
    io->pwm->CTL.field.USEF2 = 0;  /* do not use FIFO */
    io->pwm->CTL.field.MSEN1 = 1;  /* use M/S algorithm, level=pwm->DAT1/PWM_RANGE */
    io->pwm->CTL.field.MSEN2 = 1;  /* use M/S algorithm, level=pwm->DAT2/PWM_RANGE */
    io->pwm->CTL.field.CLRF1 = 1;  /* clear the FIFO, even though it is not used */
    io->pwm->CTL.field.PWEN1 = 1;  /* enable the PWM channel */
    io->pwm->CTL.field.PWEN2 = 1;  /* enable the PWM channel */

    printf( "\n Press 'ctl c' to quit.\n");
    
    DLevelRed   =  5;             /* set dim level for Red   LED */
    DLevelGreen =  2;             /* set dim level for Green LED */
    io->pwm->DAT1 = DLevelRed;    /* set Red   LED light level=15/100, GPIO12 dim */
    io->pwm->DAT2 = DLevelGreen;  /* set Green LED light level= 4/100, GPIO13 dim */
    
    /* Dimming finished, LED dim level continue even if program stops, */
    /*  because LED dimming is maintained by the hardware pwm unit     */
    
    /* when finished, clean the GPIO pins */
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
