
***********************************************************************
*
* Title:          Signal Wave Generation and Digital Clock Program and ADC
*
* Objective:      CMPEN 472 HW10
*
* Date:	          11/10/2023 -- 11/14/2023
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
* Program:        Interrupt based milti-tasking programing, with multiple interrupt type handling.
*                 
*                 
* Algorithm:      Simple Serial I/O use, Interrupt and Timming: Real Time Interrupt(RTI)
*                 simple command line parsing and basic system I/O subroutine
*			            ADC register use
*
* Register use:	  Accumulator: A and B: used as ckecking and passing values to buffer, output message.
*                 X: address pointer. extend division
*                 Y: address pointer, extend multiplication
*                 D: arithmetic calculation, EMUL and IDIV.
*
* Memory use:     RAM Locations from $3000 for data, 
*                 RAM Locations from $3100 for program
*
* Output:         Program to generate sawtooth wave, triangle wave, and square wave while the digital 
*                 clock is running on the background.
*                 
*                 
* Observation:    The wave generation function is based on the Timer Interrupt OC3 at the rate of 125usec (8000Hz). 
*                 The Homework 10 program generates wave form on the terminal connected to the HC12 by sending the 
*                 signal value in 8-bit unsigned integer numbers, printed on the terminal screen. One number is printed 
*                 every 125usec and total 2048 numbers are printed for each wave generation command. At the same time, 
*                 the digital clock is running and displayed on the 7-segment display connected to the PORTB of the HCS12 board.
*
* Command:        (1)Command gw : generate sawtooth wave, printing 0 through 255, repeated for total 2048 points
*                 (2)Command gw2: generate sawtooth wave of 100Hz, wave repeated for total 2048 points
*                 (3)Command gt : generate triangle wave, printing 0 through 255, then 255 down to 0, repeated for total 2048 points
*                 (4)Command gq : generate square wave, printing 0 for 255 times, then print 255 for 255 times, then repeated for total 2048 points
*                 (5)Command gq2: generate square wave of 100Hz, wave repeated for total 2048 points           
*                 (6)Clock Command 't', 'q', 'h', 'm', 's', same as HW8
*                 (7)Command adc: generate the waveform absed on the command file feed into the command windows
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

; Real Time Interrupt Resister Set Up
CRGFLG      EQU         $0037         ; Clock and Reset Generator Flags
CRGINT      EQU         $0038         ; Clock and Reset Generator Interrupts
RTICTL      EQU         $003B         ; Real Time Interrupts Control

; Timer Register Set Up
TIOS        EQU         $0040         ; Timer Input Capture (IC) or Output Compare (OC) select
TIE         EQU         $004C         ; Timer interrupt enable register
TCNTH       EQU         $0044         ; Timer free running main counter
TSCR1       EQU         $0046         ; Timer system control 1
TSCR2       EQU         $004D         ; Timer system control 2
TFLG1       EQU         $004E         ; Timer interrupt flag 1
TC3H        EQU         $0056         ; Timer channel 3 high register

; ADC registers Set Up
ATDCTL2     EQU         $0082         ; Analog-to-Digital Converter (ADC) registers
ATDCTL3     EQU         $0083
ATDCTL4     EQU         $0084
ATDCTL5     EQU         $0085
ATDSTAT0    EQU         $0086
ATDDR0H     EQU         $0090
ATDDR0L     EQU         $0091
ATDDR7H     EQU         $009e
ATDDR7L     EQU         $009F
                                                    
CR          EQU         $0D           ; Carriage return,ASCII 'Return' key
LF          EQU         $0A           ; Line feed, ASCII 'next line' character
s           EQU         $73           ; 's' ASCII
q           EQU         $71           ; 'q' ASCII
t           EQU         $74           ; 't' ASCII
m           EQU         $6D           ; 'm' ASCII
h           EQU         $68           ; 'h' ASCII
g           EQU         $67           ; 'g' ASCII
w           EQU         $77           ; 'w' ASCII
space       EQU         $20           ; ' ' space in ASCII 
colon       EQU         $3A           ; ':' colon in ASCII
ASCIInumBias EQU        $30           ; Set the ASCII number '0' to '9' bias as $30
DATAmax     EQU         2048          ; Dtat Count maximum, 2048 constant
SquareMax   EQU         255           ; the maxinum value of square wave
***********************************************************************
*
* Data Section: address used [ $3000 to $30FF ] RAM memory
*
            ORG         $3000         ; Reserved RAM memory starting address 
            
Here        DS.B        $0C           ; Reserve 12 bytes
Terminate1  DC.B        $00           ; Terminate Byte  

ctr2p5m     DS.W        1             ; Interrupt counter for 2.5 mSecs of time
ctr125u     DS.W        1             ; 16 bit interrupt counter for 125uSec of time
BUF         DS.B        6             ; Character buffer for a 16 bit number in decimal ASCII
CTR         DS.B        1             ; Character buffer fill count
Counter0    DC.B        $03           ; Set Counter0 = 3
trig_counter DC.B       $00           ; the triangular counter
squaVal     DC.B        255           ; the square value
squaCounter2 DC.B       0             ; the square counter
sawtVal     DC.B        0             ; the 100Hz sawtVal
sawtCounter DC.B        0             ; the 100Hz sawtCounter

ACheck      DC.B        $00           ; Set ACheck flog as 0 for init state user is typing, 
wavetype    DC.B        $00           ; 0 for no wave, 1 for sawtooth, 2 for triangle, 3 for square, 4 for sawtooth 100Hz, 5 for square 100Hz
                                      ;  and 1 for error occur, and 2 for new time, 3 for valid time display
IsStartTime DC.B        $00           ; Set IsStartTime flag as 0 for user hasn't enter start time. and 1 for the start time entered 
h_flag      DC.B        $00           ; Hours flag, 1:h command is entered 0:no
m_flag      DC.B        $00           ; Minute flag, 1: m command is entered. 0:no
s_flag      DC.B        $00           ; Second flag, 1: s command is entered. 0:no
q_flag      DC.B        $00           ; Q_flag, 1: q command is entered. 0:no
trig_flag   DC.B        $01           ; triangle flag, 0 for decrease and 1 for increase

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
      
*
***********************************************************************
*
* Interrupt Vector Section: address Used from $FFF0 in simulation ($3FF0 for CSM-12C128)
*
            ORG         $FFF0         ; RTI interrupt vector setup for the simulator
            ;ORG         $3FF0         ; RTI interrupt vector setup for the CSM-12C128 board
            DC.W        rtiisr
            
            ORG         $FFE8         ; Timer channel 6 interrupt vector setup on simulator
            DC.W        oc3isr        
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

            LDX         #60
            STX         ctr2p5m       ; initialize interrupt counter with 40.
            CLI                       ; enable interrupt, global
            
            LDX          #main_msg0   ; Print the menu   
            JSR          printmsg     
            JSR          CRandLF      ; Carrage return and Line feed     ; Print the menu   
            


mainLoopinit
            LDY         #Here         ; Load Y with $3000
            LDAB        #$00          ; Load B with 0
            
            ;JSR         CRandLF       ; Carriage return and Line Feed
            LDX         #prompt       ; print the prompt message
            JSR         printmsg  
            
            
mainLoop    
            JSR         clockUpdate   ; Jump to clockUpdate
              
            JSR         getchar       ; Check the ketboard if there is command typed in
            CMPA        #$00          ; If nothing typed, keep checking
            BEQ         mainLoop                
            
            CMPB        #$0C          ; Check if user enter more than 12 charcters
            LBEQ        Error         ; Print Error, invalid input format
            
            JSR         putchar      ; What is typed on key board is displayed on the terminal window - echo print 
                   
            STAA        1, Y+        ; Store the value in Accumulator A to address in Y
            INCB                     ; Increase B
            
            CMPA        #CR          ; Detect if user do an 'enter'
            BNE         mainLoop     ; If Enter/Return key is pressed, load the saved
                                     ;  character and do command behaviors
                                     ;  if not, branch back to menuLoop           
                   
                                                 
            ; Check the first char to determine which command
            LDY         #Here         ; Load Y as the pointer that point to address 'Here'
            LDAA        1,Y+          ; Load A with the content in address that stored in Y
            
            CMPA        #CR           ; If user enter nothing, just go the next line with prompt
            BEQ         mainLoopinit  ; Branch back to mainLoopinit        
            CMPA        #$61           ; Check if the command is adc
            LBEQ         adc           ; Branch to adc
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
            CMPA        #g            ; Check if the command is g
            LBEQ        G             ; Branch to G
            LBRA        Error0        ; If none is matched, the comamnd is invalid
            
                     
            
setTime
            ; Check the second char
            LDAA        1,Y+          ; Load the second char into Accumulator A
            CMPA        #space        ; Check if the second char is a space
            LBNE        Error1        ; if not, branch to Error01, invaliad input format
            
            
            ; Check the hour part
            JSR         PutHours      ; Jump to PutHours, Y will goes to next two in this subroutine 
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error1        ; If Acheck = 1, error happen, jump to Error1
                                      ; else
            LDAA        tempBuffer1   ; Load A with the result
            STAA        hoursBuffer   ; Store it to the hourBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; Check the colon
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #colon        ; Check if is a colon
            LBNE        Error1        ; if not, branch to Error1, invaliad input format
            
            ; Check the minutes part
            JSR         PutMinsorSecs ; Jump to PutMinsandSecs
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error1        ; If Acheck = 1, error happen, jump to Error1
            
    
            LDAA        tempBuffer1   ; Load A with the result
            STAA        minsBuffer    ; Store it to the minsBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; check the colon
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #colon        ; Check if is a colon
            LBNE        Error1        ; if not, branch to Error1, invaliad input format
            
            ; check the seconds part
            JSR         PutMinsorSecs ; Jump to PutMinsandSecs
            LDAA        ACheck        ; Load A with ACheck
            CMPA        #$01          ; Check if error happen
            LBEQ        Error1        ; If Acheck = 1, error happen, jump to Error1
            
            LDAA        tempBuffer1   ; Load A with the result
            STAA        secsBuffer    ; Store it to the secsBuffer
            LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            
            ; check the return key
            LDAA        1,Y+          ; Load content in address in Y into Accumulator A
            CMPA        #CR           ; Check if is a return
            LBNE        Error1        ; branch to Error1
            
            LDAA        #$02          ; Load A with 2
            STAA        ACheck        ; Store ACheck = 2, new time is set
            LDAA        #$01          ; Load A with 1
            STAA        IsStartTime  
             ; Store IsStartTime = 1, means the user has put a valid time 
            
            LDX         #set_msg      ; Print the message that clock has been set
            JSR         printmsg      ; Jump to printmsg 
            JSR         CRandLF       ; Carriage Return and Line Feed 
            LBRA        mainLoopinit  ; Branch to mainLoopinit
            
Quit
            LDAA        Y             ; Load the second char
            CMPA        #CR           ; Check if the second char is the carriage return
            LBNE        Errorq        ; Branch to Errorq, invalid command
            
            LDX         #end_msg       ; print Quit message
            JSR         printmsg
            JSR         CRandLF
            LDX         #tw_msg0     
            JSR         printmsg
            JSR         CRandLF
     
            LBRA        TypeWriter    ; Branch to TypeWriter
            
hourDisp
            LDAA        1,Y+          ; Load the second char which should be an enter
            CMPA        #CR           ; Compare it with an enter
            LBNE         Errorh        ; if not, input is not valid
            
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
            LBNE         Errorm        ; if not, input is not valid
            
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
            LBNE         Errors        ; if not, input is not valid
            
            LDAA        #$01          ; Load A with 1
            STAA        s_flag        ; flag the s, means display the second in 7-segment led
            LDAA        #$00          ; Load A with 0
            STAA        m_flag        ; Flag down the minute
            STAA        h_flag        ; Flag down the second
            
            LDAA        #$03          ; Load A with 3
            STAA        ACheck        ; Set Acheck = 3, valid input 
            LBRA        mainLoopinit  ; Branch to mainLoopinit

adc         LDAA        1, Y+         ; Check the second char
            CMPA        #$64          ; Compare the second char with 'd'
            LBNE        Error0        ; if not, branch to Error0
            LDAA        1, Y+         ; Check the third char
            CMPA        #$63          ; Compare the third char with 'c'
            LBNE        Error0        ; if not, branch to Error0
            LDAA        1, Y+         ; Check the forth char
            CMPA        #CR           ; Check if the forth char is an enter
            LBNE        Error0        ; if not, branch to Error0
            
            ; adc command is typed
            LDAA        #$06          ; LOAD A with 6
            STAA        wavetype      ; Store it to wavetype
            
            LDX         #adc_msg      ; Print the adc_msg user guide
            JSR         printmsg      
            LBRA        GLoop         ; Branch to GLoop
            
            
             
            
G
            LDAA        1, Y+         ; Check the second char
            CMPA        #w            ; Compare the second char with w
            BEQ         GW            ; if yes, branch to GW
            CMPA        #t            ; Compare the second char with t
            BEQ         GT            ; If yes, branch to GT
            CMPA        #q            ; Compare the second char with q
            BEQ         GQ            ; IF yes, branch to GQ
            LBRA        Error0        ; if nothing meet, wrong input is entered
            
GW          
            LDAA        1,Y+          ; Check the third char
            CMPA        #$32          ; if the third char is number 2
            BEQ         GW2           ; Branch to GW2
            
            CMPA        #CR           ; Chekc if the thrid char is a return
            LBNE         Error0        ; If not Wrong input
            
            LDAA        #$01          ; Load A with 1
            STAA        wavetype      ; Set the wave type
            
            LDX         #g_msg        ; print the g_msg
            JSR         printmsg
            LDX         #gw_msg       ; print the gw_msg
            JSR         printmsg      ; 
            BRA         GLoop         ; Branch to GLoop
                        
GT          
            LDAA        1,Y+          ; Check the third char
            CMPA        #CR           ; Check if the thrid is a return
            LBNE         Error0        ; If not, wrong input then
            
            LDAA        #$02          ; Load A with 2
            STAA        wavetype      ; Set the wave type
            
            LDX         #g_msg        ; print the g_msg
            JSR         printmsg
            LDX         #gt_msg       ; print the gt_msg
            JSR         printmsg      ; 
            BRA         GLoop         ; Branch to GLoop
            
GQ                        
            LDAA        1,Y+          ; Check the third char
            CMPA        #$32          ; if the third char is number 2
            BEQ         GQ2           ; Branch to GW2
            
            CMPA        #CR           ; Chekc if the thrid char is a return
            LBNE         Error0        ; If not Wrong input
            
            LDAA        #$03          ; Load A with 3
            STAA        wavetype      ; Set the wave type
            
            LDX         #g_msg        ; print the g_msg
            JSR         printmsg
            LDX         #gq_msg       ; print the gq_msg
            JSR         printmsg      ; 
            BRA         GLoop         ; Branch to GLoop
            
GW2         
            LDAA        1,Y+          ; Check the forth char
            CMPA        #CR           ; Check if the forth is a return
            LBNE         Error0        ; If not, wrong input then
            
            LDAA        #$04          ; Load A with 4
            STAA        wavetype      ; Set the wave type
            
            LDX         #g_msg        ; print the g_msg
            JSR         printmsg
            LDX         #gw2_msg       ; print the gw2_msg
            JSR         printmsg      ; 
            BRA         GLoop         ; Branch to GLoop

GQ2         
            LDAA        1,Y+          ; Check the forth char
            CMPA        #CR           ; Check if the forth is a return
            LBNE        Error0        ; If not, wrong input then
            
            LDAA        #$05          ; Load A with 5
            STAA        wavetype      ; Set the wave type
            
            LDX         #g_msg        ; print the g_msg
            JSR         printmsg
            LDX         #gq2_msg       ; print the gq2_msg
            JSR         printmsg      ; 
            BRA         GLoop         ; Branch to GLoop
            
GLoop       
            JSR         clockUpdate
            JSR         getchar       ; The user need to press the enter to start
            CMPA        #0
            BEQ         GLoop
            CMPA        #CR           ; check if return entered
            BNE         GLoop         ; if not branch back to GWLoop
            JSR         putchar       ; print an enter
            JSR         delay1ms      ; flush out SCI serial port 
                                      ; wait to finish sending last characters
            LDX         #0            ; Return key entered
            STX         ctr125u       ; Reset the ctr125u counter
            JSR         StartTimer3oc ; Init the Timer
            
            CLI                       ; Interrupt enable, for timer OC3 interrupt start
            
            LDAA        wavetype      ; Load A with wavetype
            CMPA        #$06          ; If A!= 6
            BNE         Loop2048      ; Branch to Loop2048
                                      ; else
            ; ADC initialization
            LDAA        #%11000000    ; Turn on ADC, clear flags, disable ADC interrupt
            STAA        ATDCTL2
            LDAA        #%00001000    ; Single conversion per sequence, no FIFO
            STAA        ATDCTL3       
            LDAA        #%10000111    ; 8 bit, ADCLK = 24MHz/16 = 1.5MHz, sampling time = 2*(1/ADCLK)
            STAA        ATDCTL4 
            ; First Conversion
            LDAA        #%10000111    ; right justified, unsigned, single conversion,
            STAA        ATDCTL5       ; single channel, CHANNEL 7, start the conversion 
; after the interrupt is set, loop until all 2049 points are collected for adc
Loop2049    
            JSR         clockUpdate
            LDD         ctr125u   
            CPD         #2049         ; check if 2049 byte has been send
            BHS         LoopTxON      ; Branch to LoopTxON
            BRA         Loop2049      ; Branch to Loop2048                    

; after the interrupt is set, loop until all 2048 points are collected
Loop2048    
            JSR         clockUpdate
            LDD         ctr125u   
            CPD         #DATAmax      ; check if 2048 byte has been send
            BHS         LoopTxON      ; Branch to LoopTxON
            BRA         Loop2048      ; Branch to Loop2048
            

            
LoopTxON    
            LDAA        #%00000000
            STAA        TIE           ; disable OC3 interrupt
            
            JSR         CRandLF       ; Go to next line
            
            LDX         #done_msg      ; Print the finished message
            JSR         printmsg      
            JSR         CRandLF
            
            LDAA        #$00          ; LOAd A with 0
            STAA        trig_counter  ; Reset trig_counter
            STAA        squaCounter2  ; Reset squaCounter
            STAA        sawtCounter   ; Reset sawtCounter
            STAA        sawtVal       ; Reset sawtVal
            LDAA        #$01          ; LOad A with 1
            STAA        trig_flag     ; Reset the flag
            LDAA        #255          ; LOAD A with 255
            STAA        squaVal       ; Reset squaVal
            LDAA        #%00000000    ; Turn off ADC, clear flags, disable ADC interrupt
            STAA        ATDCTL2
            
            LBRA        mainLoopinit  ; Branch back to mainLoopinit 
                        
            
Error       JSR         CRandLF
Error0      LDX         #error_msg0   ; Load X as pointer to address of error_msg0
            BRA         PrintError    ; Branch to Print Error
                        
Error1      LDAA        #$00          ; Load A with 0
            STAA        tempBuffer1   ; Clean tempBuffer1
            LDAA        #$01          ; Load A with 1
            STAA        ACheck        ; Set ACheck = 1
            LDAA        temphours     ; Restore hours
            STAA        hoursBuffer
            LDAA        tempminutes   ; Restore minutes
            STAA        minsBuffer
            LDAA        secsBuffer    ; Restore seconds
            STAA        tempseconds
            
            LDX         #error_msg1   ; Load X as a pointer to address of error_msg1
            BRA         PrintError    ; Branch to Print Error
            
Errorq      LDX         #error_msgq   ; Load X as a pointer to address of error_msgq
            BRA         PrintError    ; Branch to Print Error
           
Errorh      LDX         #error_msgh   ; Load X as a pointer to address of error_msgh
            BRA         PrintError    ; Branch to Print Error
            
Errorm      LDX         #error_msgm   ; Load X as a pointer to address of error_msgm
            BRA         PrintError    ; Branch to Print Error

Errors      LDX         #error_msgs   ; Load X as a pointer to address of error_msgs
            
PrintError  JSR         printmsg      ; Print the message
            JSR         CRandLF       ; Carriage Return and Line Feed
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
; rtiisr subroutine function
; 
; Function: RTI interrupt service routine       
;**********************************************           

rtiisr      BSET         CRGFLG,%10000000 ; clear RTI Interrupt Flag - for the next one
            LDX          ctr2p5m          ; every time the RTI occur, increase
            INX                     ;    the 16bit interrupt count
            STX          ctr2p5m
rtidone     RTI
            
***********************************************************************   


***********************************************************************            
;**********************************************
; oc3isr interrupt service routine
; 
; Function: timer interrupt service routine       
;**********************************************           

oc3isr      LDD          #3000        ; 125usec with (24MHz/1 clock)
            ADDD         TC3H         ;  for next interrupt
            STD          TC3H         ; 
            BSET         TFLG1,%00001000  ; Clear timer CH3 intrerupt flag, not needed if fast clear enabled
            
            LDAA         wavetype     ; Load A with the value in wavetype
            CMPA         #$01         ; Check the waveform type 
            BEQ          sawtooth     ; Branch to sawtooth
            CMPA         #$02         
            BEQ          triangle     ; Branch to triangle
            CMPA         #$03
            LBEQ          square       ; Branch to square
            CMPA         #$04
            LBEQ          sawtooth2    ; Branch to sawtooth 100Hz
            CMPA         #$05
            LBEQ          square2      ; Branch to square 100Hz
            CMPA         #06
            BEQ          adc_data     ; Branch to adc_data
            
adc_data    
            LDAB         ATDDR0L          ; for SIMULATOR, pick up the lower 8bit result
            
            LDAA         #%10000111       ; right justified, unsigned, single conversion,
            STAA         ATDCTL5          ; single channel, CHANNEL 7, start the conversion 
            
            LDX          ctr125u          ; Load the ctr125u
            CPX          #$00             ; Compare with 0
            BEQ          SkipFirst        ; Yes, Branch to SkipFirst 
            
            CLRA
            JSR          pnum10           ; print the ATD result
 
SkipFirst   INX                           ; Update OC3 (125usec) interrupt counter
            STX          ctr125u          ; Branch to oc3done 
               
            LBRA         oc3done

            
sawtooth
            LDD          ctr125u      
            LDX          ctr125u  
            INX                       ; Update OC6 (125usec) interrupt counter
            STX          ctr125u      
            CLRA                      ; Ptint ctr125u, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            LBRA          oc3done      ; Branch to oc3done


            
triangle
            LDX          ctr125u  
            INX                       ; Update OC6 (125usec) interrupt counter
            STX          ctr125u     
            LDAB         trig_counter ; Load B with the trig_counter
            LDAA         trig_flag    ; Load A with the trig_flag
            CMPA         #$01         ; Check the flag
            BEQ          Increase     ; Branch to increase
            BRA          Decrease     ; if flag not = 1,branch to Decrease 
            
Increase    CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            CMPB         #255         ; Check if B reach 255
            BEQ          setflag1     ; Branch to setflag1
            INCB                      ; If not reach to 255
            STAB         trig_counter ; update the trig_counter
            LBRA          oc3done      ; Branch to oc3done
            
setflag1    LDAA         #$00         ; load A with 0
            STAA         trig_flag    ; Set the trig_flag = 0
            LBRA         oc3done      ; Branch to oc3done
                              
Decrease    CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            CMPB         #0           ; Check if B reach to 0
            BEQ          setflag2     ; Branch to setflag2
            DECB                      ; if not reach to 0
            STAB         trig_counter ; update the trig_counter
            LBRA         oc3done      ; Branch to oc3done
            
setflag2    LDAA         #$01         ; Load A with 1
            STAA         trig_flag    ; Update the trig_flag = 1
            LBRA         oc3done




square      LDAB         squaVal      ; Load A with square value
            CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            
            LDD          ctr125u      ; Load X with ctr125u
            CMPB         #255         ; Compare B with 255
            BNE          IncCounter   ; Yes, Branch to IncCounter
            
            
            LDAA         #SquareMax   ; LOAD A with 255
            SUBA         squaVal      ; Update the value
            STAA         squaVal      ; Store the updated value to squaCounter
            
IncCounter  LDX          ctr125u      ; Load X with ctr125u
            INX                       ; Else, increase X
            STX          ctr125u      ; update ctr125u 
            BRA          oc3done            



sawtooth2   LDAA         sawtCounter  ; Load A with sawtCounter
            LDAB         sawtVal      ; Load B with sawtVal
            CMPA         #4           ; Check if sawCounter = 3
            BEQ          Add4         ; Branch to add4
            
            CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            LDAA         sawtCounter  ; Load A with sawtCounter
            INCA                      ; Increase A
            STAA         sawtCounter  ; Update sawtCounter
                 
            BRA          updateCounter ; branch to update the counter      
                     
Add4        INCB                      ; Increase B so that we add 4 from the previous
            CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            LDAA         #$01         ; Load A with 1
            STAA         sawtCounter  ; Reset the sawtCounter
            
updateCounter
            LDX          ctr125u      ; Update ctr125u 
            INX
            STX          ctr125u 
            CMPB         #253         ; Compare B with 253
            BEQ          Add1         ; Branch to add 1
            CMPB         #255         ; Chekc B is 255
            BEQ          Reset  
            ADDB         #3           ; Add 3 to B
            STAB         sawtVal      ; Update sawtVal
            BRA          oc3done  
            
Add1        ADDB         #2           ; Only increase B by 1
            STAB         sawtVal      ; Update the sawtVal
            BRA          oc3done      
          
Reset       LDAB         #$00         ; LOAD B with 0
            STAB         sawtVal      ; Reset the sawtVal
            STAB         sawtCounter
            BRA          oc3done         



square2     LDAB         squaVal      ; Load A with square value
            CLRA                      ; Ptint trig_counter, only the last byte, so that the number will loop from 0 to 255
            JSR          pnum10       ; To make the file RXData3.txt with exactly 2048 data
            
            LDAB         squaCounter2 ; Load the current number of samples of one period
            CMPB         #39          ; Check if B = 39
            BNE          updateCounter0 ; Branch to updateCounter0
            
            LDAA         #$00         ; Load A with 0
            STAA         squaCounter2 ; Reset the counter to 0
            LDAA         #SquareMax   ; LOAD A with 255
            SUBA         squaVal      ; Update the value
            STAA         squaVal      ; Store the updated value to squaCounter
            BRA          updateCounter1 ; Branch to updateCounter1
            
updateCounter0
            INCB                      ; Increase B
            STAB         squaCounter2 ; Update the squaCounter2
                     
updateCounter1
            LDX          ctr125u      ; Load X with ctr125
            INX                       ; Else, increase X
            STX          ctr125u      ; update ctr125u 
            
            BRA          oc3done  

          
            
oc3done     RTI
            
*********************************************************************** 


***********************************************************************            
;**********************************************
;StartTimer3oc subroutine 
; 
; Program: Start the timer interrupt, timer channel 3 output compare
; Input:   Constants - channel 3 output compare, 125usec at 24MHz
; Output:  None, only the timer interrupt
; Registers modified: D used and CCR modified
; Algorithm:
;           initialize TIOS, TIE, TSCR1, TSCR2, TC3H, and TFLG1
;**********************************************           

StartTimer3oc
            PSHD
            LDAA   #%00001000
            STAA   TIOS              ; set CH3 Output Compare
            STAA   TIE               ; set CH3 interrupt Enable
            LDAA   #%10000000        ; enable timer, Fast Flag Clear not set
            STAA   TSCR1
            LDAA   #%00000000        ; TOI Off, TCRE Off, TCLK = BCLK/1
            STAA   TSCR2             ;   not needed if started from reset

            LDD    #3000            ; 125usec with (24MHz/1 clock)
            ADDD   TCNTH            ;    for first interrupt
            STD    TC3H             ; 

            BSET   TFLG1,%00001000   ; initial Timer CH3 interrupt flag Clear, not needed if fast clear set
            LDAA   #%00001000
            STAA   TIE               ; set CH3 interrupt Enable
            PULD
            RTS
            
*********************************************************************** 


***********************************************************************            
;**********************************************
; Program: print a word (16bit) in decimal to SCI port
; Input:   Register D contains a 16 bit number to print in decimal number
; Output:  decimal number printed on the terminal connected to SCI port
; 
; Registers modified: CCR
; Algorithm:
;     Keep divide number by 10 and keep the remainders
;     Then send it out to SCI port
;  Need memory location for counter CTR and buffer BUF(6 byte max)
;**********************************************           

pnum10          PSHD                   ;Save registers
                PSHX
                PSHY
                CLR     CTR            ; clear character count of an 8 bit number

                LDY     #BUF
pnum10p1        LDX     #10
                IDIV                   ; Interger divide
                BEQ     pnum10p2       ; If interger divide finish, Quotient = 0, branch to pnum10p2
                STAB    1,y+           ; else store the remainer to BUF
                INC     CTR            ; increase count that record how many digit has been filled
                TFR     x,d            ; Put the quotient in X to D
                BRA     pnum10p1       ; Keep Looping

pnum10p2        STAB    1,y+
                INC     CTR                        
;--------------------------------------

pnum10p3        LDAA    #$30           ; Print the number     
                ADDA    1,-y
                JSR     putchar
                DEC     CTR
                BNE     pnum10p3
                JSR     CRandLF
                
                PULY
                PULX
                PULD
                RTS
            
*********************************************************************** 


***********************************************************************  
;**********************************************
; delay1ms subroutine function
;
; Function: delat 1 ms

;**********************************************

delay1ms:       PSHX
                LDX   #$1000           ; count down X, $8FFF may be more than 10ms 
d1msloop        NOP                    ;   X <= X - 1
                DEX                    ; simple loop
                BNE   d1msloop
                PULX
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
; clockUpdate subroutine function
; 
; Function: Each 1s, it will going to update the time
;       
; Input:    hourBuffer, minuteBuffet, secondBuffer
; Output:   update time after 1s in bourBuffer, minuteBuffer, secondBuffer
;          

;**********************************************

clockUpdate            
            PSHD                          ; Save D
            PSHX                          ; Save X
            PSHY                          ; Save Y
            
            LDX          ctr2p5m          ; Check for 1.0 s
            CPX          #60              ; 2.5msec * 60 = 0.15 sec. If want 1s, then needs 400, but simulation is slow
            LBLO         IODone           ; Not Yet one second, Branch to done
            
            LDX          #0               ; 1 sec is up,
            STX          ctr2p5m          ; clear counter to restart
            
            LDAB         ACheck           ; Let B keep the value of ACheck
            
            ; Figure out what need to print for time
            LDAA         IsStartTime      ; Load A with the IsStartTime flag
            CMPA         #$00             ; Compare with 0
            BEQ          HMS              ; If yes, branch to PrintTim
            CMPB         #$02             ; Compare B with 2
            BEQ          HMS              ; If ACheck = 2, new time is set, branch to HMS
            JSR          ChangeTime       ; Else, branch to ChangeTime, to increment time by 1s
                       
HMS         ; Figure out the h, m, s command
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
            BRA          IOReset          ; Else, all three flag are 0, branch to CMD part    
            
GetHour     LDAA         hoursBuffer      ; Load A with hoursBuffer
            BRA          DispTime         ; Branch to Display Time

GetMinute   LDAA         minsBuffer       ; Load A with minsBuffer
            BRA          DispTime         ; Branch to Display Time

GetSecond   LDAA         secsBuffer       ; Load A with secsBuffe
            
DispTime    STAA         tempBuffer1      ; Store it into tempBuffer1
            JSR          GetDispNumber    ; Jump to GetDispNumber
            LDAA         dispBuffer       ; Load the result from dispBuffer
            STAA         PORTB            ; Send it to PORTB, display it on 7-segment led
           
IOReset     LDAA         #$00             ; Load A with 0
            STAA         ACheck           ; Reset ACheck 
                         
            LDAA        hoursBuffer   ; Before update the hourBuffer
            STAA        temphours     ; Store it first
            LDAA        minsBuffer    ; Before update the minsBuffer
            STAA        tempminutes   ; Store it first
            LDAA        minsBuffer    ; Before update the secsBuffer
            STAA        tempminutes   ; Store it first
            
IODone      PULY                          ; Restore Y
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
prompt      DC.B        'HW11> ', $00
main_msg0   DC.B        'Welcome to HW10 program, this program will provede digital clock features and can plot different waveforms', CR
main_msg1   DC.B        "1. For Digital Clock Program, We have 5 commands can use, they are:'t' for 'set time' command, 'q' for 'quit' command,", CR
main_msg2   DC.B        "   'h' for 'hour display' command, 'm' for 'minute display' command, and s for 'second display' command", CR
main_msg3   DC.B        '2. h, m ,s command will have time display on the 7-segment LED',CR
main_msg4   DC.B        "3. All commands have to be lower case for letters. and the time format has to be 'hh:mm:ss'", CR
main_msg5   DC.B        '4. There has to be a space after the letter you typed on command', CR
main_msg6   DC.B        '5. We provide 5 commands for different waveform generation, each generation will contain 2048 total samples.', CR
main_msg7   DC.B        '6. Command gw : generate sawtooth wave, printing 0 through 255', CR
main_msg8   DC.B        '7. Conmand gw2:  generate sawtooth wave of 100Hz', CR
main_msg9   DC.B        '8. Command gt : generate triangle wave, printing 0 through 255, then 255 down to 0', CR 
main_msg10  DC.B        '9. Command gq : generate square wave, printing 0 for 255 times, then print 255 for 255 times',CR
main_msg11  DC.B        '10.Command gq2 : generate square wave of 100Hz', CR
main_msg12  DC.B        '11.Command adc: run ADC function', $00
set_msg     DC.B        '> Clock been set successfully', $00
error_msg0  DC.B        '> Invalid input format.', $00
error_msg1  DC.B        'Error> Invalid time format. Correct example => 00:00:00 to 23:59:59', $00
error_msgs  DC.B        "Error> Invalid command. ('s' for 'second display' and 'q' for 'quit')", $00
error_msgm  DC.B        "Error> Invalid command. ('m' for 'minute display' and 'q' for 'quit')", $00
error_msgh  DC.B        "Error> Invalid command. ('h' for 'hour display' and 'q' for 'quit')", $00
error_msgq  DC.B        "Error> Invalid command. ('q' for 'quit')", $00
g_msg       DC.B        'Open RxData3.txt and hit enter key to start generating ', $00
gw_msg      DC.B        'sawtooth wave ...', $00
gw2_msg     DC.B        'sawtooth wave 100Hz', $00
gt_msg      DC.B        'triangle wave ...', $00
gq_msg      DC.B        'square wave ...', $00
gq2_msg     DC.B        'square wave 100Hz ...', $00
adc_msg     DC.B        'Open text file to store data, and hit enter key to start', $00
done_msg    DC.B        'Done. Waveform has been generated. Make sure to close the RxData3.txt', $00
end_msg     DC.B        '       Analog to Digital Conversion, Wave Generator and Clock stopped and Typewrite program started.', $00
tw_msg0     DC.B        '       You may type below.', $00 

            END               ; this is end of assembly source file
                                 ; lines below are ignored - not assembled/compiled             