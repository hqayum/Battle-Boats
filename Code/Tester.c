/* 
 * File:   Tester.c
 * Author: Hasson Qayum
 * Collaborator: Kameron Gill
 */
// To test thoroughly, uncomment printf statements in Protocol.c

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//CMPE13 Support Library
#include "BOARD.h"
#include "CircularBuffer.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Buttons.h"
#include "Oled.h"
#include "Protocol.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****
static uint32_t counter;
static uint8_t buttonEvents;

// **** Declare any function prototypes here ****



int main()
{
    BOARD_Init();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a 10ms timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

    // Disable buffering on stdout
    setbuf(stdout, NULL);

    ButtonsInit();

//    OledInit();
//
//    // Prompt the user to start the game and block until the first character press.
//    OledDrawString("Press BTN4 to start.");
//    OledUpdate();
//    while ((buttonEvents & BUTTON_EVENT_4UP) == 0);


    /******************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     *****************************************************************************/
    
    char *message;
    message = malloc(PROTOCOL_MAX_MESSAGE_LEN);
    
    // Initiate Guess Data
    GuessData GDTest;
    GDTest.row = 553487;
    GDTest.col = 30023;
    GDTest.hit = 1111111;
    

//    GDTest.row = 57;
//    GDTest.col = 1;
//    GDTest.hit = 1111;
    
    // Test Encode Guess messages
    ProtocolEncodeCooMessage(message, &GDTest);
    printf("%s", message);
    ProtocolEncodeHitMessage(message, &GDTest);
    printf("%s", message);
    
    
    // Initiate Negotiation Data
    NegotiationData NDTest;
    ProtocolGenerateNegotiationData(&NDTest);
    
    // Check values of generated Negotiation Data
    printf("%d\t", NDTest.encryptedGuess);
    printf("%d\t", NDTest.encryptionKey);
    printf("%d\t", NDTest.guess);
    printf("%d\n", NDTest.hash);
    
    // Test Encode Negotiation Messages
    ProtocolEncodeChaMessage(message, &NDTest);
    printf("%s", message);
    ProtocolEncodeDetMessage(message, &NDTest);
    printf("%s", message);
    
    
    // Manual test, hard-coded
//    ProtocolEncodeCooMessage(message, &GDTest);
//    ProtocolEncodeHitMessage(message, &GDTest);
//    ProtocolEncodeChaMessage(message, &NDTest);
    ProtocolEncodeDetMessage(message, &NDTest);
    //message = "$DET,222,43*F7\n";
    printf("\n%s\n\n", message);
    
    
    // Test Decode and make sure FSM is working properly
    NegotiationData nData;
    GuessData gData;
    int iteration = 0;
    ProtocolParserStatus decodeReturn;
    while(iteration < strlen(message)) {
        decodeReturn = ProtocolDecode(message[iteration], &nData, &gData);
        if (decodeReturn == PROTOCOL_PARSING_FAILURE) {
            printf("ERROR!!!!\n");
        } else if (decodeReturn == PROTOCOL_PARSED_DET_MESSAGE) {
            printf("returned DET\n");
        } else if (decodeReturn == PROTOCOL_PARSED_HIT_MESSAGE) {
            printf("returned HIT\n");
        } else if (decodeReturn == PROTOCOL_PARSED_COO_MESSAGE) {
            printf("return COO\n");
        } else if (decodeReturn == PROTOCOL_PARSED_CHA_MESSAGE) {
            printf("return CHA\n");
        }
        iteration++;
    }
    
   // Test PVND function
    if (ProtocolValidateNegotiationData(&NDTest) == SUCCESS) {
        printf("PVND successful\n");
    } else {
        printf("PVND failed\n");
    }
    
    // Test TurnOrder function
    NegotiationData TurnOrderTest;
    ProtocolGenerateNegotiationData(&TurnOrderTest);
    if(ProtocolGetTurnOrder(&NDTest, &TurnOrderTest) == TURN_ORDER_START){
        printf("I Start\n");
    } else if(ProtocolGetTurnOrder(&NDTest, &TurnOrderTest) == TURN_ORDER_DEFER){
        printf("They Start\n");
    } else { // if(ProtocolGetTurnOrder(&NDTest, &TurnOrderTest) == TURN_ORDER_TIE)
        printf("TIE\n");
    }

/******************************************************************************
 * Your code goes in between this comment and the preceeding one with asterisks
 *****************************************************************************/

    while (1);
}



/**
 * This is the interrupt for the Timer2 peripheral. It just keeps incrementing a counter used to
 * track the time until the first user input.
 */
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    // Increment a counter to see the srand() function.
    counter++;

    // Also check for any button events
    buttonEvents = ButtonsCheckEvents();
}
