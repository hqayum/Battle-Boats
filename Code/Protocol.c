/* 
 * File:   Protocol.c
 * Author: Hasson Qayum
 * Collaborator: Kameron Gill
 */

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//CMPE13 Support Library
#include "BOARD.h"
#include "Protocol.h"

// Microchip libraries
// #include <xc.h>
// #include <plib.h>

// User libraries

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// States for our FSM, used in ProtocolDecode()

typedef enum {
    WAITING,
    RECORDING,
    FIRST_CHECKSUM_HALF,
    SECOND_CHECKSUM_HALF,
    NEWLINE
} ParserState;

// struct based off rubric for lab

typedef struct {
    char sentence[PROTOCOL_MAX_MESSAGE_LEN]; // character array for our struct
    int index; // index of character array
    ParserState state; // current state of FSM, ParserState
    uint8_t checksum; // holds checksum value
} ProtocolDecodeVariables;

// **** Define any module-level, global, or external variables here ****

// Initialize our struct
static ProtocolDecodeVariables proDecode = {
    {}, 0, WAITING, 0
};
//proDecode = {{}, 0, WAITING, 0};
//proDecode.index = 0;
//proDecode.state = WAITING;

// variables used in FSM
static uint8_t compareChecksums; // compares to struct checksum for checksum matching
static char *stringParser; // parse for M_ID
static char *parse1; // DATA1
static char *parse2; // DATA2
static char *parse3; // DATA3 - only for "HIT"

// **** Declare any function prototypes here ****
static void StringClearMessage(char in[]); // simply clears a string that gets passed in
static uint8_t FunctionChecksum(char in[]); // calculates 1-byte checksum of string
static int ValidInput(char in); // checks if ASCII encoded hex-char is Valid or not
static uint8_t AsciiToHex(char in); // converts ASCII encoded hex-char to numerical representation

/**
 * Encodes the coordinate data for a guess into the string `message`. This string must be big
 * enough to contain all of the necessary data. The format is specified in PAYLOAD_TEMPLATE_COO,
 * which is then wrapped within the message as defined by MESSAGE_TEMPLATE. The final length of this
 * message is then returned. There is no failure mode for this function as there is no checking
 * for NULL pointers.
 * @param message The character array used for storing the output. Must be long enough to store the
 *                entire string, see PROTOCOL_MAX_MESSAGE_LEN.
 * @param data The data struct that holds the data to be encoded into `message`.
 * @return The length of the string stored into `message`.
 */
int ProtocolEncodeCooMessage(char *message, const GuessData *data)
{
    // Got guidance from tutoring on Encoding, and it's pretty straightforward
    StringClearMessage(message);
    char cooMessage[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(cooMessage, PAYLOAD_TEMPLATE_COO, data->row, data->col);
    uint8_t reChecksum = FunctionChecksum(cooMessage);
    sprintf(message, MESSAGE_TEMPLATE, cooMessage, reChecksum);
    return strlen(message);
}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeHitMessage(char *message, const GuessData *data)
{
    // Got guidance from tutoring on Encoding, and it's pretty straightforward
    StringClearMessage(message);
    char hitMessage[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(hitMessage, PAYLOAD_TEMPLATE_HIT, data->row, data->col, data->hit);
    uint8_t reChecksum = FunctionChecksum(hitMessage);
    sprintf(message, MESSAGE_TEMPLATE, hitMessage, reChecksum);
    return strlen(message);
}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeChaMessage(char *message, const NegotiationData *data)
{
    // Got guidance from tutoring on Encoding, and it's pretty straightforward
    StringClearMessage(message);
    char chaMessage[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(chaMessage, PAYLOAD_TEMPLATE_CHA, data->encryptedGuess, data->hash);
    uint8_t reChecksum = FunctionChecksum(chaMessage);
    sprintf(message, MESSAGE_TEMPLATE, chaMessage, reChecksum);
    return strlen(message);
}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeDetMessage(char *message, const NegotiationData *data)
{
    // Got guidance from tutoring on Encoding, and it's pretty straightforward
    StringClearMessage(message);
    char detMessage[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(detMessage, PAYLOAD_TEMPLATE_DET, data->guess, data->encryptionKey);
    uint8_t reChecksum = FunctionChecksum(detMessage);
    sprintf(message, MESSAGE_TEMPLATE, detMessage, reChecksum);
    return strlen(message);
}

/**
 * This function decodes a message into either the NegotiationData or GuessData structs depending
 * on what the type of message is. This function receives the message one byte at a time, where the
 * messages are in the format defined by MESSAGE_TEMPLATE, with payloads of the format defined by
 * the PAYLOAD_TEMPLATE_* macros. It returns the type of message that was decoded and also places
 * the decoded data into either the `nData` or `gData` structs depending on what the message held.
 * The onus is on the calling function to make sure the appropriate structs are available (blame the
 * lack of function overloading in C for this ugliness).
 *
 * PROTOCOL_PARSING_FAILURE is returned if there was an error of any kind (though this excludes
 * checking for NULL pointers), while
 * 
 * @param in The next character in the NMEA0183 message to be decoded.
 * @param nData A struct used for storing data if a message is decoded that stores NegotiationData.
 * @param gData A struct used for storing data if a message is decoded that stores GuessData.
 * @return A value from the UnpackageDataEnum enum.
 */
ProtocolParserStatus ProtocolDecode(char in, NegotiationData *nData, GuessData *gData)
{
    switch (proDecode.state) {
    case WAITING:
        // arrow to itself
        if (in != '$') {
            //printf("in != $\n");
            return PROTOCOL_WAITING;
        } else if (in == '$') {
            // arrow to RECORDING
            proDecode.index = 0;
            proDecode.state = RECORDING;
            //printf("in == $, go to RECORDING\n");
            return PROTOCOL_PARSING_GOOD;
        }
        break;
    case RECORDING:
        // arrow to itself
        if (in != '*') {
            proDecode.sentence[proDecode.index] = in;
            proDecode.index += 1;
            //printf("index++\n");
            return PROTOCOL_PARSING_GOOD;
        } else if (in == '*') {
            // arrow to FIRST_CHECKSUM_HALF
            //printf("in == * so go to FIRST_CHECKSUM_HALF\n");
            proDecode.state = FIRST_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        }
        break;
    case FIRST_CHECKSUM_HALF:
        // arrow to SECOND_CHECKSUM_HALF
        if (ValidInput(in) == SUCCESS) {
            //printf("Valid input, go to SECOND_CHECKSUM_HALF\n");
            proDecode.checksum = AsciiToHex(in) << 4; // shift 4 bits left for first half
            proDecode.state = SECOND_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        } else if (ValidInput(in) == STANDARD_ERROR) {
            // arrow to WAITING
            //printf("invalid input, go to WAITING\n");
            proDecode.state = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
    case SECOND_CHECKSUM_HALF:
        proDecode.checksum |= AsciiToHex(in); // get the second half checksum
        compareChecksums = FunctionChecksum(proDecode.sentence); // get message checksum
        // arrow to NEWLINE
        if (ValidInput(in) == SUCCESS && compareChecksums == proDecode.checksum) {
            proDecode.sentence[proDecode.index] = '\0';
            proDecode.state = NEWLINE;
            //printf("Checksums equal, go to NEWLINE\n");
            return PROTOCOL_PARSING_GOOD;
        } else if (ValidInput(in) == STANDARD_ERROR || compareChecksums != proDecode.checksum) {
            // arrow to WAITING
            //printf("Invalid input or Checksums unequal\n");
            proDecode.state = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
    case NEWLINE:
        // visual to help me parse:
        // For hit: "$M_ID,DATA1,DATA2,DATA3*XX\n"
        // For the rest: "$M_ID,DATA1,DATA2*XX\n"
        // arrow to WAITING on right
        if (in == '\n') {
            proDecode.state = WAITING;
            stringParser = strtok(proDecode.sentence, ",");
            //printf("StringParser: %s\n", stringParser);
            if (strcmp(stringParser, "HIT") == 0) {
                parse1 = strtok(NULL, ",");
                parse2 = strtok(NULL, ",");
                parse3 = strtok(NULL, "*");
                //printf("%s\t%s\t%s\n", parse1, parse2, parse3);
                gData->row = atoi(parse1);
                gData->col = atoi(parse2);
                gData->hit = atoi(parse3);
                //printf("%d\t%d\t%d\n", gData->row, gData->col, gData->hit);
                return PROTOCOL_PARSED_HIT_MESSAGE;
            } else if (strcmp(stringParser, "COO") == 0) {
                parse1 = strtok(NULL, ",");
                parse2 = strtok(NULL, "*");
                //printf("%s\t%s\n", parse1, parse2);
                gData->row = atoi(parse1);
                gData->col = atoi(parse2);
                //printf("%d\t%d\n", gData->row, gData->col);
                return PROTOCOL_PARSED_COO_MESSAGE;
            } else if (strcmp(stringParser, "CHA") == 0) {
                parse1 = strtok(NULL, ",");
                parse2 = strtok(NULL, "*");
                //printf("%s\t%s\n", parse1, parse2);
                nData->encryptedGuess = atoi(parse1);
                nData->hash = atoi(parse2);
                //printf("%d\t%d\n", nData->encryptedGuess, nData->hash);
                return PROTOCOL_PARSED_CHA_MESSAGE;
            } else if (strcmp(stringParser, "DET") == 0) {
                parse1 = strtok(NULL, ",");
                parse2 = strtok(NULL, "*");
                //printf("%s\t%s\n", parse1, parse2);
                nData->guess = atoi(parse1);
                nData->encryptionKey = atoi(parse2);
                //printf("%d\t%d\n", nData->guess, nData->encryptionKey);
                return PROTOCOL_PARSED_DET_MESSAGE;
            } else {
                //printf("INCORRECT M_ID!!!\n");
                return PROTOCOL_PARSING_FAILURE;
            }
        } else {
            // arrow to WAITING on left
            //printf("in != newline\n");
            proDecode.state = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
    default:
        // Shouldn't come here unless not in a state, FSM FAILURE
        //printf("default case ERROR\n");
        return PROTOCOL_PARSING_FAILURE;
        break;
    }
    // if we don't enter switch statement, return FAILURE
    //printf("FAILURE\n");
    return PROTOCOL_PARSING_FAILURE;
}

/**
 * This function generates all of the data necessary for the negotiation process used to determine
 * the player that goes first. It relies on the pseudo-random functionality built into the standard
 * library. The output is stored in the passed NegotiationData struct. The negotiation data is
 * generated by creating two random 16-bit numbers, one for the actual guess and another for an
 * encryptionKey used for encrypting the data. The 'encryptedGuess' is generated with an
 * XOR(guess, encryptionKey). The hash is simply an 8-bit value that is the XOR() of all of the
 * bytes making up both the guess and the encryptionKey. There is no checking for NULL pointers
 * within this function.
 * @param data The struct used for both input and output of negotiation data.
 */
void ProtocolGenerateNegotiationData(NegotiationData *data)
{
    // random 16-bit number for guess
    uint16_t actualGuess = rand();
    data->guess = actualGuess;

    // random 16-bit number for encryptionKey 
    uint16_t forEncryptKey = rand();
    data->encryptionKey = forEncryptKey;

    // XOR(guess, encryptionKey)
    data->encryptedGuess = ((data->guess) ^ (data->encryptionKey));

    // 8-bit value XOR of all bytes of guess and encryptionKey
    uint8_t forHash = (data->guess >> 8) & 0xFF;
    forHash ^= (data->guess) & 0xFF;
    forHash ^= (data->encryptionKey >> 8) & 0xFF;
    forHash ^= data->encryptionKey & 0xFF;
    data->hash = forHash;
}

/**
 * Validates that the negotiation data within 'data' is correct according to the algorithm given in
 * GenerateNegotitateData(). Used for verifying another agent's supplied negotiation data. There is
 * no checking for NULL pointers within this function. Returns TRUE if the NegotiationData struct
 * is valid or FALSE on failure.
 * @param data A filled NegotiationData struct that will be validated.
 * @return TRUE if the NegotiationData struct is consistent and FALSE otherwise.
 */
uint8_t ProtocolValidateNegotiationData(const NegotiationData *data)
{
    // How to check for data->guess and key when rand()?

    // check for data->encryptedGuess
    if (data->encryptedGuess != ((data->guess) ^ (data->encryptionKey))) {
        //return FALSE;
        return STANDARD_ERROR;
    }
    // check for data->hash
    uint8_t forHash = (data->guess >> 8) & 0xFF;
    forHash ^= (data->guess) & 0xFF;
    forHash ^= (data->encryptionKey >> 8) & 0xFF;
    forHash ^= data->encryptionKey & 0xFF;
    if (forHash != data->hash) {
        //return FALSE;
        return STANDARD_ERROR;
    }
    // NegotiationData struct is consistent
    //return TRUE;
    return SUCCESS;
}

/**
 * This function returns a TurnOrder enum type representing which agent has won precedence for going
 * first. The value returned relates to the agent whose data is in the 'myData' variable. The turn
 * ordering algorithm relies on the XOR() of the 'encryptionKey' used by both agents. The least-
 * significant bit of XOR(myData.encryptionKey, oppData.encryptionKey) is checked so that if it's a
 * 1 the player with the largest 'guess' goes first otherwise if it's a 0, the agent with the
 * smallest 'guess' goes first. The return value of TURN_ORDER_START indicates that 'myData' won,
 * TURN_ORDER_DEFER indicates that 'oppData' won, otherwise a tie is indicated with TURN_ORDER_TIE.
 * There is no checking for NULL pointers within this function.
 * @param myData The negotiation data representing the current agent.
 * @param oppData The negotiation data representing the opposing agent.
 * @return A value from the TurnOrdering enum representing which agent should go first.
 */
TurnOrder ProtocolGetTurnOrder(const NegotiationData *myData, const NegotiationData *oppData)
{
    // XOR(myData.encryptionKey, oppData.encryptionKey)
    uint32_t data = (myData->encryptionKey ^ oppData->encryptionKey);
    // check least significant bit and compare with who is closest
    if ((data & 0x01) == 1) {
        if (myData->guess > oppData->guess) {
            return TURN_ORDER_START;
        } else if (myData->guess < oppData->guess) {
            return TURN_ORDER_DEFER;
        } else { // TIE!
            return TURN_ORDER_TIE;
        }
    } else { // (data & 0x01) == 0
        if (myData->guess > oppData->guess) {
            return TURN_ORDER_DEFER;
        } else if (myData->guess < oppData->guess) {
            return TURN_ORDER_START;
        } else { // TIE!
            return TURN_ORDER_TIE;
        }
    }
}

// Got guidance from tutoring

static void StringClearMessage(char in[])
{
    int i;
    for (i = 0; i < PROTOCOL_MAX_MESSAGE_LEN; i++) {
        in[i] = '\0';
    }
}

// Got guidance from tutoring

static uint8_t FunctionChecksum(char in[])
{
    uint8_t bitOp = 0;
    int i;
    for (i = 0; i < strlen(in); i++) {
        bitOp ^= in[i];
    }
    return bitOp;
}

// Helper function used in FSM

static int ValidInput(char in)
{
    if (in >= '0' && in <= '9') {
        return SUCCESS;
    } else if (in >= 'A' && in <= 'F') {
        return SUCCESS;
    } else if (in >= 'a' && in <= 'f') {
        return SUCCESS;
    } else {
        return STANDARD_ERROR;
    }
    return STANDARD_ERROR;
}

// Got guidance from tutoring

static uint8_t AsciiToHex(char in)
{
    if (ValidInput(in) == STANDARD_ERROR) {
        return STANDARD_ERROR;
    } else {
        if (in >= '0' && in <= '9') {
            return (in - 48);
        } else if (in >= 'A' && in <= 'F') {
            return (in - 55);
        } else if (in >= 'a' && in <= 'f') {
            return (in - 87);
        }
    }
    return STANDARD_ERROR;
}
