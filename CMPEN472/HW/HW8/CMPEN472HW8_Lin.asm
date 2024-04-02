***********************************************************************
*
* Title:          SCI Serial Port, Interrupts and Timming 
*
* Objective:      CMPEN 472 HW8
*
* Date:	          10/17/2023 -- 10/19/2023
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
* Program:        Simple SCI Serial Port I/O and Interrupts and Timming. And Implement the real
*                 Time digital clock in three 7-segment displays
*                 
*                 
* Algorithm:      Simple Serial I/O use, Interrupt and Timming: Real Time Interrupt(RTI)
*                 simple command line parsing and basic system I/O subroutine
*
* Register use:	  Accumulator: A and B: used as ckecking and passing values to buffer, output message.
*                 X: address pointer. extend division
*                 Y: address pointer, extend multiplication
*                 D: arithmetic calculation, EMUL and IDIV.
*
* Memory use:     RAM Locations from $3000 for data, 
*                 RAM Locations from $3100 for program
*
* Output:         
*                 Terminal Window and three 7-segment display for time 
*                 
* Observation:    This is a  program will perform digital clock in real time. Rules are:
*                 1. 24 hour clock, display on the terminal and the two 7-segment LED displays
*                 2. "t" for 'set time' command, "q" for 'quit' command, "h" for 'hour display' command,
*                 3. "m" for 'minute display' command, and "s" for 'second display' command
*                 4. Display two digit hour, minute, or second on the LED display
*                 5. Two digit display on the two 7-segment LED displays attached to PORTB
*                 6. Update the time on the terminal and LED displays every second
*                 7. Clock commands ("t", "q", "h", "m", and "s" commands) entered on the terminal screen
*                 8. Use only one line on the terminal for the clock, separate columns for "Clock", "CMD>", and "Error>" display
*                 9. Show "Clock> " prompt for time display, "CMD> " prompt for command, and "Error> " prompt for error message
*                10. In case of an invalid input, print error message on the same line: "Error> Invalid input"
*                11. Clock time command display continue updated every second, while a "t" command being entered
*                12. Use Real Time Interrupt feature to keep the time               
*
***********************************************************************
*
* Parameter Declearation Section
*
* Export Stmbols
            XDEF        Entry         ; export 'pstart' symbol
            ABSENTRY    Entry         ; for assembly entry point
            
* Symbols and Macros
PORTA       EQU         $0000         ; i/o port a address
DDRA        EQU         $0002
PORTB       EQU         $0001         ; i/o port b address
DDRB        EQU         $0003         ; 

SCIBDH      EQU         $00C8         ; Serial port(SCI) Baud Register H
SCIBDL      EQU         $00C9         ; Serial port(SCI) Baud Register L
SCICR2      EQU         $00CB         ; Serial port(SCI) Control Register 2
SCISR1      EQU         $00CC         ; Serial port(SCI) Status Register 1
SCIDRL      EQU         $00CF         ; Serial port(SCI) Data Register

CRGFLG      EQU         $0037         ; Clock and Reset Generator Flags
CRGINT      EQU         $0038         ; Clock and Reset Generator Interrupts
RTICTL      EQU         $003B         ; Real Time Interrupts Control
                                                    
CR          EQU         $0D           ; Carriage return,ASCII 'Return' key
LF          EQU         $0A           ; Line feed, ASCII 'next line' character
s           EQU         $73           ; 's' ASCII
q           EQU         $71           ; 'q' ASCII
t           EQU         $74           ; 't' ASCII
m           EQU         $6D           ; 'm' ASCII
h           EQU         $68           ; 'h' ASCII
space       EQU         $20           ; ' ' space in ASCII 
colon       EQU         $3A           ; ':' colon in ASCII
ASCIInumBias EQU        $30           ; Set the ASCII number '0' to '9' bias as $30
***********************************************************************
*
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address 
            
Here        DS.B        $0B           ; Reserve 11 bytes
Terminate1  DC.B        $00           ; Terminate Byte  

ctr2p5m     DS.W        1             ; Interrupt counter for 2.5 mSecs of time
Counter0    DC.B        $03           ; Set Counter0 = 3

ACheck      DC.B        $00           ; Set ACheck flog as 0 for init state user is typing, 
                                      ;  and 1 for error occur, and 2 for new time, 3 for valid time display
IsStartTime DC.B        $00           ; Set IsStartTime flag as 0 for user hasn't enter start time. and 1 for the start time entered 
h_flag      DC.B        $00           ; Hours flag, 1:h command is entered 0:no
m_flag      DC.B        $00           ; Minute flag, 1: m command is entered. 0:no
s_flag      DC.B        $00           ; Second flag, 1: s command is entered. 0:no
q_flag      DC.B        $00           ; Q_flag, 1: q command is entered. 0:no

hoursBuffer DS.B        1             ; Reserve 1 byte for storing hours that user typed in
minsBuffer  DS.B        1             ; Reserve 1 byte for storing minutes that user typed in
secsBuffer  DS.B        1             ; Reserve 1 byte for storing seconds that user typed in  
dispBuffer  DS.B        1             ; Reserve 1 byte for storing diaplay number
temphead1   DC.B        $00           ; Set temphead1 for get decimal number from hex
tempBuffer1 DS.B        1             ; Reserve 1 byte for temp restoring data
temphead2   DC.B        $00           ; Set temphead2 for get decimal number from hex
tempBuffer2 DS.B        1             ; Reserve 1 byte for temp restoring data
temphours   DS.B        1             ; Reserve 1 byte for temp restoring data
tempminutes DS.B        1             ; Reserve 1 byte for temp restoring data
tempseconds DS.B        1             ; Reserve 1 byte for temp restoring data

prompt1     DC.B        'Clock> '     ; Clock prompt         
OutTime     DC.B        '        '    ; Reverve 8 bytes for output time  
Terminate2  DC.B        $00           ; Terminate Byte
prompt2     DC.B        '     CMD> '  ; CMD prompt
OutCMD      DS.B        $0B           ; Reserve 11 bytes for output cmd  
Terminate3  DC.B        $00           ; Terminate Byte  
prompt3     DC.B        '     Error> ' ; Error prompt
OutError    DS.B        $19           ; Reserve 25 bytes for error
Terminate4  DC.B        $00           ; Terminate Byte 
      
*
***********************************************************************
*
* Interrupt Vector Section: address Used from $FFF0 in simulation ($3FF0 for CSM-12C128)
*
            ORG         $FFF0         ; RTI interrupt vector setup for the simulator
            ;ORG         $3FF0         ; RTI interrupt vector setup for the CSM-12C128 board
            DC.W        rtiisr
*
***********************************************************************
*
* Program Section: address used [ $3100 to $3FFF ] RAM Memory
*
            ORG         $3100         ; Program start address, in RAM
Entry       LDS         #$3100        ; Initialize the stack pointer

            LDAA        #%11111111    ; Set PORTB bit 0,1,2,3,4,5,6,7
            STAA        DDRA          ; Set all bit as output 
            STAA        DDRB          ; Set all bit as output 
            STAA        PORTA         ; Init all bits of PORTA
            STAA        PORTB         ; Init all bits of PORTB
            
            LDAA        #$0C          ; Enable SCI port Tx and Rx units 
            STAA        SCICR2        ; Disable SCI interrupts
            
            LDD         #$0001        ; Set SCI Baud Register =  $0001 => 1.5M baud at 24MHz (for simulation)         
;            LDD         #$0002       ; Set SCI Baud Register = $0002 => 750K baud at 24MHz
;            LDD         #$000D       ; Set SCI Baud Register = $000D => 115200 baud at 24MHz
;            LDD         #$009C       ; Set SCI Baud Register = $009C => 9600 baud at 24MHz                                  
            STD         SCIBDH        ; SCI port baud rate change $000D => 115200 baud at 24MHz
            
            BSET        RTICTL,%00011001 ; set RTI: dev=10*(2**10)=2.555msec for C128 board
                                         ;  4MHz quartz oscillator clock
            BSET        CRGINT,%10000000 ; enable RTI interrupt
            BSET        CRGFLG,%10000000 ; clear RTI IF (Interrupt Flag)


            LDX         #40
            STX         ctr2p5m          ; initialize interrupt counter with 40.
            CLI                          ; enable interrupt, global
            
            JSR         printMenu     ; Print the menu   
            


mainLoopinit
            LDY         #Here         ; Load Y with $3000
            LDX         #OutCMD       ; Load X with the address of output message
            LDAB        #$00          ; Load B with 0
            
            
            
mainLoop    
            JSR         Interrupt_Out ; Jump to Interrupt_Out
            
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$00          ; If A is not 0, means either a vialid input in entered or error occur
            BNE         mainLoop      ; Just keep looping until 1s reached and print output
            
            LDAA        q_flag        ; Load A with q_flag
            CMPA        #$01          ; If q_flag is 1
            LBEQ        TypeWriter    ; Branch to TypeWriter Program
            
            JSR         getchar       ; Check the ketboard if there is command typed in
            CMPA        #$00          ; If nothing typed, keep checking
            BEQ         mainLoop                
            
            CMPB        #$0B          ; Check if user enter more than 11 charcters
            LBEQ        Error0        ; Print Error
            
            STAA        1, Y+         ; Store the value in Accumulator A to address in Y
            INCB                      ; Increase B
            
            CMPA        #CR           ; Detect if user do an 'enter'
            BEQ         next          ; If Enter/Return key is pressed, load the saved
                                      ;  character and do command behaviors
                                      ;  if not, branch back to menuLoop
            STAA        1, X+         ; Store the value in Accumulator A to address in X  
            BRA         mainLoop      ; Branch back to mainLoop              
                   
                                                 
next        ; Check the first char to determine which command
            LDY         #Here         ; Load Y as the pointer that point to address 'Here'
            LDAA        1,Y+          ; Load A with the content in address that stored in Y
            
            CMPA        #CR           ; If user enter nothing, just go the next line with prompt
            LBEQ         CR_handler    ; Branch back to CR_handler        
            CMPA        #t            ; Check If the command is set command
            BEQ         setTime       ; Branch to setTime 
            CMPA        #q            ; Check if the command is quit command
            LBEQ        Quit          ; Branch to Quit
            CMPA        #h            ; Check if the command is hour display command
            LBEQ        hourDisp      ; Branch to hourDisp
            CMPA        #m            ; Check if the command is minute display command 
            LBEQ        minuteDisp    ; Branch to minuteDisp
            CMPA        #s            ; Check if the command is second display command
            LBEQ        secondDisp    ; Branch to secondDisp
            LBRA        Error1        ; If none is matched, the comamnd is invalid
            
                     
            
setTime
            ; Check the second char
            LDAA        1,Y+          ; Load the second char into Accumulator A
            CMPA        #space        ; Check if the second char is a space
            LBNE        Error0        ; if not, branch to Error01, invaliad input format
            
            
            ; Check the hour part
            JSR         PutHours      ; Jump to PutHours, Y will goes to next two in this subroutine 
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error0        ; If Acheck = 1, error happen, jump to Error01
                                      ; else
            LDAA        tempBuffer1   ; Load A with the result
            STAA        hoursBuffer   ; Store it to the hourBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; Check the colon
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #colon        ; Check if is a colon
            LBNE        Error0        ; if not, branch to Error01, invaliad input format
            
            ; Check the minutes part
            JSR         PutMinsorSecs ; Jump to PutMinsandSecs
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error0        ; If Acheck = 1, error happen, jump to Error01
            
    
            LDAA        tempBuffer1   ; Load A with the result
            STAA        minsBuffer    ; Store it to the minsBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; check the colon
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #colon        ; Check if is a colon
            LBNE        Error0        ; if not, branch to Error01, invaliad input format
            
            ; check the seconds part
            JSR         PutMinsorSecs ; Jump to PutMinsandSecs
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error0        ; If Acheck = 1, error happen, jump to Error01
            
            LDAA        tempBuffer1   ; Load A with the result
            STAA        secsBuffer    ; Store it to the secsBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; check the return key
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #CR           ; Check if is a return
            BNE         Error0        ; branch to Error0
            
            LDAA        #$02          ; Load A with 2
            STAA        ACheck        ; Store ACheck = 2, new time is set
            LDAA        #$01          ; Load A with 1
            STAA        IsStartTime   ; Store IsStartTime = 1, means the user has put a valid time 
              
            LBRA        mainLoopinit  ; Branch to mainLoopinit
            

Quit
            LDAA        Y             ; Load the second char
            CMPA        #CR           ; Check if the second char is the carriage return
            BNE         Error0        ; Branch to Error01, invalid input
            
            LDAA        #$01          ; Load A with 1
            STAA        q_flag        ; Set the q_flag as 1
            LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set ACheck = 3, valid input
            LBRA        mainLoopinit  ; Branch back to mainLoopinit
            
hourDisp
            LDAA        1,Y+          ; Load the second char which should be an enter
            CMPA        #CR           ; Compare it with an enter
            BNE         Error0        ; if not, input is not valid
            
            LDAA        #$01          ; Load A with 1
            STAA        h_flag        ; flag the h, means display the hour in 7-segment led
            LDAA        #$00          ; Load A with 0
            STAA        m_flag        ; Flag down the minute
            STAA        s_flag        ; Flag down the second
            
            LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set Acheck = 3, valid input
            LBRA        mainLoopinit  ; Branch to mainLoopinit 

minuteDisp
            LDAA        1,Y+          ; Load the second char which should be an enter
            CMPA        #CR           ; Compare it with an enter
            BNE         Error0        ; if not, input is not valid
            
            LDAA        #$01          ; Load A with 1
            STAA        m_flag        ; flag the m, means display the minutes in 7-segment led
            LDAA        #$00          ; Load A with 0
            STAA        h_flag        ; Flag down the hour
            STAA        s_flag        ; Flag down the second
            
            LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set Acheck = 3, valid input
            LBRA        mainLoopinit  ; Branch to mainLoopinit 
        

secondDisp 
            LDAA        1,Y+          ; Load the second char which should be an enter
            CMPA        #CR           ; Compare it with an enter
            BNE         Error0        ; if not, input is not valid
            
            LDAA        #$01          ; Load A with 1
            STAA        s_flag        ; flag the s, means display the second in 7-segment led
            LDAA        #$00          ; Load A with 0
            STAA        m_flag        ; Flag down the minute
            STAA        h_flag        ; Flag down the second
            
            LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set Acheck = 3, valid input 
            LBRA        mainLoopinit  ; Branch to mainLoopinit

CR_handler  LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set ACheck = 3, we treat CR as a viald input
            LBRA        mainLoopinit  ; Branch to mainLoopinit  
            
Error0      LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            LDAA        #$01          ; Load A with 1
            STAA        ACheck        ; Set ACheck = 1
            LDAA        temphours     ; Restore hours
            STAA        hoursBuffer
            LDAA        tempminutes   ; Restore minutes
            STAA        minsBuffer
            LDAA        secsBuffer    ; Restore seconds
            STAA        tempseconds
            JSR         fillError0    ; Jump to printError0 subroutine
            LBRA        mainLoopinit  ; Branch back to mainLoopinit

Error1      LDAA        #$01          ; Load A with 1
            STAA        ACheck        ; Set ACheck = 1
            JSR         fillError1    ; Jump to printError1 subroutine
            LBRA        mainLoopinit  ; Branch back to mainLoopinit   
            
TypeWriter
            BCLR         CRGINT,%10000000   ; Disable RTI interrupt
looop       JSR          getchar            ; Type writer - check the key board
            CMPA         #$00               ;  if nothing typed, keep checking
            BEQ          looop
                                            ;  Otherwise - what is typed on key board
            JSR          putchar            ; is displayed on the terminal window - echo print

            STAA         PORTB              ; Show the character on PORTB

            CMPA         #CR
            BNE          looop              ; If Enter/Return key is pressed, move the
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
            PSHA                     ; Save A       
            LDAA        #CR          ; move the cursor to beginning of the line
            JSR         putchar      ;   Cariage Return/Enter key
            LDAA        #LF          ; move the cursor to next line, Line Feed
            JSR         putchar      ;   Lien feed/next line      
            PULA                     ; Get stored A  
            RTS
            
*********************************************************************** 


***********************************************************************            
;**********************************************
; rtiisr subroutine function
; 
; Function: RTI interrupt service routine       
;**********************************************           

rtiisr      BSET   CRGFLG,%10000000 ; clear RTI Interrupt Flag - for the next one
            LDX    ctr2p5m          ; every time the RTI occur, increase
            INX                     ;    the 16bit interrupt count
            STX    ctr2p5m
rtidone     RTI
            
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

NULL           EQU      $00
printmsg       PSHA                    ;Save registers
               PSHX
printmsgloop   LDAA     1,X+           ;pick up an ASCII character from string
                                        ;   pointed by X register
                                        ;then update the X register to point to
                                        ;   the next byte
               CMPA     #NULL
               BEQ      printmsgdone   ;end of strint yet?
               JSR      putchar        ;if not, print character and do next
               BRA      printmsgloop

printmsgdone   PULX 
               PULA
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
; printMenu subroutine function
; 
; Function: Print menu message
;       
; Input:    msg2 to msg12    
; Output:   Print menu message
;          
;**********************************************

printMenu
            PSHX                      ; Save X
            
            LDX          #main_msg0   ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg1   ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed 
            
            LDX          #main_msg2   ; print the message 
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg3   ; print the message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg4   ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg5   ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed

            PULX                      ; Restore X
            RTS                       ; Return

***********************************************************************            


***********************************************************************
;**********************************************
; fillError0 subroutine function
; 
; Function: Fill error message to output 
;       
; Input:    error message 'error_msg0'    
; Output:   fill error message
;          
;**********************************************

fillError0
            PSHY                         ; Save Y
            PSHX                         ; Save X
            PSHA                         ; Save A
            
            
            LDY          #error_msg0     ; Load Y with the address of error_msg0
            LDX          #OutError       ; Load X with the address of OutMsg
            
FE0Loop     LDAA         1,Y+            ; Load A with the content in address in Y
            CMPA         #$00            ; Check if reach the terminate
            BEQ          FE0Done         ; branch to Fe0Done
            STAA         1,X+            ; Store A to X
            BRA          FE0Loop
            
            
            
FE0Done     PULA                         ; Restore A
            PULX                         ; Restore X
            PULY                         ; Restore Y
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; fillError1 subroutine function
; 
; Function: Fill error message to output 
;       
; Input:    error message 'error_msg1'    
; Output:   fill error message
;          
;**********************************************

fillError1
            PSHY                         ; Save Y
            PSHX                         ; Save X
            PSHA                         ; Save A   
            
            
            LDY          #error_msg1     ; Load Y with the address of error_msg0
            LDX          #OutError       ; Load X with the address of OutMsg
            
FE1Loop     LDAA         1,Y+            ; Load A with the content in address in Y
            CMPA         #$00            ; Check if reach the terminate
            BEQ          FE1Done         ; branch to Fe0Done
            STAA         1,X+            ; Store A to X
            BRA          FE1Loop
            
            
            
FE1Done     PULA                         ; Restore A
            PULX                         ; Restore X
            PULY                         ; Restore Y
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; CleanFunction subroutine function
;
; Function: it will set the memory address In Y to Null
; Input:    address in Y
; Output:   all $00 in output message
; Register: register Y, accumulator A       
;**********************************************           

CleanFunction     
            PSHA                      ; Save A  
           
CFLoop      LDAA        Y             ; Load A with the value that contained in address in Y
            
            CMPA        #$00          ; Check if it is a null 
            BEQ         CFDone        ; Branch to COMDone  
            
            LDAA        #$00          ; Load A with 0
            STAA        Y             ; Store 0 to the address in Y
            INY                       ; Increase Y
            BRA         CFLoop        ; Branch back to keep cleaning the entire output message        
                 
CFDone      PULA                      ; Get stored A  
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; PutHours subroutine function
;
; Function: will put the hours that the user typed into the hoursBuffer
;           and check for any potential error
; Input:    hours that the user put
; Output:   tempBuffer1 or error flag set
; Register: register Y, accumulator A       
;**********************************************           

PutHours
            PSHD                      ; Save D  
            
            LDAA        Y             ; Load A with address in Y
            LDAB        1,Y+          ; Load B with address in Y and Y++
            ; first digit of hout should be 0-2
            CMPA        #$32          ; Compare A with $32 which is 2 in ASCII
            BHI         PHError       ; If A is greater than 2, than error happen
            CMPA        #$30          ; Compare A with $30 which is 0 in ASCII
            BLO         PHError       ; Branch if A is less than $30
            SUBA        #ASCIInumBias  ; Substract A with ASCII number Bias
            JSR         FormDECtoHEX  ; Jump to FormDECtoHEX
            
            ; second digit of hour should be 0-9 for first digit is 0,1 
            ; second digit of hour shobe be 0-4 for first digit is 2
            LDAA        1,Y+          ; Load the second digit
            CMPB        #$32          ; If the first digit is 2
            BEQ         PHfirstis2    ; Branch to PHfirstis2
            CMPA        #$39          ; Compare A with $39 which is 9 in ASCII
            BHI         PHError       ; If A is greater than 9, than error happen
            CMPA        #$30          ; Compare A with $30 which is 0 in ASCII
            BLO         PHError       ; Branch if A is less than $30    
            SUBA        #ASCIInumBias  ; Substract A with ASCII number Bias
            JSR         FormDECtoHEX  ; Jump to FormDECtoHEX
            BRA         PHDone        ; Jump to PHDone
                        
PHfirstis2                
            CMPA        #$33          ; Compare A with $34 which is 4 in ASCII
            BHI         PHError       ; If A is greater than 4, than error happen
            CMPA        #$30          ; Compare A with $30 which is 0 in ASCII
            BLO         PHError       ; Branch if A is less than $30    
            SUBA        #ASCIInumBias  ; Substract A with ASCII number Bias
            JSR         FormDECtoHEX  ; Jump to FormDECtoHEX
            BRA         PHDone        ; Jump to PHDone  
            
            
PHError     LDAA        #$01          ; LOAD A with 1
            STAA        ACheck        ; Set ACheck to 1, error occur            
            
                 
PHDone      PULD                      ; restored D  
            RTS
            
*********************************************************************** 
           
           
***********************************************************************
;**********************************************
; PutMinsorSecs subroutine function
;
; Function: will put the mins ot secs that the user typed into the minsBuffer or secsBuffer
;           and check for any potential error
; Input:    minutes or seconds that the user put
; Output:   tempBuffer1 or error flag set
; Register: register Y, accumulator A       
;**********************************************           

PutMinsorSecs
            PSHA                      ; Save A  
            
            LDAA        1,Y+          ; Load A with address in Y
            ; first digit of hout should be 0-5
            CMPA        #$35          ; Compare A with $35 which is 5 in ASCII
            BHI         PMSError       ; If A is greater than 2, than error happen
            CMPA        #$30          ; Compare A with $30 which is 0 in ASCII
            BLO         PMSError      ; Branch if A is less than $30
            SUBA        #ASCIInumBias  ; Substract A with ASCII number Bias
            JSR         FormDECtoHEX  ; Jump to FormDECtoHEX
            
            ; second digit of hour should be 0-9 
            LDAA        1,Y+          ; Load the second digit
            CMPA        #$39          ; Compare A with $39 which is 9 in ASCII
            BHI         PMSError      ; If A is greater than 9, than error happen
            CMPA        #$30          ; Compare A with $30 which is 0 in ASCII
            BLO         PMSError      ; Branch if A is less than $30    
            SUBA        #ASCIInumBias  ; Substract A with ASCII number Bias
            JSR         FormDECtoHEX  ; Jump to FormDECtoHEX
            BRA         PMSDone       ; Jump to PMSDone
                        
        
            
PMSError    LDAA        #$01          ; LOAD A with 1
            STAA        ACheck        ; Set ACheck to 1, error occur            
            
                 
PMSDone     PULA                      ; restored A  
            RTS
            
*********************************************************************** 
            

***********************************************************************
;**********************************************
; FormDECtoHEX subroutine function
; 
; Function: Get time transfer from dec to hex form
;       
; Input:    tempBuffer1 and tempBuffer2 (1 for final result. 2 for passed parameter)
; Output:   result in tempBuffer1
;          
;**********************************************

FormDECtoHEX
            STAA         tempBuffer2      ; Store the Actual HEX value to tempBuffer2
            
            PSHD                          ; Save D
            
            LDAA         tempBuffer1      ; Load tempBuffer1
            LDAB         #$0A             ; Load B with 10
            MUL                           ; AxB
            ADDB         tempBuffer2      ; Add B with the content in B
            STAB         tempBuffer1      ; Store it to tempBuffer1
           
FDHDone     PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; Interrupt_Out subroutine function
; 
; Function: Each 1s, it will going to update the time and print output message
;       
; Input:    all output buffer, and hour, mins, secs buffer, flags and ACheck
; Output:   output update each one second
;          

;**********************************************

Interrupt_Out            
            PSHD                          ; Save D
            PSHX                          ; Save X
            PSHY                          ; Save Y
            
            LDX          ctr2p5m          ; Check for 1.0 s
            CPX          #60              ; 2.5msec * 60 = 0.15 sec. If want 1s, then needs 400, but simulation is slow
            LBLO         IODone           ; Not Yet one second, Branch to done
            
            LDX          #0               ; 0.5sec is up,
            STX          ctr2p5m          ;     clear counter to restart
            
            LDAB         ACheck           ; Let B keep the value of ACheck
            
            ;LDAA         #CR              ; Load A with Carriage Return
            ;JSR          putchar          ; Do CR on terminal
            

            ; Figure out what need to print for time
            LDAA         IsStartTime      ; Load A with the IsStartTime flag
            CMPA         #$00             ; Compare with 0
            BEQ          PrintTime        ; If yes, branch to PrintTim
            CMPB         #$02             ; Compare B with 2
            BEQ          PutTime          ; If ACheck = 2, new time is set, branch to PutTime
            JSR          ChangeTime       ; Else, branch to ChangeTime, to increment time by 1s
            
PutTime     JSR          FillOutTime      ; Jump to FillOutTime to fill the OutTime in the memory bases on the time
                        
            
PrintTime   LDX          #prompt1         ; Load A with the address of prompt1
            JSR          printmsg         ; Jump to printmsg
            
            ; Check the q command             
            LDAA         q_flag           ; Load A with the value in q_flag
            CMPA         #$01             ; If q_flag = 1, then Branch to print q command message
            BEQ          Q_message        ; Branch to Q_message
            
            ; Figure out the h, m, s command
            LDAA         h_flag           ; Load the h_flag to A
            CMPA         #$01             ; Check if h_flag is 1
            BEQ          GetHour          ; Yes, Branch to GetHour
                                          ; Else, h_flag = 0, check the m_flag next
            LDAA         m_flag           ; Load the m_flag to A
            CMPA         #$01             ; Check if m_flag is 1
            BEQ          GetMinute        ; Yes, branch to GetMinute
                                          ; Else, m_flag = 0, check the s_flag next
            LDAA         s_flag           ; Load the s_flag to A
            CMPA         #$01             ; Check if s_flag is 1
            BEQ          GetSecond        ; Yes, branch to GetSecond
            BRA          CMD_part         ; Else, all three flag are 0, branch to CMD part    
            
GetHour     LDAA         hoursBuffer      ; Load A with hoursBuffer
            BRA          DispTime         ; Branch to Display Time

GetMinute   LDAA         minsBuffer       ; Load A with minsBuffer
            BRA          DispTime         ; Branch to Display Time

GetSecond   LDAA         secsBuffer       ; Load A with secsBuffe
            
DispTime    STAA         tempBuffer1      ; Store it into tempBuffer1
            JSR          GetDispNumber    ; Jump to GetDispNumber
            LDAA         dispBuffer       ; Load the result from dispBuffer
            STAA         PORTB            ; Send it to PORTB, display it on 7-segment led
            
                        
CMD_part    ; Figure out what shold be print in CMD part
            LDY          #Here            ; Load Y as a pointer to Here
            LDAA         Y                ; Load the first byte of Here to A
            CMPA         #$00             ; If the first byte is a NULL
            BEQ          IOReset          ; Means the user does not type anything, don't need to print anything
            
            LDX          #prompt2         ; Load X with the address of prompt2
            JSR          printmsg         ; Print message
        
            CMPB         #$00             ; Compare B with 0
            BNE          CleanInfo        ; If Acheck is not 0, then after print out the message we need to 
                                          ; Clean out the Here and OutCMD for next command.           
            BRA          IOReset          ; Else, if ACheck is 0, user is still typing, Branch to IOReset

CleanInfo   LDY          #Here            ; Load Y with address of Here
            JSR          CleanFunction    ; Clean the Here Buffer
            LDY          #OutCMD          ; Load Y with address of OutCMD
            JSR          CleanFunction    ; Clean the OutCMD Buffer        
        
        
            ; Figure out when to print error
            CMPB         #$01             ; Check if ACheck == 1
            BEQ          PrintError       ; If ACheck == 1, error occur, print the error
            BRA          IOReset          ; Branch to IOReset
            
PrintError  LDX          #prompt3         ; Load X with the address of prompt3
            JSR          printmsg         ; Print the error message
            LDY          #OutError        ; Load Y with the address of OutError
            JSR          CleanFunction    ; Jump to CleanFunction 
            BRA          IOReset          ; Branch to IOReset

Q_message   LDX          #prompt2         ; Load X with the address of prompt2
            JSR          printmsg         ; Print message
            JSR          CRandLF          ; Next Line
            LDX          #end_msg         ; Load X with address of end_msg
            JSR          printmsg         ; Print message
            JSR          CRandLF          ; Next line
            LDX          #tw_msg0         ; Load X with address of tw_msg0 
            JSR          printmsg         ; Print message 
            JSR          CRandLF          ; Next line                                
           
IOReset     LDAB         #0
            STAB         ACheck
            LDAA         #CR              ; Load A with Carriage Return
            JSR          putchar          ; Do CR on terminal
            
IODone                  
            LDAA        hoursBuffer   ; Before update the hourBuffer
            STAA        temphours     ; Store it first
            LDAA        minsBuffer    ; Before update the minsBuffer
            STAA        tempminutes   ; Store it first
            LDAA        minsBuffer    ; Before update the secsBuffer
            STAA        tempminutes   ; Store it first
            
            PULY                          ; Restore Y
            PULX                          ; Restore X
            PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; ChangeTime subroutine function
; 
; Function: Increment the time by 1s
;       
; Input:    hoursBuffer, minsBuffer, secsBuffer
; Output:   time update on hoursBuffer, minsBuffer, secsBuffer 
;          
;**********************************************

ChangeTime            
            PSHA                          ; Save A
            
            LDAA         secsBuffer       ; Load seconds to A
            CMPA         #59              ; Check if seconds = 59
            BEQ          SecondBack       ; Branch to SecondBack
            ADDA         #$01             ; If A != 59, then add 1, update it to secsBuffer and return
            STAA         secsBuffer       ; Store new time to secsBuffer
            BRA          CTDone           ; Branch to CTDone
            
SecondBack  LDAA         #$00             ; Load A with 0
            STAA         secsBuffer       ; Let seconds back to 00
            
            LDAA         minsBuffer       ; Load minutes to A
            CMPA         #59              ; Chekc is minutes = 59
            BEQ          MinuteBack       ; Branch to MinuteBack
            ADDA         #$01             ; If not 59, then add 1, update it to minsBuffer and return 
            STAA         minsBuffer       ; Store new time to minsBuffer
            BRA          CTDone           ; Branch to CTDone
            
MinuteBack  LDAA         #$00             ; Load A with 0
            STAA         minsBuffer       ; Let minute back to 00
            
            LDAA         hoursBuffer      ; Load hour to A
            CMPA         #23              ; Chekc is hour = 23
            BEQ          HourBack         ; Branch to HourBack
            ADDA         #$01             ; If not 59, then add 1, update it to hoursBuffer and return 
            STAA         hoursBuffer      ; Store new time to hoursBuffer
            BRA          CTDone           ; Branch to CTDone  
            
HourBack    LDAA         #$00             ; Load A with 0
            STAA         hoursBuffer      ; Let minute back to 00
       
           
CTDone      PULA                          ; Restore A
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; FillOutTime subroutine function
; 
; Function: Fille the OutTime memory area based on what is in hour, minute, second Buffer
;       
; Input:    MinsBuffer, hoursBuffer, secsBuffer
; Output:   OutTime
;          
;**********************************************

FillOutTime 
            PSHA                          ; Save A
            PSHY                          ; Save Y
          
            LDY          #Terminate2      ; Load Y as a pointer to Terminate2
            ; Store second 
            LDAA         secsBuffer       ; Load A with the second time
            STAA         tempBuffer1      ; Store it to tempBuffer1
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            LDAA         #colon           ; Load A with a ASCII of colon
            STAA         1,-Y             ; Store it to OutTime
            
            ; Store minutes
            LDAA         minsBuffer       ; Load A with the minute time
            STAA         tempBuffer1      ; Store it to tempBuffer1
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            LDAA         #colon           ; Load A with a ASCII of colon
            STAA         1,-Y             ; Store it to OutTime
            
            ; Store Hours
            LDAA         hoursBuffer      ; Load A with the hour time
            STAA         tempBuffer1      ; Store it to tempBuffer1
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the remainder from tempBuffer2
            STAA         1,-Y             ; Store it to OutTime, from back to top
            
          
           
FOTDone     PULY                          ; Restore Y
            PULA                          ; Restore A
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; FormHEXtoDEC subroutine function
; 
; Function: Get time transfer from HEX into DEC
;       
; Input:    tempBuffer1 and tempBuffer2 
; Output:   Quotient in tempBuffer1, and remainder in tempBuffer2
;          
;**********************************************

FormHEXtoDEC
            PSHD                          ; Save D
            PSHX                          ; Save X

            LDD          temphead1        ; load D with temphead1
            LDX          #$0A             ; Load X with 10
            IDIV                          ; Integer Division
            STX          temphead1        ; Update the quotient in temp1
            ADDB         #ASCIInumBias     ; Add B with ASCIInumBias
            STAB         tempBuffer2      ; Store B   

           
FHDDone     PULX                          ; Restore X
            PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; GetDispNumber subroutine function
; 
; Function: It will transfer the hex number to another hex number
;            but present its decimal form in 7-segment display
;       
; Input:    time from tempBuffer1 
; Output:   7-segment diaplay hex in diapBuffer
;          
;**********************************************

GetDispNumber
            PSHA                          ; Save A

            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the first remainder from tempBuffer2
            SUBA         #ASCIInumBias     ; Since FormHEXtoDEC adds the ASCIInumBias to remainder.
                                          ; I don't need that bias, so we need to subtract that
            STAA         dispBuffer       ; Store it to the dispBuffer
            
            JSR          FormHEXtoDEC     ; Jump to FromHEXtoDEC
            LDAA         tempBuffer2      ; Get the second remainder from tempBuffer2
            SUBA         #ASCIInumBias     ; Since FormHEXtoDEC adds the ASCIInumBias to remainder.
                                          ; I don't need that bias, so we need to subtract that
            LSLA                          ; After getting the second remainder, we need to left shift 4 bits
            LSLA                          ;
            LSLA                          ;
            LSLA                          ; Now we have the tenth digital decimal number at the top four bits
            ADDA         dispBuffer       ; Add the first remainder to A, now the second digit of the decimal number at lower 4 bits
            STAA         dispBuffer       ; Store the result to dispBuffer
                      
GDNDone     PULA                          ; Restore A
            RTS                           ; Return 
            
***********************************************************************
;  Message Here
main_msg0   DC.B        'Welcome to HW8 program, you will experience a 24 hours clock. And below are some rules.', $00
main_msg1   DC.B        "1. We have 5 commands can use, they are:'t' for 'set time' command, 'q' for 'quit' command,", $00
main_msg2   DC.B        "   'h' for 'hour display' command, 'm' for 'minute display' command, and s for 'second display' command", $00
main_msg3   DC.B        '2. h, m ,s command will have time display on the 7-segment LED',$00
main_msg4   DC.B        "3. All commands have to be lower case for letters. and the time format has to be 'hh:mm:ss'", $00
main_msg5   DC.B        '4. There has to be a space after the letter you typed on command', $00
error_msg0  DC.B        'Invalid input format.', $00
error_msg1  DC.B        "Invalid command.", $00
end_msg     DC.B        '       Clock stopped and Typewrite program started.', $00
tw_msg0     DC.B        '       You may type below.', $00 

            END               ; this is end of assembly source file
                                 ; lines below are ignored - not assembled/compiled             
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            