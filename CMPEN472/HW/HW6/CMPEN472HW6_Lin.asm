***********************************************************************
*
* Title:          SCI Serial Port and Arithematic Instructions Use
*
* Objective:      CMPEN 472 HW6
*
* Date:	          10/03/2023
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
*                 Typewriter program and Arithmetic instruction use
*                 
* Algorithm:      Simple Serial I/O use, typewriter, command use, 
*                   and basic system I/O subroutine
*
* Register use:	  D,X,Y,A,B
*
* Memory use:     RAM Locations from $3000 for data, 
*                 RAM Locations from $3100 for program
*
* Output:         
*                 PORTB bit 7 to bit 4, LED 1,2,3,4. 
*                 PORTB bit 7 to 0 for 2 7-segment display
*                 
* Observation:    This is a  program will be able to let user to read 
*                 data from specific memory address and write data to specific
*                 memory address and this program will change to type writer program if 
*                 the users choose 'QUIT' command
*                   Command S: This command shows the contents of memory location 
*                              specified by the address in hexadecimal number followed 
*                              by the 'S' character
*                   Command W: this command writes data into the memory location 
*                              specified by the address in hexadecimal number followed 
*                              by the 'W' character. 
*                   QUIT: Quit menu program, and run type writrer program
*
***********************************************************************
*
* Parameter Declearation Section
*
* Export Stmbols
            XDEF        pstart        ; export 'pstart' symbol
            ABSENTRY    pstart        ; for assembly entry point
            
* Symbols and Macros
PORTB       EQU         $0001         ; i/o port b address
DDRB        EQU         $0003         ; 

SCIBDH      EQU         $00C8         ; Serial port(SCI) Baud Register H
SCIBDL      EQU         $00C9         ; Serial port(SCI) Baud Register L
SCICR2      EQU         $00CB         ; Serial port(SCI) Control Register 2
SCISR1      EQU         $00CC         ; Serial port(SCI) Status Register 1
SCIDRL      EQU         $00CF         ; Serial port(SCI) Data Register
                                                    
CR          EQU         $0D           ; Carriage return,ASCII 'Return' key
LF          EQU         $0A           ; Line feed, ASCII 'next line' character
S           EQU         $53           ; Letter S
W           EQU         $57           ; Letter W
Q           EQU         $51           ; Letter Q
space       EQU         $20           ; Space
ASCIInumBias EQU        $30           ; Set the ASCII number '0' to '9' bias as $30
ASCIIletterBias   EQU   $37           ; Set the ASCII number 'A' to 'F' bias as $37
***********************************************************************
*
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address 

Here        DS.B        $0D           ; Reserve 13 bytes
Buffer0     DC.B        $00           ; This Buffer0 will be used to constrct the address
Buffer1     DS.B        1             ; Reserve 1 byte for loading char
Buffer2     DS.B        2             ; Reserve 2 bytes for destinate addr
Buffer3     DS.B        5             ; Reserve 5 bytes for data in Decimal
Buffer4     DC.B        $00           ; This Buffer4 is used for terminate
; output message section      
Start       DC.B        $24         
Addr        DS.B        4     
            DC.B        ' = $'
Hexnum      DS.B        4
            DC.B        '    '
Decinum     DS.B        5
            DC.B        $00
; Output message section
                                      
Counter     DC.B        $00           ; Counter will record how many chars user typed
Counter1    DC.B        4             ; Set Counter1 = 4, for addr and Hex
Counter2    DC.B        5             ; Set Counter2 = 5, for decimal
ACheck      DC.B        $00           ; Set Check label, if Address error happen, it will set to one
TW_msg0     DC.B        '************* Ternimate Main Program, Loading Type Writer Program *************', $00
TW_msg1     DC.B        'Hello', $00
TW_msg2     DC.B        'You may type below', $00    


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
            ORG         $3100        ; Program start address, in RAM
pstart      LDS         #$3100       ; Initialize the stack pointer

            LDAA        #%11111111   ; Set PORTB bit 0,1,2,3,4,5,6,7
            STAA        DDRB         ; As output
            
            LDAA        #%00000000   ; Clear all bits of PORTB
            STAA        PORTB        
            
            LDAA        #$0C         ; Enable SCI port Tx and Rx units 
            STAA        SCICR2       ; Disable SCI interrupts
            
            LDD         #$0001       ; Set SCI Baud Register =  $0001 => 1.5M baud at 24MHz (for simulation)         
;            LDD         #$000D       ; Set SCI Baud Register = $009C =>   9600 baud at 24MHz                                  
            STD         SCIBDH       ; SCI port baud rate change$000D => 115200 baud at 24MHz
;            LDD         #$009C       ; Set SCI Baud Register = 
            
            JSR         printMenu    ; Print the menu

mainLoopinit LDY        #Here        ; Load Y with $3000
             LDAB       #$00         ; Load B with 0
             STAB       ACheck       ; Set Address error check byte to 0
             LDX        #0           ; Load register x with number 0
             STX        Buffer2      ; Initialize Buffer2 with 0
             
mainLoop    JSR         getchar      ; Check the ketboard if there is command typed in
            CMPA        #$00         ; If nothing typed, keep checking
            BEQ         mainLoop                
            
            CMPB        #$0D         ; Check if user enter more than 5 charcters
            LBEQ        overError    ; Print Error
            
            JSR         putchar      ; What is typed on key board is displayed on the terminal window - echo print 
                   
            STAA        1, Y+        ; Store the value in Accumulator A to address in Y
            INCB                     ; Increase B
            STAB        Counter      ; Store B to Counter
            
           
            CMPA        #CR          ; Detect if user do an 'enter'
            BNE         mainLoop     ; If Enter/Return key is pressed, load the saved
                                     ;  character and do command behaviors
                                     ;  if not, branch back to menuLoop

            LDAA        #LF          ; Cursor to next line
            JSR         putchar

            LDAA        Here         ; Load the first character to A
            CMPA        #S           ; Check if the first letter is S
            BEQ         S_command    ; Yes, branch to S_command
            
            CMPA        #W           ; Check if the first letter is W
            LBEQ         W_command    ; Yes, branch to W_command
            
            CMPA        #Q           ; Check if the first letter is Q
            LBEQ         Q_command 
            
            LBRA         commandError ; If none of the letter, branch to commandError

S_command
            LDAB        Counter      ; Counter record how many char the user types
            DECB                     ; Since we already check the first char, we need to decrement one
            
            CMPB        #$01         ; Compare B with number 1
            LBEQ         addrError0   ; Branch to address error 0
            
            CMPB        #$02         ; Compare B with 2
            LBEQ         addrError1   ; Branch to address error 1
 
            CMPB        #$06         ; Compare B wiht 6
            LBHI         addrError2   ; Branch tot address error 2 if B > 6
            
            
            LDAA        $3001        ; Load the second character
            CMPA        #$24         ; Check if the second char is '$'
            LBNE         addrError2   ; Branch to address error 2
            DECB                     ; Decrement B
            
            LDY         #$3002       ; load Y with address $3002
            JSR         SAddrWData   ; Jump to form address
            LDAA        ACheck       ; Load ACheck to A to see if error ouucr
            CMPA        #$01         ; Compare A with 1
            LBEQ        addrError3   ; Branch to print address error  
            
            JSR         FillandPrint ; Fill and print the output message
            BRA         mainLoopinit ; Branch to mainLoopinit
            

W_command
            LDAA        $3001        ; Load the second character
            CMPA        #$24         ; Check if the second char is '$'
            LBNE        addrError3   ; Branch to address error 3
            
            LDAA        $3002        ; Load the thrid character
            CMPA        #CR          ; Check if the third char id enter
            LBEQ        addrError3   ; Branch to address error 3

            LDY         #$3002       ; load Y with address $3002
            JSR         WFormAddrLoop ; Jump to form address
            LDAA        ACheck       ; Load ACheck to A to see if error ouucr
            CMPA        #$01         ; Compare A with 1
            LBEQ        mainLoopinit ; If A=1, error occur   
            
            ; try to put data into memory space 
            LDX         Buffer2      ; Load X with address
            LDD         #$00         ; Load D with 0
            STD         Buffer2      ; Clear Buffer2 for forming data
            
            LDAA        Y            ; Load what is in Y to A
            CMPA        #$24         ; Compare if A is a '$'
            BEQ         HexData      ; Branch to HexData
            
            JSR         WDecDataLoop ; Jump to WDecDataLoop to form data in Hex
            LDAA        ACheck       ; Load ACheck to A to see if error ouucr
            CMPA        #$01         ; Compare A with 1
            LBEQ        mainLoopinit ; If A=1, error occur
            BRA         StoreData    ; Branch to SoreData                     
                    
HexData
            INY                      ; Increase Y
            JSR         SAddrWData   ; Jump to SAddrWData to foem Hex data
            LDAA        ACheck       ; Load ACheck to A to see if error ouucr
            CMPA        #$01         ; Compare A with 1
            LBEQ        dataError0   ; If A=1, error occur
                                     ; else, store the data
            
StoreData   
            LDY         Buffer2      ; Load Data to Y
            STY         X            ; Store Data into the address in X, which we store the data to desinate address
            ; try to put data into memory space
            
            STX         Buffer2      ; Store address to Buffer2
            JSR         FillandPrint ; Fill and Print the output message
            LBRA        mainLoopinit

Q_command
            LDAA         $3001        ; Load the second character
            CMPA         #$55         ; check if the second character is 'U'
            BNE          quitError    ; If not 'U', go to error message
            
            LDAA         $3002        ; Load the third character
            CMPA         #$49         ; check if the third character is 'I'
            BNE          quitError    ; If not 'I', go to error message
            
            LDAA         $3003        ; Load the forth character
            CMPA         #$54         ; check if the forth character is 'T'
            BNE          quitError    ; If not 'T', go to error message
            
            LDAA         $3004        ; Load the forth character
            CMPA         #CR          ; check if the fifth character is return
            BNE          quitError    ; If not return, go to error message
            
            BRA          typewriter   ; Go to type Writer Program

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
            

overError
            JSR         CRandLF      ; Carrage return and Line feed
            JSR         printErrorO  ; Jump to printErrorO subroutine
            LBRA        mainLoopinit ; Go back to menuLoop  
            
commandError
            JSR         printErrorC  ; Jump to printErrorC subroutine
            LBRA        mainLoopinit ; Go back to menuLoop  

addrError0
            JSR         printErrorA0 ; Jump to printErrorA0 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop     

addrError1
            JSR         printErrorA1 ; Jump to printErrorA1 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop                 
                        
addrError2
            JSR         printErrorA2 ; Jump to printErrorA2 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop  
            
addrError3
            JSR         printErrorA3 ; Jump to printErrorA3 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop
            
dataError0    
            JSR         printErrorD0 ; Jump to printErrorD0 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop
            
quitError
            JSR         printErrorQ0 ; Jump to printErrorD0 subroutine
            LBRA        mainLoopinit ; Go back to menuLoop
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
            LDAA        #CR          ; move the cursor to beginning of the line
            JSR         putchar      ;   Cariage Return/Enter key
            LDAA        #LF          ; move the cursor to next line, Line Feed
            JSR         putchar      ;   Lien feed/next line      
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
            
            LDX          #main_msg6   ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg7   ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed
            
            LDX          #main_msg8   ; print the  message
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed

            PULX                      ; Restore X
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
            PSHX                      ; Save X
            LDX          #TW_msg0     ; Print start message
            JSR          printmsg           
            JSR          CRandLF      ; Carrage return and Line feed

            LDX          #TW_msg1     ; print the first message, 'Hello'
            JSR          printmsg           
            JSR          CRandLF      ; Carrage return and Line feed

            LDX          #TW_msg2     ; print the second message
            JSR          printmsg
            JSR          CRandLF      ; Carrage return and Line feed

            PULX                      ; Restore X
            RTS                       ; Return

***********************************************************************


***********************************************************************
;**********************************************
; printErrorO subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'overerror_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorO
            PSHX                         ; Save X     
            LDX          #overerror_msg  ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorC subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'commanderror_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorC
            PSHX                         ; Save X
            LDX          #commanderror_msg  ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorA0 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'addrerror0_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorA0
            PSHX                         ; Save X  
            LDX          #addrerror0_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorA1 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'addrerror1_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorA1
            PSHX                         ; Save X
            LDX          #addrerror1_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorA2 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'addrerror2_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorA2
            PSHX                         ; Save X
            LDX          #addrerror2_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed            
            
            PULX                      ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorA3 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'addrerror3_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorA3
            PSHX                         ; Save X
            LDX          #addrerror3_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorD0 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'dataerror0_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorD0
            PSHX                         ; Save X
            LDX          #dataerror0_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorQ0 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'quiterror0_msg '    
; Output:   Print error message
;          
;**********************************************

printErrorQ0
            PSHX                         ; Save X
            LDX          #quiterror0_msg ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; FormAddr subroutine function
; 
; Function: get the destinate address from what is user typed
;       
; Input:    Buffer2, and accumulator A 
; Output:   update Buffer2, the final destinate address
;          
;**********************************************

FormAddr
            STAA         Buffer1          ; Store the Actual HEX value to Buffer1
            
            PSHD                          ; Save register D
            LDD          Buffer2          ; Load D with what is in Buffer2
            
            LSLD                          ; Shift register left by 4 bits
            LSLD
            LSLD
            LSLD            
            
            ADDD         Buffer0          ; Add register D with Buffer0|Buffer1
            
            STD          Buffer2          ; Store value in register D to Buffer2

            PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; FormDECtoHex subroutine function
; 
; Function: get the destinate Decimal data from what is user typed to Hex
;       
; Input:    Buffer2, and accumulator A 
; Output:   update Buffer2, the final destinate address
;          
;**********************************************

FormDECtoHex
            STAA         Buffer1          ; Store the Actual HEX value to Buffer1
            
            PSHD                          ; Save D
            PSHY                          ; Save Y
            
            LDD          Buffer2          ; Load D with what is in Buffer2
            LDY          #$0A             ; Load Y with value 10
            
            EMUL                          ; Extend Multiply
            CPY          #$00             ; Compare Y with 0
            BNE          overflowError    ; If y not equal to 0, then error occur             
            
            ADDD         Buffer0          ; Add register D with Buffer0|Buffer1
            BCS          overflowError    ; If C = 1, then error occur
            
            STD          Buffer2          ; Store value in register D to Buffer2
            BRA          FDHDone
            
overflowError
            LDAA         #$01             ; Load A = 1
            STAA         ACheck           ; Set ACheck = 1, error occur

FDHDone     PULY                          ; Restore Y
            PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; SAddrWData subroutine function
; 
; Function: conver ascii hex to real hex number
;       
; Input:    address in S command and Hex data in W command
; Output:   buffer2
;          
;**********************************************
SAddrWData
            PSHD                     ; Save D 
            LDAB        Counter2     ; B = 5 
            
SAWDLoop    LDAA        1, Y+        ; Load A with the content in address Y 
            
            CMPA        #CR          ; Check if it is return 
            BEQ         SAWDDone     ; Yes, go to getData, all ASCII has been convert to real HEX number
            
            DECB                     ; Decrease B
            BEQ         SAWDaddrError3  ; Wrong addr, address is oversize     
            
            CMPA        #$30         ; Compare to '0'
            BLO         SAWDaddrError3   ; If A less than $30. then wrong addr
            
            CMPA        #$39         ; Compare to '9'
            BLE         ASCIInum     ; If A less and equal to $39, then it is a number between 0-9
            
            CMPA        #$41         ; Compare accumulator A with 'A'
            BLO         SAWDaddrError3   ; If Accumulator A is greater than $39 and less than $41, wrong addr
            
            CMPA        #$46         ; Compare accumulator A with 'F'
            BLE         ASCIIletter  ; If Accumulator >= $41 and <= $46, then it is a letter between A to F
            
            BRA         SAWDaddrError3   ; Accumular A > $46, wrong addr

ASCIInum    SUBA        #ASCIInumBias    ; Substract A with ASCII number bias to get real HEX value
            JSR         FormAddr     ; Jump to ASCIInumtoHEX
            BRA         SAWDLoop     ; keep looping            
            
ASCIIletter SUBA        #ASCIIletterBias    ; Substract A with ASCII number bias to get real HEX value
            JSR         FormAddr     ; Jump to ASCIInumtoHEX
            BRA         SAWDLoop  
            
SAWDaddrError3
            LDAA        #01          ; Load A with 1
            STAA        ACheck       ; Set Acheck byte to 1. error occur
            
SAWDDone    PULD                     ; Restore D          
            RTS                      ; Return
            
*********************************************************************** 


***********************************************************************
;**********************************************
; WFormAddrLoop subroutine function
; 
; Function: Loop the address to form address
;       
; Input:    address the user typed
; Output:   buffer2
;          
;**********************************************
WFormAddrLoop
            PSHD                     ; Save D
            LDAB        Counter2     ; B = 5 
            
WFALoop     LDAA        1, Y+        ; Load A with the content in address Y

            CMPA        #CR          ; Check if it is a return
            BEQ         WdataError0   ; Miss data 
            
            CMPA        #space       ; Check if it is space 
            BEQ         WFADone      ; Yes, go to getData, all ASCII has been convert to real HEX number
            
            DECB                     ; Decrease B
            BEQ         WaddrError3  ; Wrong addr, address is oversize            
            
            CMPA        #$30         ; Compare to '0'
            BLO         WaddrError3  ; If A less than $30. then wrong addr
            
            CMPA        #$39         ; Compare to '9'
            BLE         WASCIInum     ; If A less and equal to $39, then it is a number between 0-9
            
            CMPA        #$41         ; Compare accumulator A with 'A'
            BLO         WaddrError3   ; If Accumulator A is greater than $39 and less than $41, wrong addr
            
            CMPA        #$46         ; Compare accumulator A with 'F'
            BLE         WASCIIletter  ; If Accumulator >= $41 and <= $46, then it is a letter between A to F
            
            BRA         WaddrError3   ; Accumular A > $46, wrong addr

WASCIInum   SUBA        #ASCIInumBias    ; Substract A with ASCII number bias to get real HEX value
            JSR         FormAddr     ; Jump to ASCIInumtoHEX
            BRA         WFALoop      ; keep looping            
            
WASCIIletter SUBA        #ASCIIletterBias    ; Substract A with ASCII number bias to get real HEX value
            JSR         FormAddr     ; Jump to ASCIInumtoHEX
            BRA         WFALoop  

WdataError0
            JSR         printErrorD0 ; Jump to printErrorD0 subroutine
            LDAA        #01          ; Load A with 1
            STAA        ACheck       ; Set Acheck byte to 1. error occur
            BRA         WFADone       
            
WaddrError3
            JSR         printErrorA3 ; Jump to printErrorA3 subroutine
            LDAA        #01          ; Load A with 1
            STAA        ACheck       ; Set Acheck byte to 1. error occur
            
WFADone     PULD                     ; Restore D  
            RTS                      ; Return
            
***********************************************************************


***********************************************************************
;**********************************************
; WDecDataLoop subroutine function
; 
; Function: form the hex data from decimal data 
;       
; Input:    the decimal bytes the user typed
; Output:   buffer2
;          
;**********************************************
WDecDataLoop
            PSHD                     ; Save D
            LDAB        #$06         ; B = 6 
            
WDDLoop     LDAA        1, Y+        ; Load A with the content in address Y 
            
            CMPA        #CR          ; Check if it is return 
            BEQ         WDDDone      ; Yes, go to getData, all ASCII has been convert to real HEX number
            
            DECB                     ; Decrease B
            BEQ         WDDdataError0  ; Wrong data, address is oversize  
            
            CMPA        #$30         ; Compare to '0'
            BLO         WDDdataError0  ; If A less than $30. then wrong addr
            
            CMPA        #$39         ; Compare to '9'
            BLE         WDDASCIInum     ; If A less and equal to $39, then it is a number between 0-9
           
            
            BRA         WDDdataError0   ; Accumular A > $46, wrong addr

WDDASCIInum   
            SUBA        #ASCIInumBias    ; Substract A with ASCII number bias to get real HEX value
            JSR         FormDECtoHex ; Jump to ASCIInumtoHEX
            LDAA        ACheck       ; Load A with ACheck
            CMPA        #$01         ; If A = 0, overflow error occur
            BEQ         WDDdataError0  ; Print overflow error
            BRA         WDDLoop      ; keep looping            
            
WDDdataError0
            JSR         printErrorD0 ; Jump to printErrorC subroutine
            LDAA        #$01          ; Load A with 1
            STAA        ACheck       ; Set Acheck byte to 1. error occur
            
            
WDDDone     PULD                     ; Restore D  
            RTS                      ; Return
            
***********************************************************************  


***********************************************************************
;**********************************************
; DivGetHex subroutine function
; 
; Function: extra lowest bype from double accumulator D 
;       
; Input:    double accumulator D and number 16 ub register X
; Output:   Quotient in Buffer2 and Remainer in D, but actually in accumulator B
;          
;**********************************************

DivGetHex
            PSHD                           ; Save D
            PSHX                           ; Save X
            
            LDD         Buffer2            ; Load D with Buffer2 which is the address 
            LDX         #$10               ; Load X with number 16
            IDIV                           ; D / X, Quotient in X and Remainder in D
            STX         Buffer2            ; Update the quotient
            
            CMPB        #$09               ; Compare remainder with number 9
                                           ; up to now, the number we checked will be 0 - F only
            BLS         numbias            ; number detected, branch to numbias to add bias
            
            ADDB        #ASCIIletterBias   ; Add letter bias
            BRA         storeB             ; Branch to storeB

numbias     ADDB        #ASCIInumBias      ; Add number bias

storeB      STAB        Buffer1            ; Store B to buffer1

            PULX                           ; Restore X 
            PULD                           ; Restore D 
            RTS                            ; Return
            
***********************************************************************  


***********************************************************************
;**********************************************
; DivGetDec subroutine function
; 
; Function: get the lowest decimal number from D 
;       
; Input:    double accumulator D and number 10 in register X
; Output:   Quotient in Buffer2 and Remainder will be stored in Buffer1
;          
;**********************************************

DivGetDec
            PSHD                           ; Save D
            PSHX                           ; Save X
            
            LDD         Buffer2            ; Load D with Buffer2 which is the address 
            LDX         #$0A               ; Load X with number 10
            IDIV                           ; D / X, Quotient in X and Remainder in D
            STX         Buffer2            ; Update the quotient
            ADDB        #ASCIInumBias      ; Add number bias
            STAB        Buffer1            ; Store B to buffer1

            PULX                           ; Restore X 
            PULD                           ; Restore D 
            RTS                            ; Return
            
***********************************************************************


***********************************************************************
;**********************************************
; FO subroutine function
; 
; Function: fill out the output message
;       
; Input:    Buffer 1,2   
; Output:   output message filled 
;          
;**********************************************

FO    
            PSHD                     ; Save D
            
            LDAA        Counter1     ; Load A with counter1
            
FOLoop      JSR         DivGetHex    ; Jump to DivGetHex to extract lowest byte
            LDAB        Buffer1      ; Load the remainder to accumulator B
            STAB        1, Y-        ; Store to address part of output
            DECA                     ; Decrease A
            BEQ         FODone       ; If X = 0 branch to FODone
            BRA         FOLoop       ; Branch to fill in data in HEX

FODone      PULD                     ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; fillBuffer3 subroutine function
; 
; Function: fill out the Buffer3 with decimal number output
;       
; Input:    Buffer 1,2   
; Output:   Buffer3
;          
;**********************************************

fillBuffer3    
            PSHD                     ; Save D
            
            LDAA        Counter2     ; Load A with counter1
            
fBloop      JSR         DivGetDec    ; Jump to DivGetDec to extract lowest byte
            LDAB        Buffer1      ; Load the remainder to accumulator B
            STAB        1, Y-        ; Store to address part of output
            DECA                     ; Decrease A
            BEQ         fBdone       ; If X = 0 branch to FODone
            BRA         fBloop       ; Branch to fill in data in HEX

fBdone      PULD                     ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; FillDecimal subroutine function
; 
; Function: fill out the decimal place in output message
;       
; Input:    Buffer 3   
; Output:   decinal place in output message
;          
;**********************************************

FillDecimal    
            PSHX                     ; Save X     
            PSHD                     ; Save D
            LDAA        Counter2     ; Load A with Counter2 = 5
            LDX         #Buffer3     ; Load X with address of Buffer3
FDLoop1     LDAB        1,X+         ; Load B with the value in address in X
            CMPB        #$30         ; We are looking for the first non zero number
            BNE         FDLoop2      ; Keep looking for the first non zero number
            DECA                     ; Decrease A
            BNE         FDLoop1      ; Keep Looping
                                     ; If we don't get to FDLoop2, it means that the input decimal number is 0
                                     ; Then fill one 0 into decimal place in output message
            STAB        Y            ; Store one 0 to address in Y which is the first byte in decimal place in output
            BRA         FDDone       ; Branch to FDCone
            
FDLoop2     STAB        1, Y+        ; Store B to Address in Y
            DECA                     ; Decrease A
            BEQ         FDDone       ; If A = 0; branch to FDDone
            LDAB        1, X+        ; Load the next byte
            BRA         FDLoop2      ; Branch to FDLoop2
            

FDDone      PULD                     ; Restore D
            PULX                     ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; ClearDecinum subroutine function
; 
; Function: reset all bytes in Buffer3
;       
; Input:    Decinum   
; Output:   Decinum memory space with all 0
;          
;**********************************************

ClearDecinum
            PSHD                     ; Save D
            PSHY                     ; Save Y
            
            LDAA        Counter2     ; Load A with Counter2=5
            LDY         #Decinum     ; Load the address Decinum to Y
            LDAB        #$00         ; Load B with 0
            
CDLoop      STAB        1, Y+        ; Clear the bytes in Y
            DECA                     ; Decrease A
            BNE         CDLoop       ; IF A not 0, keeping Looping
              
            
            PULY                     ; Restore Y
            PULD                     ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; FillandPrint subroutine function
; 
; Function: Fill and Print the outputmsg 
;       
; Input:    Buffer2   
; Output:   Filled and Print output msg
;          
;**********************************************

FillandPrint  
            PSHX                     ; Save X
            PSHY                     ; Save Y
            
            LDX         Buffer2      ; Keep the address in X, Since buffer2 will be wape out later
            LDY         #$301B       ; Load Y with #Addr, start to fill in address in output
            JSR         FO           ; Jump to fill output, address
            ; Start to fill data part of output
            LDY         X            ; Load Y with the address
            STY         Buffer2      ; Update Buffer2 as data
            LDX         Buffer2      ; Also keep the data in X, since Buffer2 will be wape out
            LDY         #$3023       ; Load Y with #Data, start to fill in data of output
            JSR         FO           ; Jump to fill output, data
            ; Start to fill decimal part of the output 
            STX         Buffer2      ; Restore data back to Buffer2
            LDY         #$3015     ; Load Y with address of Buffer3
            JSR         fillBuffer3  ; Fill in Buffer3
            LDY         #Decinum     ; Start ti fill out the decimal place in output
            JSR         ClearDecinum ; Clear all bytes before fill in decimal place
            JSR         FillDecimal  ; Jump to fill decimal 
            
            ; Time to Ptint Output
            LDX         #Start       ; Load X with Start address
            JSR         printmsg     ; Print output
            JSR         CRandLF      ; Carriage return and Line feed
            
            PULY                     ; Restore Y
            PULX                     ; Restore X
            RTS                      ; Return
            
*********************************************************************** 

; More Message Here
main_msg0   DC.B        'Welcome to HW6 program', $00
main_msg1   DC.B        'We supply three commands for you to use', $00
main_msg2   DC.B        '1: S$XXXX (This command will show the contents of memory location in word)', $00
main_msg3   DC.B        '2: W$XXXX [#number in decimal or hex] (This command write the data word to memory location)', $00
main_msg4   DC.B        "3: QUIT (Quit the main program, run 'type writer' program)", $00
main_msg5   DC.B        'Note:', $00
main_msg6   DC.B        'a. All letters in the address or number present in Hex need to be CAPITALIZED', $00
main_msg7   DC.B        'b. When put the data to memory (using W command), the data that put in can be entered as decimal or Hex number', $00
main_msg8   DC.B        'c. The data that put into the memory has to be 16 bit usigned number', $00
overerror_msg   DC.B    'Invalid Command or data is too long', $00 
commanderror_msg   DC.B    "Invalid Command used, Please use 'S', 'W'. or 'QUIT' command", $00  
addrerror0_msg    DC.B  "Invalid Input, missing '$' and actual address number", $00
addrerror1_msg    DC.B  'Invalid Input, missing actual address number', $00 
addrerror2_msg    DC.B  "Invalid Input, please input valid address format, like '$xxxx'", $00
addrerror3_msg    DC.B  'Invalid Input, address', $00        
dataerror0_msg    DC.B  'Invalid Input, data', $00 
quiterror0_msg    DC.B  'Invalid Quit Command', $00
            END               ; this is end of assembly source file
                                 ; lines below are ignored - not assembled/compiled  















