**************************************************************************************
*
* Title:          LED Light Blinking 
*
* Objective:      CMPEN 472 HW2 
*
* Date:           08/28/2023 --- 08/30/2023
*
* Programmer:     Xuhong Lin
*
* PSU ID:         962361481
*
* PSU Email:      xql5448@psu.edu
*
* Company:        The Pennsylvania State University 
*                 Department of Computer Science and Engineering 
*
* Algorithm:      Simple Parallel I/O use and time delay-loop
*
* Register Use:   A: LED Light on/off state and Switch 1 on/off state
*                 X,Y: Delay loop counters
*
* Memory Use:     RAM Locations from $3000 for data.
*                 RAM Locations from $3100 for Program
*
* Input:          Parameters hard-coded in the program - PORTB
*                 Switch 1 at PORTB bit 0
*                 Switch 2 at PORTB bit 1
*                 Switch 3 at PORTB bit 2
*                 Switch 4 at PORTB bit 3
*
* Output:         LED 1 at PORTB bit 4
*                 LED 2 at PORTB bit 5
*                 LED 3 at PORTB bit 6
*                 LED 4 at PORTB bit 7
*
* Oberservation:  This is a program that blinks LEDs and blinking period
*                 can be changed with the delay loop counter value. And 
*                 Two modes are builded in this program. The default mode
*                 will have LED 1, 4 blink at the same time but with different
*                 on/off state. The other mode is that four LEDs lights in 
*                 sequence from LED 4 to LED 1, and then backward.
*
* Note:           All Homework programs MUST have comments silimar to this 
*                 HW2 program that has already provided. Adding more
*                 explanations and comments help you and others to understand
*                 program later.
*
* Commetns:       This program is developed and simulated using CodeWarrior 
*                 development software and targeted for Axion Manufacturing's
*                 CSM-12C128 board running at 24MHz.
*
* Side_note:      Nop Instruction takes 1 clock cycle. In this program, the clock 
*                 frequency is 24MHz which is 41.6ns per NOP instruction. This is
*                 a ideal assumption. In fact, time will also be effected by windows
*                 10/11 computer performance. 
*
**************************************************************************************
* Parameter Declearation Section
*
* Export Symbols 
            XDEF        pstart        ; export 'pstart' symbol
            ABSENTRY    pstart        ; for assembly entry point
*
* Symbols and Macros
PORTA       EQU         $0000         ; I/O port A addresses
DDRA        EQU         $0002         ; direct the input or output for portA
PORTB       EQU         $0001         ; I/O port B addresses
DDRB        EQU         $0003         ; direct the input or output for portB
*
**************************************************************************************
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address [ $3000 ]
Counter1    DC.W        $0100         ; X register count number for time delay 
                                      ;   inner loop for msec ($0040 is 4 times faster than $0100)
Counter2    DC.W        $00BF         ; Y register count number for tiem delay
                                      ;   outer loop for sec
                                      
                                      ; all remaining data memory space for stack,
                                      ;   up to program memory start
*
**************************************************************************************
* Program Section: address used [ $3100 to $3FFF ] RAM memory
*
            ORG         $3100         ; Program start address. in RAM
pstart      LDS         #$3100        ; initialize the stack pointer

;            LDAA        #%11110000    ; LED 1, 2, 3, 4 at PORTB bit 4, 5, 6, 7 FOR CSM-12C128 board
            LDAA        #%11111111    ; LED 1,2,3,4 at PORTB bit 4,5,6,7 for Simulation only
            STAA        DDRB          ; Set PORTB bit 4,5,6,7 as output 
            
            LDAA        #%00000000
            STAA        PORTB         ; Turn off LED 1,2,3,4 (all bits in PORTB, for simulation)
            
mainLoop
            BCLR        PORTB,%01100000     ; Turn off LED 2, 3 afrer branch from sw1pushed
            BSET        PORTB,%10000000     ; Turn on LED 4 at PORTB bit 7
            JSR         delay1sec           ; Wait for 1 second
            
            BSET        PORTB,%00010000     ; Turn on LED1 at PORTB bit 4
            BCLR        PORTB,%10000000     ; Turn off LED 4 at PORTB bit 7
            JSR         delay1sec           ; Wait for 1 second
            
            LDAA        PORTB
            ANDA        #%00000001          ; Read switch 1 at PORTB bit 0
            BNE         sw1pushed           ; Check to see if it is pushed
            
sw1notpush  BCLR        PORTB,%00010000     ; Turn OFF LED 1 at PORTB bit 4
            BRA         mainLoop      
            
;sw1pushed   BSET        PORTB,%00010000     ; Turn on LED 1 at PORTB bit 4
;            BRA         mainLoop

sw1pushed   
            BCLR        PORTB,%01010000     ; Turn off LED 1 from mainLoop and Turn off LED 3 
                                            ; from sw1pushed loop since LEDs light in forward
                                            ; and backward sequence
            BSET        PORTB,%10000000     ; Turn on LED 4 at bit 7 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec
            
            BCLR        PORTB,%10000000     ; Turn off LED 4
            BSET        PORTB,%01000000     ; Turn on LED 3 at bit 6 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec
            
            BCLR        PORTB,%01000000     ; Turn off LED 3
            BSET        PORTB,%00100000     ; Turn on LED 2 at bit 5 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec       
            
            BCLR        PORTB,%00100000     ; Turn off LED 2
            BSET        PORTB,%00010000     ; Turn on LED 1 at bit 4 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec
            
            BCLR        PORTB,%00010000     ; Turn off LED 1
            BSET        PORTB,%00100000     ; Turn on LED 2 at bit 5 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec
            
            BCLR        PORTB,%00100000     ; Turn off LED 2
            BSET        PORTB,%01000000     ; Turn on LED 3 at bit 6 and turn off all other LEDs
            JSR         delay1sec           ; Wait for 1 sec
            
            LDAA        PORTB
            ANDA        #%00000001          ; Read switch 1 at PORTB bit 0
            BNE         sw1pushed           ; Check to see if it is still pushed
            
            BRA         mainLoop            ; if sw1 is not pushed, go back to mainloop mode


*
**************************************************************************************
* Subroutine Section: address used [ $3100 to $3FFF ] RAM memory
*
;**************************************************************************************
;*delay1sec subroutine
;*
delay1sec
            PSHY                        ; save Y
            LDY         Counter2        ; long delay by
            
dly1Loop    JSR         delayMS         ; total time delay = Y * delayMS
            DEY
            BNE         dly1Loop        
            
            PULY                        ; restore Y
            RTS                         ; return 
            
;**************************************************************************************
; delayMS subroutine
;
; This subroutine cause few msec. delay
;
; Input: a 16 bit count number in 'Counter1' 
; Output: time delay, cpu cycle wasted
; Registers in use: X register, as counter
; Memory locations in use: a 16 bit input number at 'Counter1'
; 
; Comments: one can add more NOP instructions to lengthen the delay time
;
delayMS
            PSHX                        ; Save X
            LDX         Counter1        ; short delay
            
dlyMSLoop   NOP                         ; total delay = X * NOP
            DEX
            BNE         dlyMSLoop  
            
            PULX                        ; restore X
            RTS                         ; return
*
* Add any subroutines here
*
            END                         ; last line of a file                 
      
            
            
            
            


                                      
                                      
                                      
                                      
                                     