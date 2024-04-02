*************************************************************************
*
* Title: StarFill (in Memory lane)
*
* Objective:  CMPEN472 HW1
*
* Date:       08/23/2023
*
* Programmer: Xuhong Lin
*
* PSU ID:     962361481
*
* PSU email:  xql5448@psu.edu
*
* Overview:   This is a program that use to get student start with
*             assebmly programming. This program will fill 222 '*'
*             in memory address from $3000 to $30DD. Accumulator A, B
*             are used to store '*' and counters correspondingly. And 
*             register x is used tp track the filling address which 
*             starts from $3000. In the loop, 'staa' oprant is used to  
*             store '*' to the current memory location that stored in 
*             register x. In each iteration, the loop will increase
*             the value in register x which cause the pointer to move 
*             downward, and decrease the counter number. And we check
*             if the counter reaches 0. 
*
* Algorithm:  Simple while-loop demo of HCS12 assembly program
*
* Register use: A accumulator: charactor data to be filled
*               B accumulator: counter, num of filled lacations
*               X register:    memoty address pointer
*
* Memory use: RAM locatins from $3000 to $30DD to store '*'
*
* Input: Parameters hard coded in the program
*
* Output: Data filled in memory locations from $3000 to $30DD changed
*
*************************************************************************
* Parameter Decleration Section
*
* Export Symbols 
        XDEF      pgstart ; export 'pgstart' symbol
        ABSENTRY  pgstart ; for assembly entry point
* Symbols and Macros
PORTA   EQU       $0000   ; i/o port address
PORTB   EQU       $0001
DDRA    EQU       $0002
DDRB    EQU       $0003
*************************************************************************
* Data Section
*
        ORG       $3000   ; reserved memory starting address
here    DS.B      $DE     ; 222 memory locations reserved
count   DC.B      $DE     ; constant, counter = 222 at address $30DE
*
*************************************************************************
* Program Section
*
        ORG       $3100   ; Program start address, in RAM
pgstart ldaa      #'*'    ; load '*' into accumulator A
        ldab      count   ; load star counter into B
        ldx       #here   ; load address pointer into X
loop    staa      0,x     ; put a star
        inx               ; point to next location
        decb              ; decrease counter
        bne       loop    ; if not done, repeat
done    bra       done    ; task finished
*
*Add any subroutines here(which we do not need in this HW)
*                        
        END               ; last line of a file       
*************************************************************************