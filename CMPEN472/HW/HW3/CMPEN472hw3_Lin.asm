************************************************************************************************
*
* Title:          LED Light ON/OFF and Switch ON/OFF
*
* Objective:      CMPEN 472 HW3 
*
* Date:           09/07/2023 --- 9/11/2023
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
* Program:        LED1 blink every 1 ms, and LED3 keeps light up
*                 ON for 0.05 ms, OFF for 0.95 ms when switch 1 is not pressed
*                 ON for 0.65 ms, OFF for 0.35 ms when switch 1 is pressed
*
* Note:           on CSM-12C128 board, Switch 1 is at PORTB bit 0, LED 1 is 
*                 at PORTB bit 4, and LED 3 is at PORTB bit 6. This program is developed and 
*                 simulated problem.So, one MUST set "switch 1" at PORTB bit 0 as an OUTPUT - 
*                 not an INPUT (IF running on CSM-12C128 board, PORB bit 0 must be set to INPUT)  
*
* Algorithm:      Simple Parallel I/O use and time delay-loop
*
* Register Use:   A: LED Light on/off state and Switch on/off state
*                 X,Y: Delay loop counters
*
* Memory Use:     RAM Locations from $3000 for data.
*                 RAM Locations from $3100 for Program
*
* Input:          Parameters hard-coded in the program - PORTB
*                 Switch 1 at PORTB bit 0
*                   (set this bit as an output for simulation only - and add Switch)
*                 Switch 2 at PORTB bit 1
*                 Switch 3 at PORTB bit 2
*                 Switch 4 at PORTB bit 3
*
* Output:         LED 1 at PORTB bit 4
*                 LED 2 at PORTB bit 5
*                 LED 3 at PORTB bit 6
*                 LED 4 at PORTB bit 7
*
* Observation:    This is a program that blinks LEDs and blinking period can 
*                 be changed with the delay loop counter value.

*
* Commetns:       This program is developed and simulated using CodeWarrior 
*                 development software and targeted for Axion Manufacturing's
*                 CSM-12C128 board running at 24MHz.
*
* Side_note:      Here is how I estimate 10 us in program. In this program, we set the cup
*                 frequency to 24MHz which will be 1/(24Mhz) = 41.67ns. In order to achieve
*                 10 us delay, we need 240 cycles. Then, in the delay10usec function, each
*                 delay10usloop loop will take 5 cpu cycles for branck back and 3 cycles for 
*                 pass through. Then, PSHX takes 2 cycles, LDX takes 3 cycles, PULX takes 3 cycles
*                 , RTS takes 5 cycles. By adding those instruction cycles, we get equation 
*                 that 16 + 5X = 240, Therefore, we have Counter1 to be x+1 = 46 cycles.
*                 (x + 1 is x times that branch back to loop, and 1 time that pass through)
************************************************************************************************
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
***********************************************************************************************
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address [ $3000 ]
;Counter1    DC.W        $002E        ; 
Counter1    DC.W        $0018         ; X register count number for time delay 
                                      ;    loop for 10 usec. Based on my PC, Counter set to
                                      ;    46($002E) will produce a 10 us delay. However, In order to 
                                      ;    produce a better performance, I decide to set the 
                                      ;    Counter1 to $0018 which is 24.

Counter65level     DC.B     $0041     ; count 65 times LED 1 on when sw1 pressed       
Counter5level      DC.B     $0005     ; count 5 times LED 1 on when sw1 not pressed
TotalCounter       DC.B     $0064     ; total counter is 100 

                                      ; all remaining data memory space for stack,
                                      ;   up to program memory start
*
***********************************************************************************************
* Program Section: address used [ $3100 to $3FFF ] RAM memory
*
            ORG         $3100         ; Program start address. in RAM
pstart      LDS         #$3100        ; initialize the stack pointer

            LDAA        #%11110001    ; LED 1,2,3,4 at PORTB bit 4,5,6,7 for Simulation only
            STAA        DDRB          ; Set PORTB bit 0,4,5,6,7 as output, the bit 0 is for sw1
            
            LDAA        #%01000000
            STAA        PORTB         ; Turn off LED 1,2,4, Turn on 3 (all bits in PORTB, for simulation)
            
mainLoop
            LDAA        PORTB         ; check bit 0 of PORTB, switch 1
            ANDA        #%00000001    ; If 0, run blinkLED1 5% light level
            BNE         p65LED1on      ; If 1, run blinkLED1 65% light level 
            
p5LED1on    
            LDAB        Counter5level               ; Load counter of "on" when sw1 not pressed to register B
            BSET        PORTB,%00010000             ; turn on LED 1, set bit to 1 in simulation, opposite om board
            JSR         delaycountlevel             ; jump to delaycountlevel to make 5% light level
            LDAB        TotalCounter-Counter5level  ; Load counter of"off" then sw1 is not pressed, 95 times 
            BCLR        PORTB,%00010000             ; Turn off LED 1, set bit to 0 in simulation, opposite on board
            JSR         delaycountlevel             ; jump to delaycountlevel to make 95% off
            BRA         mainLoop                    ; check switch, loop forever!
            
p65LED1on    
            LDAB        Counter65level              ; Load counter of "on" when sw1 pressed to register B
            BSET        PORTB,%00010000             ; turn on LED 1, set bit to 1 in simulation, opposite in board
            JSR         delaycountlevel             ; jump to delaycountlevel to make 65% light level
            LDAB        TotalCounter-Counter65level ; Load counter of"off" then sw1 is not pressed, 35 times 
            BCLR        PORTB,%00010000             ; Turn off LED 1, set bit to 0 in simulation, opposite on board
            JSR         delaycountlevel             ; jump to delaycountlevel to make 95% off
            BRA         mainLoop                    ; check switch, loop forever!

*
***********************************************************************************************
* Subroutine Section: address used [ $3100 to $3FFF ] RAM memory
*
;***********************************************************************************************
; LED delay times based on the light level we current have
delaycountlevel 
            JSR         delay10usec               ; delay 10 us
            DECB                                  ; Decrease counter
            BNE         delaycountlevel           ; If not 0 branch back to delay_cycle
            RTS                                   ; If 0, then return 
                                                  
                                                  
;***********************************************************************************************
; delay10US subroutine
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
delay10usec
            PSHX                        ; Save X -- 2 cycles 
            LDX         Counter1        ; short delay -- 
            
dly10usLoop  NOP                         ; total delay = X * NOP
            DEX                          ; the delay10usloop take 5 cycles for branch back, 
                                         ; 3 cycles for pass through
            BNE         dly10usLoop  
            
            PULX                        ; restore X -- 3 cycles
            RTS                         ; return -- 5 cycles
*
* Add any subroutines here
*
            END                         ; last line of a file                 
      
            
            
            
            


                                      
                                      
                                      
                                      
                                     