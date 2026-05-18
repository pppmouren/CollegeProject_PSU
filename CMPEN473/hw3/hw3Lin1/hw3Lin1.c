/**************************************************
* File:  hw3Lin1.c 
* Homework 3 Program Part1
* By Xuhong Lin
* CMPEN 473, Spring 2024, Penn State University
* 
* This is a LED on/off program in C, 
*  for Raspberry Pi 4 Computer - with Raspberry Pi OS 32bit
*  - 1. This Program will blink four light - red, green, blue, orange in sequence
* 	 2. Only one LED lit at a given time
* 	 3. Each LED light turn-on only for 0.25 second
* 	 4. Program repeats the four LED lighting sequence until 's' key hit
*    5. 's' key should re-start/resume with subsequent 's' key hit.
* 	 6. 'q' key hit to quit
* 	 7. only use GPIO 22 and GPIO 32 pins
* 
* Raspberry Pi 4 Computer (RPi4) GPIO pin connections:
*   Red   LED on GPIO 22 - with 33O Ohm resistor in series
*   Green LED on GPIO 23 - with 220 Ohm resistor in series
* 	Blue  LED on GPIO 23 - with 220 Ohm resistor in series
* 	Orange LED on GPIO 23 - with 20 Ohm resistor in series
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


int get_pressed_key(void){
  struct termios  original_attributes;
  struct termios  modified_attributes;
  long   oldf, newf;
  int    ch;

  /* Make input available immediately, without 'enter' key,       */
  /* no input processing, disable line editing, noncanonical mode */
  tcgetattr( STDIN_FILENO, &original_attributes );
  modified_attributes = original_attributes;
  modified_attributes.c_lflag &= ~(ICANON | ECHO);
  modified_attributes.c_cc[VMIN] = 1;
  modified_attributes.c_cc[VTIME] = 0;
  tcsetattr( STDIN_FILENO, TCSANOW, &modified_attributes );

  /* Save the mode of STDIN, then      <use fcntl>   */
  /*   change the mode to NONBLOCK, then get char,   */
  /*   and then restore the STDIN mode back.         */
  /* Key character show on the terminal (echo print) */
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  newf = oldf | O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, newf);
  ch = getchar();                       //get the character
  fcntl(STDIN_FILENO, F_SETFL, oldf);   //restore the STDIN mode back

  //reset the input to the orginal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &original_attributes );

  if (ch != -1) {                    // '0' <= valid key range <= 'z'
    if (ch > 47) {                   // ignore all other keys
      if (ch < 123) {
        } //printf( "\n ch= %d\n", ch);}
      else {ch = -1;}
    }
    else {ch = -1;}
  }

  return ch;
  // this is keyboard reading without 'Enter' key and in
  // non-blocking mode.  This function returns single key
  // value (ASCII code), and returns -1 if no key was hit
}


int main( void )
{
  struct io_peripherals *io;
	bool done = false;
	int status = 1;
 
  io = import_registers();
  if (io != NULL)
  {
    /* print where the I/O memory was actually mapped to */
    printf( "mem at 0x%8.8X\n", (unsigned int)io );
    

    /* INIT set the pin function to OUTPUT for GPIO22 */
    /* INIT set the pin function to INPUT for GPIO23 */
    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23
    
    /* set the pin function to INPUT for GPIO 04 - for a switch input (not used here)*/
    //io->gpio->GPFSEL0.field.FSEL4 = GPFSEL_INPUT;   //GPIO 04
    
    
    /* set initial output state - Off */
    GPIO_CLR(io->gpio, 22);
    
    printf("\nThe LEDs will blink as red, green, blue and orange in sequence\n");
    printf("Each LED will turn on for 0.25s\n");
    printf("Press 's' key to stop or restart program\n");
    printf("Press 'q' to quit program\n");
    printf("Any other key enter will not affect program running\n");
    
	while(!done)
	{
		
		switch(get_pressed_key())
		{
			case 's':
				if (status == 1){
					status = 0;
				}
				else{
					status = 1;
				}
				break;
				
			case 'q':
				done = true;
				break;
				
			default:
				if(status == 1){
					// Turn on Red
					io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
					io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
					GPIO_CLR(io->gpio, 22);
					usleep(250*1000); // light for 0.25s
					
					// Turn on Green
					io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
					io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
					GPIO_SET(io->gpio, 22);
					usleep(250*1000); // light for 0.25s
					
					// Turn on Blue
					io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
					io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
					GPIO_CLR(io->gpio, 23);
					usleep(250*1000); // light for 0.25s
					
					// Turn on Orange
					io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
					io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
					GPIO_SET(io->gpio, 23);
					usleep(250*1000); // light for 0.25s
					
					io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
					io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT; 	//GPIO 23 INPUT to turn off blue and orange
				}

		
		
	}
    
  }

    io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
    io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;

    printf("\n key hit 'q', quit program\n");
    
  }
  else
  {
    ; /* warning message already issued */
  }

  printf("\n Main program done\n \n"); 

  return 0;
}

/*

if (get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					done = true;
					break;
				}
			}
		}
		else if (get_pressed_key() == 'q'){
			done = true;
			continue;
		}
		else{
			// Turn on Red
			io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
			io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
			GPIO_CLR(io->gpio, 22);
			
			
		}
		
		if (get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					done = true;
					break;
				}
			}
		}
		else if (get_pressed_key() == 'q'){
			done = true;
			continue;
		}
		else{
			// Turn on Green
			io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
			io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
			GPIO_SET(io->gpio, 22);
			
			
		}
		
		if (get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					done = true;
					break;
				}
			}
		}
		else if (get_pressed_key() == 'q'){
			done = true;
			continue;
		}
		else{
			// Turn on Blue
			io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
			io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
			GPIO_CLR(io->gpio, 23);
			
			
		}
		
		if (get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					done = true;
					break;
				}
			}
		}
		else if (get_pressed_key() == 'q'){
			done = true;
			continue;
		}
		else{
			// Turn on Orange
			io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
			io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
			GPIO_SET(io->gpio, 23);
			
		}

		
		
		switch(get_pressed_key())
		{
			case 'q':
				if(get_pressed_key() == 'q'){
					break;
				}
				
			default:
				// Turn on Red
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
				GPIO_CLR(io->gpio, 22);
				usleep(250*1000); // light for 0.25s
				
				// Turn on Green
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
				GPIO_SET(io->gpio, 22);
				usleep(250*1000); // light for 0.25s
				
				// Turn on Blue
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
				GPIO_CLR(io->gpio, 23);
				usleep(250*1000); // light for 0.25s
				
				// Turn on Orange
				io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
				io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
				GPIO_SET(io->gpio, 23);
				usleep(250*1000); // light for 0.25s
				
				
		}

		// Turn on Red
		io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
		io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
		GPIO_CLR(io->gpio, 22);
		usleep(250*1000); // light for 0.25s
		
		if(get_pressed_key() == 'q'){
			break;
		}
		
		else if(get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					break;
				}
			}
		}
		
		// Turn on Green
		io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;   //GPIO 22
		io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;   //GPIO 23 Input to turn off blue and orange
		GPIO_SET(io->gpio, 22);
		usleep(250*1000); // light for 0.25s
		
		if(get_pressed_key() == 'q'){
			break;
		}
		
		else if(get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					break;
				}
			}
		}
			
		// Turn on Blue
		io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
		io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
		GPIO_CLR(io->gpio, 23);
		usleep(250*1000); // light for 0.25s
		
		if(get_pressed_key() == 'q'){
			break;
		}
		
		else if(get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					break;
				}
			}
		}
			
		// Turn on Orange
		io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;   //GPIO 22 INPUT to turn off red and green
		io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;   //GPIO 23 
		GPIO_SET(io->gpio, 23);
		usleep(250*1000); // light for 0.25s
		
		if(get_pressed_key() == 'q'){
			break;
		}
		
		else if(get_pressed_key() == 's'){
			while(get_pressed_key() != 's'){
				if (get_pressed_key() == 'q'){
					break;
				}
			}
		}
*/
