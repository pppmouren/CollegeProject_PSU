************************************************************************************************
*
* Title:          Dimming LED Lights Using PWM
*
* Objective:      CMPEN 472 HW4 
*
* Date:           09/21/2023
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
* Program:        (0) Initially LED 1 and 3 are OFF and LED 4 is ON
*                 (1) We will observe that LED 2 goes from 0% light level to 100% light level in 0.1s;
*                 (2) LED 2 goes from 100% light level to 0% light level in 0.1 second,
*                 (3) repeat the sequence from (1) and (2)  
*
* Note:           on CSM-12C128 board, LED 1, 2, 3 and 4 are at PORTB bit 4,5,6 and 7 respectively.
*                 And Switch 1, 2, 3 and 4 are at PORTB at bit 0, 1, 2 and 3. This program will not
*                 use switches as input.
*
* Algorithm:      Simple Parallel I/O use, time delay-loop, and PWM technique
*
* Register Use:   A: LED Light on/off state and Switch on/off state, 
*                    Counters for "dim1msec" and "dimLevel" subroutine
*                 B: record current light level
*                 X: Delay 10us loop counters
*                 
* Memory Use:     RAM Locations from $3000 for data.
*                 RAM Locations from $3100 for Program
*
* Input:          Parameters hard-coded in the program - PORTB
*                 No actual input in this program. 
*                 Program will run by itself in loop
*
* Output:         LED 1 at PORTB bit 4
*                 LED 2 at PORTB bit 5
*                 LED 3 at PORTB bit 6
*                 LED 4 at PORTB bit 7
*
* Observation:    We will observe the fast blinking LED on the simulator Visualization Tool window.
*                 And we will notice that the duty cycle of an LED ON/OFF ratio change.
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

Counter1    DC.W        $002E         ; X register count number for time delay 
;Counter1    DC.W        $0001        ;  loop for 10 usec. Based on my PC, Counter set to
                                      ;  46($002E) will produce a 10 us delay. However, In order to 
                                      ;  produce a better performance, I decide to set the 
                                      ;  Counter1 to $0018 which is 24.                                    
currLevel   DC.B        $00           ; currLevel will record the current light level
maxLevel    DC.B        $64           ; maximum light level will be 100%

                                      ; all remaining data memory space for stack,
                                      ;   up to program memory start
*
***********************************************************************************************
* Program Section: address used [ $3100 to $3FFF ] RAM memory
*
            ORG         $3100               ; Program start address. in RAM
pstart      LDS         #$3100              ; initialize the stack pointer

            LDAA        #%11110000          ; LED 1,2,3,4 at PORTB bit 4,5,6,7 for Simulation only
            STAA        DDRB                ; Set PORTB bit 4,5,6,7 as output.
            
            LDAA        #%10000000
            STAA        PORTB               ; Turn off LED 1,2,4, Turn on 3 (all bits in PORTB, for simulation)
            
            LDAB        currLevel           ; Load register B with current light level
dimUp       CMPB        #$65                ; Compare register B with value 101
            BEQ         dimDownInit         ; If z = 1 means reach the level 101, branch to dimDownInit
                                            ;  If z = 0 means not reach level 101, then call dim1msec 
            JSR         dim1msec            ; Jump to dim1msec to dim LED base on current light level
            INCB                            ; Increment B Accumulator
            STAB        currLevel           ; Store new current light level to currLevel
            BRA         dimUp               ; Branch to dimUp

dimDownInit LDAB        maxLevel            ; Load register B with maxLevel = 100
            STAB        currLevel           ; Update currLevel 
dimDown     CMPB        #$00                ; Compare Accumulator B with 0
            BEQ         dimUp               ; If z = 1, means reach down to level 0, branch to dimUp
                                            ;  If z = 0, means not reach down to level 0, keep going
            JSR         dim1msec            ; Jump to dim1msec to dim LED base on current light level
            DECB                            ; Decrement B Accumulator
            STAB        currLevel           ; Store new current light level to currLevel
            BRA         dimDown             ; Branch to dimDown
*
***********************************************************************************************
* Subroutine Section: address used [ $3100 to $3FFF ] RAM memory
*
;***********************************************************************************************
; dimLevel subroutine
;
; This subroutine will delay time based on the current light level
;
; Input: a 8 bit count number in accumulator A
; Output: delay time based on the current light level
; Register in use: A Accumulator, as counter
;
dimLevel                           ; 
            JSR         delay10usec         ; Jump to delay10usec to delay 10us
            DECA                            ; Decrement accumulator A
            BNE         dimLevel            ; If z = 0, then branch back to dimLevel
                                            ; If z = 1, then return
            RTS                             ; return 
                                                                                                        
;***********************************************************************************************
; delay10us subroutine
;
; This subroutine cause 10 usec. delay
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
            LDX          Counter1       ; short delay -- 3 cycles 
            
dly10usLoop NOP                         ; total delay = X * NOP
            DEX                         ; the delay10usloop take 5 cycles for branch back, 
                                        ; 3 cycles for pass through
            BNE         dly10usLoop  
            
            PULX                        ; restore X -- 3 cycles
            RTS                         ; return -- 5 cycles
            
;***********************************************************************************************
; Dim Function Subroutine
;
; This subroutine will Dim up or Dim down LEDs
;
; Input: a 8 unsigned bit count number in "currLevel"
;         and 8 bit constant number "maxLevel"
; Output: Dim up LED from light level 0% to 100% in 0.1s 
;          or dim down LED from light level 100% to 0% in 0.1s
; Register in use: Register A  
; Memory location in use: a 8 bit input number at "currLevel", 
;                         and 8 bit constant number "maxLevel"
;
dim1msec 
            LDAA        currLevel             ; Load accumulator A with currLevel
            CMPA        #%00                  ; Compare Accumulator A with value 0
            BEQ         skip0on               ; If z = 1, branch to skip0on, since there is 
                                              ;  no need to light LED if light level is 0%
                                              ;  if z = 0, then keep going
            BSET        PORTB, #%00100000     ; Turn on LED2 at bit 5
            JSR         dimLevel              ; Jump to dimLevel
            
skip0on     LDAA        maxLevel              ; Load accumulator A with maxLevel = 100
            SUBA        currLevel             ; Substract accumulator A with currLevel 
                                              ;  to get percentage of off level
            CMPA        #%00
            BEQ         skip0off              ; If z = 1, branch to skip0off, since there is 
                                              ;  no need to turn off LED if light level is 100%
                                              ;  if z = 0, then keep going
            BCLR        PORTB, #%00100000     ; Turn off LED2 at bit 5
            JSR         dimLevel              ; Jump to dimLevel
            
skip0off    RTS                               ; Return           
          
;***********************************************************************************************            
*
* Add any subroutines here
*
            END                         ; last line of a file                 
      
            
            
            
            


                                      
                                      
                                      
                                      
                                     