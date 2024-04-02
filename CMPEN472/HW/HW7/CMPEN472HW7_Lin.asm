***********************************************************************
*
* Title:          SCI Serial Port and Elementary Calculator
*
* Objective:      CMPEN 472 HW7
*
* Date:	          10/11/2023
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
* Program:        Simple SCI Serial Port I/O and Elementary calculator, displayed on 
*                 the HyperTerminal connected to HCS12 board
*                 
* Algorithm:      Simple Serial I/O use, Arithmetic instructions use, simple command 
*                   line parsing and basic system I/O subroutine
*
* Register use:	  Accumulator: A and B: used as ckecking and passing values to buffer, output message.
*                 X: address pointer. extend division
*                 Y: address pointer, extend multiplication
*                 D: arithmetic calculation, add, divide, multiply, subtraction.
*
* Memory use:     RAM Locations from $3000 for data, 
*                 RAM Locations from $3100 for program
*
* Output:         
*                 Terminal Window
*                 
* Observation:    This is a  program will perform a basic elementary calculation. 
*                 Below are some rules:
*                   1. Input positive decimal integer numbers only
*                   2. Input and output maximum four digit numbers only
*                   3. Valid operators are: +, -, *, and /
*                   4. Input number with leading 0 is OK
*                   5. Input only two numbers and one operator in between, no spaces
*                   6. Show 'Ecalc> ' prompt and echo print user keystrokes until Return key
*                   7. Repeat print user input and print answer after the '=' sign
*                   8. In case of an invalid input format, repeat print the user input until the error char
*                   9. In case of an invalid input format, print error message on the next line:'Invalid input format'
*                  10. Keep 16 bit internal binary number format, detect and flag overflow error
*                  11. Use integer division and truncate any fraction
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
add         EQU         $2B           ; Add sign
minus       EQU         $2D           ; Minus sign
multiply    EQU         $2A           ; Multiplication sign
divide      EQU         $2F           ; division sign
equal       EQU         $3D           ; Equal sign  
ASCIInumBias EQU        $30           ; Set the ASCII number '0' to '9' bias as $30
***********************************************************************
*
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address 

Here        DS.B        $0B           ; Reserve 11 bytes
Ternimate1  DC.B        $00           ; Ternimate Byte

; output message part
Start       DS.B        $0F           ; Reserve 15 bytes maximum for correct output print
Terminate2  DC.B        $00           ; Ternimate byte
; output message part

Counter1    DC.B        4             ; Set Counter1 = 4 that will be able to check the length of input number
Counter2    DC.B        5             ; Set Counter2 = 5 that will use in Num1 detect for oversize
ACheck      DC.B        $00           ; Set check label, if error happen, it will set to one
Negsign     DC.B        $00           ; Set negative result label, if result is negative, it will set to one

headBuffer  DC.B        $00           ; Set headBuffer as $00, used for Accumulator D + headerBuffer|hexbuffer
hexBuffer   DS.B        1             ; Reserve 1 byte for store hex number after transfer from ASCII
addrBuffer  DS.B        2             ; Reserve 2 bytes for address in where we at in user input
numBuffer1  DS.B        2             ; Reserve 2 bytes for first number that present in Hex
numBuffer2  DS.B        2             ; Reserve 2 bytes for second number 
resultBuffer  DS.B      2             ; Reserve 2 bytes for result number
errorBuffer DS.B        1             ; Reserve 1 byte for error checking
addrBuffer2 DS.B        2             ; Reserve 2 bytes for address in output message
operatorBuffer  DS.B    1             ; Reserve 1 bytes for store operator
resultASCIIBuffer   DS.B  4           ; Reserve 4 Bytes for ASCII number preserntation of result
Terminate3  DC.B        $00           ; Ternimate byte


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

            LDAB       #$00         ; Load A with value 0
            LDX        #$00         ; Load X with value 0
            STAB       ACheck       ; Reset ACheck 
            STAB       Negsign      ; Reset Negsign
            STAB       errorBuffer  ; Clean errorBuffer
            STX        addrBuffer   ; Clean addrBuffer
            STX        addrBuffer2  ; Clean addrBuffer2
            STX        numBuffer1   ; Clean numBuffer1
            STX        numBuffer2   ; Clean numBuffer2
            STX        resultBuffer ; Clean resultBuffer
            JSR        CleanOutMsg  ; Clean output message
             
            LDX        #prompt      ; print the prompt message
            JSR        printmsg
             
mainLoop    JSR         getchar      ; Check the ketboard if there is command typed in
            CMPA        #$00         ; If nothing typed, keep checking
            BEQ         mainLoop                
            
            CMPB        #$0B         ; Check if user enter more than 10 charcters
            LBEQ        Error0       ; Print Error
            
            JSR         putchar      ; What is typed on key board is displayed on the terminal window - echo print 
                   
            STAA        1, Y+        ; Store the value in Accumulator A to address in Y
            INCB                     ; Increase B
            
           
            CMPA        #CR          ; Detect if user do an 'enter'
            BNE         mainLoop     ; If Enter/Return key is pressed, load the saved
                                     ;  character and do command behaviors
                                     ;  if not, branch back to menuLoop
            ; first digit check, has to be a number from 0 - 9
            LDY         #Here        ; Load Y as the pointer that point to address Here
            LDAA        1, Y+        ; Load A with the value in Y
            CMPA        #CR          ; If the first byte is return
            BEQ         mainLoopinit ; Branch back to mainLoopinit
            CMPA        #$39         ; Compare A with $39, ASCII number 9
            BHI         Error1       ; If A is > $39, then it is a wrong input
            CMPA        #$30         ; Compare A with $30, ASCII number
            BLO         Error1       ; If A < 30, then is a wrong input
                                                 
            ; Num1 form hex part                         
            LDY         #Here        ; Load Y as the pointer that point to address Here
            JSR         Num1Loop     ; Jump to Num1Loop
            LDAB        ACheck       ; Load B with ACheck
            CMPB        #$00         ; Chekc if B = 0
            BNE         Error1       ; If not 0, means error occur
            
            ; Num2 form hex part
            JSR         Num2Loop     ; Jump to Num2Loop
            LDAB        ACheck       ; Load B with ACheck
            CMPB        #$00         ; Chekc if B = 0
            BNE         Error1       ; If not 0, means error occur
            
            ; Getting which operator is
            STY         addrBuffer   ; Now Y has the address that after the return of user input
           
            LDAB        operatorBuffer    ; Load B with the operator that stored in operatorBuffer
            CMPB        #add         ; Check if it is an addition
            BEQ         Add          ; Jump to Add
            
            
            
            CMPB        #minus       ; Check if it is a minus  
            BEQ         Minus        ; Jump to Minus
            
            CMPB        #multiply    ; Chekc if it is a multiply
            BEQ         Multiply     ; Jump to Multiply
            
            CMPB        #divide      ; Chekc if it is a divide
            BEQ         Divide
            
Add
            JSR         Addition     ; Jump to Addition 
            BRA         PrintOutput  ; Branch to print output
            
Minus
            JSR         Subtraction  ; Jump to Subtraction
            BRA         PrintOutput  ; Branch to PrintOutput
            
Multiply
            JSR         Multiplication  ; Jump to Multiplication
            BRA         PrintOutput  ; Branch to PrintOutput

Divide            
            JSR         Division     ; Jump to Division
            BRA         PrintOutput  ; Branch to PrintOutput

PrintOutput
            LDAB        ACheck       ; Load B with ACheck
            CMPB        #$00         ; Chekc if B = 0
            BNE         Error2       ; If not 0, means error occur, overflow
            
            JSR         FillOutput   ; Jump to fill output message
            LDAB        ACheck       ; Load B with ACheck
            CMPB        #$00         ; Chekc if B = 0
            BNE         Error2       ; If not 0, means error occur, overflow
            LDX         #Start       ; Load address of output to X
            JSR         printmsg     ; Print Output
            JSR         CRandLF      ; Print return and line feed
            LBRA        mainLoopinit ; Branch back to mainLoopInit


Error0      JSR         CRandLF      ; Carrage return and Line feed
Error1      STY         addrBuffer   ; Store current address intot address buffer
            JSR         FillError    ; Jump tot fill error into output message
            JSR         printErrorInput ; Jump to print error input
            JSR         printError0  ; Jump to printErrorO subroutine
            LBRA        mainLoopinit ; Go back to menuLoop  

Error2      JSR         FillError    ; Jump tot fill error into output message
            JSR         printErrorInput ; Jump to print error input
            JSR         printError1  ; Jump to printErrorO subroutine
            LBRA        mainLoopinit ; Go back to menuLoop  
; Subroutine Section Below
***********************************************************************
;**********************************************
; FormDECtoHEX1 subroutine function
; 
; Function: Get Num1 transfer into HEX into numBuffer1
;       
; Input:    numBuffer1, and accumulator A 
; Output:   update numBuffer1, the final hex presentation of num1 
;          
;**********************************************

FormDECtoHEX1
            STAA         hexBuffer        ; Store the Actual HEX value to hexBuffer
            
            PSHD                          ; Save D
            PSHY                          ; Save Y
            
            LDD          numBuffer1       ; Load D with what is in numBuffer1
            LDY          #$0A             ; Load Y with value 10
            
            EMUL                          ; Extend Multiply
            CPY          #$00             ; Compare Y with 0
            BNE          overflowError1   ; If y not equal to 0, then error occur             
            
            ADDD         headBuffer       ; Add register D with HeadBuffer|hexBuffer
            BCS          overflowError1   ; If C = 1, then error occur
            
            STD          numBuffer1       ; Store value in register D to numBuffer1
            BRA          FDH1Done
            
overflowError1
            LDAA         #$01             ; Load A = 1
            STAA         ACheck           ; Set ACheck = 1, error occur

FDH1Done    PULY                          ; Restore Y
            PULD                          ; Restore D
            RTS                           ; Return 
            
***********************************************************************


***********************************************************************
;**********************************************
; Num1Loop subroutine function
; 
; Function: make num1 from decimal digits to hex form and also detect potentional error
;       
; Input:    the decimal digits   
; Output:   num1Buffer
;          
;**********************************************

Num1Loop
            PSHD                         ; Save D
            LDAB        Counter2         ; Load B = 5
                 
N1Loop      LDAA        1, Y+            ; Read the value in address Y into A
            
            CMPA        #add             ; Check if it is a add sign
            BEQ         operator_sign    ; Branch to operator_sign
            
            CMPA        #minus           ; Check if it is a minus sign
            BEQ         operator_sign    ; Branch to operator_sign
            
            CMPA        #multiply        ; Check if it is a multiply sign
            BEQ         operator_sign    ; Branch to operator_sign
            
            CMPA        #divide          ; Check if it is a divide sign
            BEQ         operator_sign    ; Branch to operator_sign
            
            DECB                         ; Decrease B
            BEQ         N1Error          ; Decinmal digits are oversize, more than 4 digits 
            
            CMPA        #$39             ; Compare A with $39, ASCII number 9
            BHI         N1Error          ; If A is > $39, then it is a wrong input
                                     
            CMPA        #$30             ; Compare A with $30, ASCII number 0
            BHS         FormNum1         ; Branch to formNum1
            
            BRA         N1Error          ; Branch to N1Error
            
            

FormNum1    SUBA        #ASCIInumBias    ; Substract the bias value
            JSR         FormDECtoHEX1    ; Jump to FormDECtoHEX1 function
            LDAA        ACheck           ; Load A with ACheck
            CMPA        #$00             ; Compare A with 0
            BNE         N1Done           ; Branch to N1Done
            BRA         N1Loop           ; Branch back to N1Loop

operator_sign
            ;STY         addrBuffer       ; Store Y to addrBuffer
            CMPB        #$05             ; COmpare B with 5
            BEQ         N1Error          ; If yes, means no number 1 is entered
            STAA        operatorBuffer   ; Store the operator
            BRA         N1Done           ; Branch to N1Done
            
N1Error     LDAA        #$01             ; Load A = 1
            STAA        ACheck           ; Set ACheck = 1, error occur
            
            
N1Done      PULD                         ; Restore D
            RTS
            
***********************************************************************


***********************************************************************
;**********************************************
; FormDECtoHEX2 subroutine function
; 
; Function: Get Num1 transfer into HEX into numBuffer12
;       
; Input:    numBuffer1, and accumulator A 
; Output:   update numBuffer2, the final hex presentation of num1 
;          
;**********************************************

FormDECtoHEX2
            STAA         hexBuffer        ; Store the Actual HEX value to hexBuffer
            
            PSHD                          ; Save D
            PSHY                          ; Save Y
            
            LDD          numBuffer2       ; Load D with what is in numBuffer2
            LDY          #$0A             ; Load Y with value 10
            
            EMUL                          ; Extend Multiply
            CPY          #$00             ; Compare Y with 0
            BNE          overflowError2   ; If y not equal to 0, then error occur             
            
            ADDD         headBuffer       ; Add register D with HeadBuffer|hexBuffer
            BCS          overflowError2   ; If C = 1, then error occur
            
            STD          numBuffer2       ; Store value in register D to numBuffer2
            BRA          FDH2Done
            
overflowError2
            LDAA         #$01             ; Load A = 1
            STAA         ACheck           ; Set ACheck = 1, error occur

FDH2Done    PULY                          ; Restore Y
            PULD                          ; Restore D
            RTS                           ; Return 
            
*********************************************************************** 


***********************************************************************
;**********************************************
; Num2Loop subroutine function
; 
; Function: make num2 from decimal digits to hex form and also detect potentional error
;       
; Input:    the decimal digits   
; Output:   num1Buffer
;          
;**********************************************

Num2Loop
            PSHD                         ; Save D
            LDAB        Counter2         ; Load B = 5
                 
N2Loop      LDAA        1, Y+            ; Read the value in address Y into A
            
            CMPA        #CR              ; Check if it is a return
            BEQ         N2Check          ; Yes, branch to N2Check
            
            DECB                         ; Decrease B
            BEQ         N2Error          ; Decinmal digits are oversize, more than 4 digits 
            
            CMPA        #$39             ; Compare A with $39, ASCII number 9
            BHI         N2Error          ; If A is > $39, then it is a wrong input
                                     
            CMPA        #$30             ; Compare A with $30, ASCII number 0
            BHS         FormNum2         ; if A >= $30, Branch to formNum1
            
            BRA         N2Error          ; Branch to N1Error
            
            

FormNum2    SUBA        #ASCIInumBias    ; Substract the bias value
            JSR         FormDECtoHEX2    ; Jump to FormDECtoHEX2 function
            LDAA        ACheck           ; Load A with ACheck
            CMPA        #$00             ; Compare A with 0
            BNE         N2Done           ; Branch to N1Done
            BRA         N2Loop           ; Branch back to N1Loop

N2Check     CMPB        #$05             ; Compare B with 5
            BEQ         N2Error          ; If yes, means there is no number entered for num2
            BRA         N2Done           ; Else,it is ok, branch to N2Done
            
N2Error     LDAA        #$01             ; Load A = 1
            STAA        ACheck           ; Set ACheck = 1, error occur
            
            
N2Done      PULD                         ; Restore D
            RTS
            
***********************************************************************


***********************************************************************
;**********************************************
; DivGetDec subroutine function
; 
; Function: get the lowest decimal number from D 
;       
; Input:    double accumulator D and number 10 in register X
; Output:   Quotient in resultBuffer and Remainder will be stored in hexBuffer
;          
;**********************************************

DivGetDec
            PSHD                           ; Save D
            PSHX                           ; Save X
            
            LDD         resultBuffer       ; Load D with Buffer2 which is the address 
            LDX         #$0A               ; Load X with number 10
            IDIV                           ; D / X, Quotient in X and Remainder in D
            STX         resultBuffer       ; Update the quotient
            ADDB        #ASCIInumBias      ; Add number bias
            STAB        hexBuffer          ; Store B to hexBuffer, actually ASCII preserntation now 

            PULX                           ; Restore X 
            PULD                           ; Restore D 
            RTS                            ; Return
            
***********************************************************************


***********************************************************************
;**********************************************
; FillASCIIBuffer subroutine function
; 
; Function: transfer the hex result to ASCII result and fill in the resultASCIIBuffer
;       
; Input:    resultBuffer
; Output:   resultASCIIBuffer
;          
;**********************************************

FillASCIIBuffer            
            PSHD                         ; Save D
            PSHY                         ; Save Y
            
            LDAB        Counter1         ; Load B = 4
            LDY         #Terminate3      ; Load Y as the pointer that points to hte ternimate of  resultASCIIBuffer
            
FABLoop     JSR         DivGetDec        ; jump to get decimal digit
            LDAA        hexBuffer        ; Get hte decimal digital
            STAA        1, -Y            ; put ASCII decimal to resultASCIIBuffer from back to top
            DECB                         ; Decrease B
            BEQ         checkoverflow    ; If B = 0, branch to checkoverflow
            BRA         FABLoop          ; Keep branching
                              ;   
checkoverflow
            LDY         resultBuffer     ; Load Y with the left quotient
            CPY         #$00             ; Compare Y with 0, it should be 0 is no overflow happens
            BNE         OverFlow         ; If not 0, means overflow occur, branch to OverFlow
            BRA         FABDone          ; If 0, no error, branch to FABDone  
            
OverFlow    LDAA        #$01             ; Load A = 01
            STAA        ACheck           ; Set ACheck = 1, error occur, overflow            
           
FABDone     PULY                         ; Restore Y
            PULD                         ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; FillOutput subroutine function
; 
; Function: Fill and print the output message
;       
; Input:    resultBuffer and Input message
; Output:   Fill and Print the output msg
; Register: B:counter 
;            A: Checking and value 
;            X: address of resultASCIIBuffer
;            Y: output address        
;**********************************************

FillOutput
            PSHX                     ; Save X
            PSHY                     ; Save Y
            PSHD                     ; Save D
            LDAB        Counter1     ; Load B = 4
            
            
            JSR         FillASCIIBuffer     ; Jump to  FillASCIIBuffer
            LDAA        ACheck       ; Check if error occur
            CMPA        #$00         ; Check with 0 
            BNE         FODone       ; If not 0, overflow error occur
            
            JSR         FillError    ; Jump to fill error, it is actually put all user input 
                                     ; into output message
            LDY         addrBuffer2  ; Load the address of output to continue
            LDAA        #equal       ; Load A with equal sign
            STAA        1,Y+         ; Store equal sign to output message 
            
            LDAA        Negsign      ; Check if the result is negative number
            CMPA        #$00         ; If Negsign = 0, mean positive result
            BEQ         FOLoopInit   ; Branch to FOLoopInit
            
            LDAA        #minus       ; Load A with minus ASCII number
            STAA        1,Y+         ; Put minus sign to output message
             
            
FOLoopInit  LDX         #resultASCIIBuffer ; Load X as the pointer points to the resultASCIIBuffer
FOLoop1     LDAA        1, X+        ; Load A with the value in address X
            CMPA        #$30         ; We are looking for the first non zero number
            BNE         FOLoop2      ; Keep looking for the first non zero number
            DECB                     ; Decrease B
            BNE         FOLoop1      ; Keep Looping
                                     ; If we don't get to FOLoop2, it means that the input decimal number is 0
                                     ; Then fill one 0 into decimal place in output message
            STAA        Y            ; Store one 0 to address in Y which is the first byte in decimal place in output
            BRA         FODone       ; Branch to FDCone
            
FOLoop2     STAA        1,Y+         ; Store A to Address in Y
            DECB                     ; Decrease B
            BEQ         FODone       ; If B = 0, branch to FODone
            LDAA        1,X+         ; Load the next decimal digit
            BRA         FOLoop2      ; Branch to FDLoop2                 
            
            
FODone      PULD                     ; Restore D
            PULY                     ; Restore Y
            PULX                     ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; Addition subroutine function
; 
; Function: Add numBuffer1 and numBuffer2 to resultBuffer
;       
; Input:    numBuffer1, and numBuffer2
; Output:   resultBuffer
;          
;**********************************************

Addition
            PSHD                         ; Save D
            
            LDD         numBuffer1       ; Load D with numBuffer1
            ADDD        numBuffer2       ; Add num1 and num2 in hex
            BCS         addOverError     ; Branch to overflow error
            STD         resultBuffer     ; Store result
            BRA         ADone            ; If no error, branch to ADone
            
addOverError
            LDAA        #$01             ; Load A = 1
            STAA        ACheck           ; Set ACheck = 1, overflow error occur
            
ADone       PULD                         ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; Subtraction subroutine function
; 
; Function: Subtraction numBuffer1 by numBuffer2 to resultBuffer
;       
; Input:    numBuffer1, and numBuffer2
; Output:   resultBuffer
;          
;**********************************************

Subtraction
            PSHD                         ; Save D
            
            LDD         numBuffer1       ; Load D with numBuffer1
            SUBD        numBuffer2       ; Subtract num1 by num2
            BCS         NegResult        ; If C = 1, negtive result, borrow occur
            STD         resultBuffer     ; If C = 0, positive result
            BRA         SUBDone          ; Branch to SUBDone

NegResult            
            COMA                         ; One's complement A 
            COMB                         ; One;s Complement B
            ADDD        #$01             ; Add one to D to perform two's complement
            STD         resultBuffer     ; Store to result
            LDAA        #$01             ; Load A with value 1
            STAA        Negsign          ; Set Negsign to 1, negative result occur
            
            
SUBDone     PULD                         ; Restore D
            RTS
            
*********************************************************************** 



***********************************************************************
;**********************************************
; Multiplication subroutine function
; 
; Function: multiply numBuffer1 and NumBuffer2 to resultBuffer
;       
; Input:    numBuffer1, and numBuffer2
; Output:   resultBuffer
;          
;**********************************************

Multiplication
            PSHD                         ; Save D
            PSHY                         ; Save Y
            
            LDD         numBuffer1       ; Load D with numBuffer1
            LDY         numBuffer2       ; Load Y with numBuffer2
            EMUL                         ; Multiply num1 and num2
            CPY         #$00             ; Compare Y with 0
            BNE         MULError         ; Overflow Error occur
            STD         resultBuffer     ; Store result 
            BRA         MULDone          ;
                       
MULError    LDAA        #$01             ; Load A with 1
            STAA        ACheck           ; Set ACheck = 1, overflow error occur
            
            
MULDone     PULY                         ; Restore Y
            PULD                         ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; Division subroutine function
; 
; Function: divide numBuffer1 and NumBuffer2 to resultBuffer
;       
; Input:    numBuffer1, and numBuffer2
; Output:   resultBuffer
;          
;**********************************************

Division
            PSHD                         ; Save D
            PSHX                         ; Save X
            
            LDD         numBuffer1       ; Load D with numBuffer1
            LDX         numBuffer2       ; Load Y with numBuffer2
            IDIV                         ; Divide num1 by num2           
            STX         resultBuffer     ; Store result 
                   
            PULX                         ; Restore X
            PULD                         ; Restore D
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printError0 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'error_msg0'    
; Output:   Print error message
;          
;**********************************************

printError0
            PSHX                         ; Save X     
            LDX          #error_msg0     ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printError1 subroutine function
; 
; Function: Print error message
;       
; Input:    error message 'error_msg1'    
; Output:   Print error message
;          
;**********************************************

printError1
            PSHX                         ; Save X     
            LDX          #error_msg1     ; If none of the condition meet, means wrong command entered
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; printErrorInput subroutine function
; 
; Function: Print the input until where goes wrong 
;       
; Input:    ouptut message    
; Output:   Print error message
;          
;**********************************************

printErrorInput
            PSHX                         ; Save X     
            LDX          #Start          ; Load X with the address that point tot the start of output message
            JSR          printmsg        ; Print the error message and end the program
            JSR          CRandLF         ; Carrage return and Line feed
            
            PULX                         ; Restore X
            RTS
            
*********************************************************************** 


***********************************************************************
;**********************************************
; CleanOutMsg subroutine function
;
; Function: it will set the memory address from $300A to $3017 to $00
; Input:    address of output message part
; Output:   all $00 in output message
; Register: register Y, accumulator A       
;**********************************************           

CleanOutMsg     
            PSHA                      ; Save A  
            PSHY                      ; Save Y
            
            LDY         #Start        ; Load Y as a pointer to output address
COMLoop     LDAA        Y             ; Load A with the value that contained in address in Y
            
            CMPA        #$00          ; Check if it is a null 
            BEQ         COMDone       ; Branch to COMDone  
            
            LDAA        #$00          ; Load A with 0
            STAA        Y             ; Store 0 to the address in Y
            INY                       ; Increase Y
            BRA         COMLoop       ; Branch back to keep cleaning the entire output message        
                 
COMDone     PULY                      ; restore Y
            PULA                      ; Get stored A  
            RTS
            
***********************************************************************

            
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

            PULX                      ; Restore X
            RTS                       ; Return

***********************************************************************


***********************************************************************
;**********************************************
; FillError subroutine function
; 
; Function: Fill the error part into the output message
;       
; Input: chars that user typed        
; Output: output message in data section   
;          
;**********************************************
FillError
            PSHY                      ; Save Y
            PSHX                      ; Save X
            PSHA                      ; Save A

            LDY          #Here        ; Load Y with Address of Here
            LDX          #Start       ; Load X with the start address of output section
             
FELoop      LDAA         1, Y+        ; Load A with the value in address Y
            CMPA         #CR          ; Check If A is a return 
            BEQ          FEDone       ; yes, Branch to FEDone
            STAA         1, X+        ; Store A intot the value in address X
            CPY          addrBuffer   ; Compare if Y reach the address that is in the buffer
            BEQ          FEDone       ; Branch to FEDone
            BRA          FELoop       ; Branch to FELoop

FEDone      STX          addrBuffer2  ; Store the address that points to where the output message has filled
            PULA                      ; Restore A
            PULX                      ; Restore X
            PULY                      ; Restore Y
            RTS                       ; Return

*********************************************************************** 


;  Message Here
main_msg0   DC.B        'Welcome to HW7 program, below will have some rules about this calculator', $00
main_msg1   DC.B        '1. Input positive decimal integer numbers only', $00
main_msg2   DC.B        '2. Input and output maximum four digit numbers only', $00
main_msg3   DC.B        '3. Valid operators are: +, -, *, and /',$00
main_msg4   DC.B        '4. Input number with leading 0 is OK', $00
main_msg5   DC.B        '5. Input only two numbers and one operator in between, no spaces', $00
prompt      DC.B        'Ecalu> ', $00
error_msg0  DC.B        'Invalid input format', $00
error_msg1  DC.B        'Overflow error', $00
            END               ; this is end of assembly source file
                                 ; lines below are ignored - not assembled/compiled 




