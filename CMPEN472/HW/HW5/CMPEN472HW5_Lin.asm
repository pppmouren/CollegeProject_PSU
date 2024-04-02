***********************************************************************
*
* Title:          SCI Serial Port and LED Display via Command Pannel
*
* Objective:      CMPEN 472 HW5
*
* Date:	          09/26/2023
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
* Program:        Simple SCI Serial Port I/O and Demonstration
*                 Typewriter program and LED display via command line,
*                 at PORTB
*                 
* Algorithm:      Simple Serial I/O use, typewriter, and command use
*
* Register use:	  A: Serial port data B:counter for recording currlent light level
*                 X,Y: Delay loop counters
*
* Memory use:     RAM Locations from $3000 for data, 
*                 RAM Locations from $3100 for program
*
* Output:         
*                 PORTB bit 7 to bit 4, LED 1,2,3,4. 
*                 PORTB bit 7 to 0 for 2 7-segment display
*                 
* Observation:    This is a  program will dispaly LEDs on/off 
*                 according to the command line that type by the writer.
*                 and this program will change to type writer program if 
*                 the users choose 'QUIT' command
*                   L1: LED 1 goes from 0% light level 
*                       to 100% light level in 0.1 seconds
*                   F1: LED 1 goes from 100% light level
*                       to 0% light level in 0.1 seconds
*                   L2: Turn on LED2
*                   F2: Turn off LED2
*                   L3: Turn on LED3
*                   F3: Turn off LED3
*                   L4: Turn on LED4
*                   F4: Turn off LED4
*                   QUIT: Quit menu program, and run type writrer program
*
***********************************************************************
*
* Parametyer Declearation Section
*
* Export Symbols
            XDEF        pstart       ; export 'pstart' symbol
            ABSENTRY    pstart       ; for assembly entry point
            
* Symbols and Macros
PORTB       EQU         $0001        ; i/o port b address
DDRB        EQU         $0003        ; 

SCIBDH      EQU         $00C8        ; Serial port(SCI) Baud Register H
SCIBDL      EQU         $00C9        ; Serial port(SCI) Baud Register L
SCICR2      EQU         $00CB        ; Serial port(SCI) Control Register 2
SCISR1      EQU         $00CC        ; Serial port(SCI) Status Register 1
SCIDRL      EQU         $00CF        ; Serial port(SCI) Data Register

CR          EQU         $0D          ; Carriage return,ASCII 'Return' key
LF          EQU         $0A          ; Line feed, ASCII 'next line' character

***********************************************************************
*
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000        ; Reserved RAM memory starting address 
                                     ; Only for Data for CMPEN472 class
Here        DS.B        $05          ; Allocate 5 bytes                                     

;Counter1    DC.W        $002E        ; X register count number for time delay 
Counter1    DC.W        $0017        ;  loop for 10 usec. Based on my PC, Counter set to
                                     ;  46($002E) will produce a 10 us delay. In order to 
                                     ; have a better performance in simulation, I decide 
                                     ; to set Counter1 to 23($0017)                                     
currLevel   DC.B        $00          ; currLevel will record the current light level
maxLevel    DC.B        $64          ; maximum light level will be 100%

msg1        DC.B        'Hello', $00
msg2        DC.B        'You may type below', $00                                     
msg3        DC.B        'Below is the commands you can use to control the LEDs.', $00


; Each message ends with $00(NULL ASCII character) for your program.
;
; There are 256 bytes from %3000 to $3100. If need more bytes for messgae
; put more messaage at the end of the program - before the last 'END' line.
;
; Reamining data memory sapce for stack
; Up to program memory start
*
***********************************************************************
*
* Program Section: address used [ $3100 to $3FFF ] RAM Memory
*
            ORG          $3100        ; Program start address, in RAM
pstart      LDS          #$3100       ; Initialize the stack pointer

            LDAA         #%11111111   ; Set PORTB bit 0,1,2,3,4,5,6,7
            STAA         DDRB         ; As output
            
            LDAA         #%00000000   ; Clear all bits of PORTB
            STAA         PORTB        
            
            LDAA         #$0C         ; Enable SCI port Tx and Rx units 
            STAA         SCICR2       ; Disable SCI interrupts
            
            LDD          #$0001       ; Set SCI Baud Register =  $0001 => 1.5M baud at 24MHz (for simulation)         
;            LDD          #$000D       ; Set SCI Baud Register = $009C =>   9600 baud at 24MHz                                                                    
;            LDD          #$009C       ; Set SCI Baud Register = $000D => 115200 baud at 24MHz
            STD          SCIBDH       ; SCI port baud rate change
            
            JSR          printMenu    ; Print the menu

menuLoopinit LDY         #Here         ; Load Y with $3000
             LDAB        #$00          ; Load B with 0
menuLoop    JSR          getchar      ; Check the ketboard if there is command typed in
            CMPA         #$00         ; If nothing typed, keep checking
            BEQ          menuLoop                
                                      
                                      ;  otherwise - what is typed on key board
            JSR          putchar      ; is displayed on the terminal window - echo print
      
            CMPB         #$05         ; Check if user enter more than 5 charcters
            LBEQ         errorOver5   ; Print Error        
            STAA         1, Y+        ; Store the value in Accumulator A to address in Y
            INCB                      ; Increase B
            
            CMPA         #CR          ; Detect if user do an 'enter'
            BNE          menuLoop     ; If Enter/Return key is pressed, load the saved
                                      ;  character and do command behaviors
                                      ;  if not, branch back to menuLoop
                                      
            LDAA         #LF          ; Cursor to next line
            JSR          putchar
            
            LDAA         Here         ; Load the first character to A            
            CMPA         #$46         ; Compare if the first character is 'F'
            BEQ          F            ; If yes, go to F 
            
            CMPA         #$4C         ; Compare of the first character is 'L'
            BEQ          L            ; If yes, go to L
            
            CMPA         #$51          ; Compare if the first character is 'Q'
            BEQ          Q            ; If yes, go to Q
            
            BRA          error        ; If none condition meet, branch to error
                                      ;  to print error message
            
F           
            LDAA         $3001        ; Load Second Character
            
            CMPA         #$31         ; Check if the second character is '1'
            BNE          f2           ; If no, go to f2 
            JSR          F1           ; Jump to F1 subroutine
            BRA          menuLoopinit ; Go bakc to menuLoopinit    
            
f2            
            CMPA         #$32         ; Check if the second character is '2'
            BNE          f3           ; If no, go to f3                   
            BCLR         PORTB,#%00100000       ; Turn off LED 2
            BRA          menuLoopinit ; Go back to menuLoopinit
            


f3
            CMPA         #$33         ; Check if the second character is '3'
            BNE          f4           ; If no, go to f4          
            BCLR         PORTB,#%01000000       ; Turn off LED 3
            BRA          menuLoopinit ; Go back to menuLoopinit
         
f4
            CMPA         #$34         ; Check if the second character is '4'
            BNE          error        ; If no, go to error          
            BCLR         PORTB,#%10000000       ; Turn off LED 4
            BRA          menuLoopinit ; Go back to menuLoopinit            

L           
            LDAA         $3001        ; Load Second Character
            
            CMPA         #$31         ; Check if the second character is '1'
            BNE          l2           ; If no, go to l2 
            JSR          L1           ; Jump to L1 subroutine
            BRA          menuLoopinit ; Go back to menuLoopinit   
            
l2            
            CMPA         #$32         ; Check if the second character is '2'
            BNE          l3           ; If no, go to l3          
            BSET         PORTB,#%00100000       ; Turn on LED 2
            BRA          menuLoopinit ; Go back to menuLoopinit

l3
            CMPA         #$33         ; Check if the second character is '3'
            BNE          l4           ; If no, go to f4          
            BSET         PORTB,#%01000000       ; Turn on LED 3
            BRA          menuLoopinit ; Go back to menuLoopinit
         
l4
            CMPA         #$34         ; Check if the second character is '4'
            BNE          error        ; If no, go to error          
            BSET         PORTB,#%10000000       ; Turn on LED 4
            BRA          menuLoopinit ; Go back to menuLoopinit
           
Q
            LDAA         $3001        ; Load the second character
            CMPA         #$55         ; check if the second character is 'U'
            BNE          error        ; If not 'U', go to error message
            
            LDAA         $3002        ; Load the third character
            CMPA         #$49         ; check if the third character is 'I'
            BNE          error        ; If not 'I', go to error message
            
            LDAA         $3003        ; Load the forth character
            CMPA         #$54         ; check if the forth character is 'T'
            BNE          error        ; If not 'T', go to error message
            
            BRA          typewriter   ; Go to type Writer Program
                                  
error       
            JSR          printError   ; Jump to printError subroutine
            LBRA         menuLoopinit ; Go back to menuLoop 

errorOver5
            JSR          CRandLF      ; Carrage return and Line feed
            JSR          printError   ; Jump to printError subroutine
            LBRA         menuLoopinit ; Go back to menuLoop  

typewriter  
            JSR          printTypeWriter    ; Jump to print type writer message
            
looop       JSR          getchar            ; type writer - check the key board
            CMPA         #$00               ;  if nothing typed, keep checking
            BEQ          looop
                                            ;  otherwise - what is typed on key board
            JSR          putchar            ; is displayed on the terminal window - echo print

            STAA         PORTB              ; show the character on PORTB

            CMPA         #CR
            BNE          looop              ; if Enter/Return key is pressed, move the
            LDAA         #LF                ; cursor to next line
            JSR          putchar
            BRA          looop           
                     
; Subroutine Section Below            
***********************************************************************
;**********************************************
; CRandLF subroutine function
;
; Function: Do carriage return and line feed to the terminal
; Input:    Accumulator A lode with CR and LF ASCII characters 
; Output:   Terminal will do carriage retuen and line feed behavior
; Register: Accumulator A        
;**********************************************           

CRandLF     
            PSHA                      ; Save A       
            LDAA         #CR          ; move the cursor to beginning of the line
            JSR          putchar      ;   Cariage Return/Enter key
            LDAA         #LF          ; move the cursor to next line, Line Feed
            JSR          putchar      ;   Lien feed/next line      
            PULA                      ; Get stored A  
            RTS
            
***********************************************************************  


***********************************************************************  
;**********************************************
; printmsg subroutine function
;
; Function: Output character string to SCI port, print message
; Input:    Register X points to ASCII characters in memory
; Output:   message printed on the terminal connected to SCI port
; 
; Registers modified: CCR
; Algorithm:
;     Pick up 1 byte from memory where X register is pointing
;     Send it out to SCI port
;     Update X register to point to the next byte
;     Repeat until the byte data $00 is encountered
;       (String is terminated with NULL=$00)
;**********************************************

NULL           EQU       $00
printmsg       PSHA                     ; Save registers
               PSHX
printmsgloop   LDAA      1,X+           ; pick up an ASCII character from string
                                        ;   pointed by X register
                                        ; then update the X register to point to
                                        ;   the next byte
               CMPA      #NULL
               BEQ       printmsgdone   ; end of strint yet?
               JSR       putchar        ; if not, print character and do next
               BRA       printmsgloop

printmsgdone   PULX                     ; Restore X
               PULA                     ; Restore A
               RTS

*********************************************************************** 


*********************************************************************** 
;**********************************************
; putchar subroutine function
;
; Function: Send one character to SCI port, terminal
; Input:    Accumulator A contains an ASCII character, 8bit
; Output:   Send one character to SCI port, terminal
; Registers modified: CCR
; Algorithm:
;    Wait for transmit buffer become empty
;      Transmit buffer empty is indicated by TDRE bit
;      TDRE = 1 : empty - Transmit Data Register Empty, ready to transmit
;      TDRE = 0 : not empty, transmission in progress
;**********************************************

putchar        BRCLR     SCISR1,#%10000000,putchar   ; wait for transmit buffer empty
               STAA      SCIDRL                      ; send a character
               RTS

*********************************************************************** 


*********************************************************************** 
;**********************************************
; getchar subroutine function
; 
; Function: Input one character from SCI port (terminal/keyboard)
;             if a character is received, other wise return NULL
; Input:    none    
; Output:   Accumulator A containing the received ASCII character
;           if a character is received.
;           Otherwise Accumulator A will contain a NULL character, $00.
; Registers modified: CCR
; Algorithm:
;    Check for receive buffer become full
;      Receive buffer full is indicated by RDRF bit
;      RDRF = 1 : full - Receive Data Register Full, 1 byte received
;      RDRF = 0 : not full, 0 byte received
;**********************************************

getchar        BRCLR     SCISR1,#%00100000,getchar7
               LDAA      SCIDRL
               RTS
getchar7       CLRA
               RTS
               
*********************************************************************** 


***********************************************************************
;**********************************************
; printError subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'msg13'    
; Output:   Print error message
;          
;**********************************************

printError  LDX          #msg13       ; If none of the condition meet, means wrong command entered
            JSR          printmsg     ; Print the error message and end the program
            JSR          CRandLF      ; Carrage return and Line feed
            RTS
            
*********************************************************************** 

           
***********************************************************************
;**********************************************
; printMenu subroutine function
; 
; Function: Print menu message
;       
; Input:    msg2 to msg12    
; Output:   Print menu message
;          
;**********************************************

printMenu
            
            LDX          #msg3        ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg14       ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed 
            
            LDX          #msg4        ; print the message 
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg5        ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg6        ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg7        ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg8        ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg9        ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg10       ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg11       ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #msg12       ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed

            RTS                       ; Return

*********************************************************************** 
  
  
***********************************************************************
;**********************************************
; printTypeWriter subroutine function
; 
; Function: Print type writer message
;       
; Input:    msg2 to msg12    
; Output:   Print type writer message
;          
;**********************************************

printTypeWriter
            LDX          #msg15       ; Print start message
            JSR          printmsg           
            JSR          CRandLF      ; Carrage return and Line feed

            LDX          #msg1        ; print the first message, 'Hello'
            JSR          printmsg           
            JSR          CRandLF      ; Carrage return and Line feed

            LDX          #msg2              ; print the second message
            JSR          printmsg
            JSR          CRandLF      ; Carrage return and Line feed


            RTS                       ; Return

***********************************************************************

 
***********************************************************************
;**********************************************
; F1 subroutine function
; 
; Function: LED 1 goes from 100% light level to 0% light level in 0.1 seconds
;       
; Input:    maxLevel and currLevel in data section    
; Output:   LED 1 goes from 100% light level to 0% light level in 0.1 seconds
;          
;**********************************************
 
F1
           PSHB                            ; Save B
           PSHA                            ; Save A
           LDAB        maxLevel            ; Load register B with maxLevel = 100
           STAB        currLevel           ; Update currLevel       
 
dimDown    CMPB        #$00                ; Compare Accumulator B with 0
           BEQ         F1return            ; If z = 1, means reach down to level 0, branch to menuLoop
                                           ;  If z = 0, means not reach down to level 0, keep going
           JSR         dim1msec            ; Jump to dim1msec to dim LED base on current light level
           DECB                            ; Decrement B Accumulator
           STAB        currLevel           ; Store new current light level to currLevel
           BRA         dimDown             ; Branch to dimDown 
 
F1return   PULA                            ; Restore A
           PULB                            ; Restore B 
           RTS                             ; Return
*********************************************************************** 


***********************************************************************
;**********************************************
; L1 subroutine function
; 
; Function: LED 1 goes from 0% light level to 100% light level in 0.1 seconds
;       
; Input:        
; Output:   LED 1 goes from 0% light level to 100% light level in 0.1 seconds
;          
;**********************************************
 
L1
           PSHB                            ; Save B
           PSHA                            ; Save A
           LDAB        #$00                ; Load register B with 0% light level first
           STAB        currLevel           ; Update currLevel       
 
dimUp      CMPB        #$65                ; Compare Accumulator B with 0
           BEQ         L1return            ; If z = 1, means reach down to level 0, branch to menuLoop
                                           ;  If z = 0, means not reach down to level 0, keep going
           JSR         dim1msec            ; Jump to dim1msec to dim LED base on current light level
           INCB                            ; Decrement B Accumulator
           STAB        currLevel           ; Store new current light level to currLevel
           BRA         dimUp               ; Branch to dimDown 
 
L1return   PULA                            ; Restore A
           PULB                            ; Restore B 
           RTS                             ; Return
*********************************************************************** 


***********************************************************************
;**********************************************
; dim1msec  subroutine function
; 
; Function: This subroutine will Dim up or Dim down LEDs
;       
; Input:    a 8 unsigned bit count number in "currLevel"
;           and 8 bit constant number "maxLevel"
; Output:   Dim up LED from light level 0% to 100% in 0.1s 
;           or dim down LED from light level 100% to 0% in 0.1s
; Register in use: Register A  
; Memory location in use: a 8 bit input number at "currLevel", 
;                         and 8 bit constant number "maxLevel"
;
;**********************************************

dim1msec 
            LDAA        currLevel             ; Load accumulator A with currLevel
            CMPA        #%00                  ; Compare Accumulator A with value 0
            BEQ         skip0on               ; If z = 1, branch to skip0on, since there is 
                                              ;  no need to light LED if light level is 0%
                                              ;  if z = 0, then keep going
            BSET        PORTB, #%00010000     ; Turn on LED1 at bit 5
            JSR         dimLevel              ; Jump to dimLevel
            
skip0on     LDAA        maxLevel              ; Load accumulator A with maxLevel = 100
            SUBA        currLevel             ; Substract accumulator A with currLevel 
                                              ;  to get percentage of off level
            CMPA        #%00
            BEQ         skip0off              ; If z = 1, branch to skip0off, since there is 
                                              ;  no need to turn off LED if light level is 100%
                                              ;  if z = 0, then keep going
            BCLR        PORTB, #%00010000     ; Turn off LED2 at bit 5
            JSR         dimLevel              ; Jump to dimLevel
            
skip0off    RTS                               ; Return 

***********************************************************************


***********************************************************************
;**********************************************
; dim1msec  subroutine function
; 
; Function: This subroutine will delay time based on the current light level
;       
; Input:    a 8 bit count number in accumulator A
; Output:   delay time based on the current light level
; Register in use: A Accumulator, as counter
;
;**********************************************

dimLevel                            
            JSR         delay10usec         ; Jump to delay10usec to delay 10us
            DECA                            ; Decrement accumulator A
            BNE         dimLevel            ; If z = 0, then branch back to dimLevel
                                            ; If z = 1, then return
            RTS                             ; return 

***********************************************************************  


***********************************************************************
;**********************************************
;delay10usec  subroutine function
; 
; Function: This subroutine cause 10 usec. delay
;       
; Input: a 16 bit count number in 'Counter1' 
; Output: time delay, cpu cycle wasted
; Registers in use: X register, as counter
; Memory locations in use: a 16 bit input number at 'Counter1'
; 
; Comments: one can add more NOP instructions to lengthen the delay time
;          
;**********************************************

delay10usec  
            PSHX                        ; Save X -- 2 cycles 
            LDX          Counter1       ; Short delay -- 3 cycles 
            
dly10usLoop NOP                         ; Total delay = X * NOP
            DEX                         ; The delay10usloop take 5 cycles for branch back, 
                                        ; 3 cycles for pass through
            BNE          dly10usLoop  
            
            PULX                        ; Restore X -- 3 cycles
            RTS                         ; Return -- 5 cycles
 
*********************************************************************** 

; More Message Here
msg4        DC.B        'L1: LED 1 goes from 0% light level to 100% light level in 0.1 seconds', $00
msg5        DC.B        'F1: LED 1 goes from 100% light level to 0% light level in 0.1 seconds', $00
msg6        DC.B        'L2: Trun on LED2', $00
msg7        DC.B        'F2: Turn off LED2', $00
msg8        DC.B        'L3: Turn on LED3', $00
msg9        DC.B        'F3: Turn off LED3', $00
msg10       DC.B        'L4: Turn on LED4', $00
msg11       DC.B        'F4: Turn off LED4', $00
msg12       DC.B        "QUIT: Quit menu program, run 'Type writer' program.", $00
msg13       DC.B        'Invalid command, Please try again', $00  
msg14       DC.B        'Please type one command at a time, and press Enter after you finish typing.(Lower case commands are not support)',$00
msg15       DC.B        '************* Ternimate Menu Program, Loading Type Writer Program *************', $00
;f2warning   DC.B        "LED 2 is already off, maybe try 'F1', 'QUIT' or commands strat with 'L'", $00 
          
            END               ; this is end of assembly source file
                                 ; lines below are ignored - not assembled/compiled            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
                                                                                                          