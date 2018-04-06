//Kameron Gill 
//Hasson Qayum

//Field.c

#include "Field.h"
#include "BOARD.h"
#include "Protocol.h"


//functions below================

void FieldInit(Field *f, FieldPosition p)
{
    int i = 0; //row
    int j = 0; //columns

    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            f->field[i][j] = p;

        }
    }

    // init boat lives for each boat type
    f->hugeBoatLives = FIELD_BOAT_LIVES_HUGE;
    f->largeBoatLives = FIELD_BOAT_LIVES_LARGE;
    f->smallBoatLives = FIELD_BOAT_LIVES_SMALL;
    f->mediumBoatLives = FIELD_BOAT_LIVES_MEDIUM;

}

FieldPosition FieldAt(const Field *f, uint8_t row, uint8_t col)
{
    return f->field[row][col];

}

FieldPosition FieldSetLocation(Field *f, uint8_t row, uint8_t col, FieldPosition p)
{
    FieldPosition temp = f->field[row][col];
    f->field[row][col] = p;
    return temp;

}

//got help from my tutor on this function

uint8_t FieldAddBoat(Field *f, uint8_t row, uint8_t col, BoatDirection dir, BoatType type)
{
    int boatLength;
    //    BoatDirection tempDir;
    BoatType tempBoat = type;

    if (tempBoat == FIELD_BOAT_SMALL) {
        boatLength = 3;
        tempBoat = FIELD_POSITION_SMALL_BOAT;
    } else if (tempBoat == FIELD_BOAT_MEDIUM) {
        boatLength = 4;
        tempBoat = FIELD_POSITION_MEDIUM_BOAT;
    } else if (tempBoat == FIELD_BOAT_LARGE) {
        boatLength = 5;
        tempBoat = FIELD_POSITION_LARGE_BOAT;
    } else if (tempBoat == FIELD_BOAT_HUGE) {

        boatLength = 6;
        tempBoat = FIELD_POSITION_HUGE_BOAT;
    } else {
        boatLength = 0;
        return STANDARD_ERROR;
    }
    int i;

    switch (dir) {
    case FIELD_BOAT_DIRECTION_NORTH:
        //int i;
        for (i = 0; i < boatLength; i++) {
            if (row - i < 0) {
                //printf("BN error\n");
                return STANDARD_ERROR;
            }
            if (f->field[row - i][col] != FIELD_POSITION_EMPTY)
                //printf("BN empty error\n");
                return STANDARD_ERROR;

        }
        // int i ;
        for (i = 0; i < boatLength; i++) {
            f->field[row - i][col] = tempBoat;
        }
        return TRUE;

        break;
    case FIELD_BOAT_DIRECTION_SOUTH:
        // int i = 0;
        for (i = 0; i < boatLength; i++) {
            if (row + i >= FIELD_ROWS) {
                return STANDARD_ERROR;
            }
            if (f->field[row + i][col] != FIELD_POSITION_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        // i = 0;
        for (i = 0; i < boatLength; i++) {
            f->field[row + i][col] = tempBoat;
        }
        return TRUE;

        break;


    case FIELD_BOAT_DIRECTION_EAST:
        //int i = 0;
        for (i = 0; i < boatLength; i++) {
            if (col + i >= FIELD_COLS) {
                return STANDARD_ERROR;
            }
            if (f->field[row][col + i] != FIELD_POSITION_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        // i = 0;
        for (i = 0; i < boatLength; i++) {
            f->field[row][col + i] = tempBoat;
        }
        return TRUE;

        break;



    case FIELD_BOAT_DIRECTION_WEST:
        // int i = 0;
        for (i = 0; i < boatLength; i++) {
            if (col - i < 0) {
                return STANDARD_ERROR;
            }
            if (f->field[row][col - i] != FIELD_POSITION_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        //i = 0;
        for (i = 0; i < boatLength; i++) {
            f->field[row][col - i] = tempBoat;
        }
        return TRUE;
        break;



    }
    return SUCCESS;

}
//got help from my tutor on this function

FieldPosition FieldRegisterEnemyAttack(Field *f, GuessData *gData)
{
    int temp = FieldAt(f, gData->row, gData->col);

    if (temp == FIELD_POSITION_SMALL_BOAT) {
        f->smallBoatLives--;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
        gData->hit = FIELD_POSITION_HIT;
    } else if (temp == FIELD_POSITION_MEDIUM_BOAT) {
        f->mediumBoatLives--;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
        gData->hit = FIELD_POSITION_HIT;
    } else if (temp == FIELD_POSITION_LARGE_BOAT) {
        f->largeBoatLives--;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
        gData->hit = FIELD_POSITION_HIT;
    } else if (temp == FIELD_POSITION_HUGE_BOAT) {
        f->hugeBoatLives--;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
        gData->hit = FIELD_POSITION_HIT;
    } else {
        f->field[gData->row][gData->col] = FIELD_POSITION_MISS;
        gData->hit = FIELD_POSITION_MISS;
    }

    return temp;

}
//got help from my tutor on this function

FieldPosition FieldUpdateKnowledge(Field *f, const GuessData *gData)
{
    FieldPosition temp = f->field[gData->row][gData->col];


    if (gData->hit == HIT_MISS)
        f->field[gData->row][gData->col] = FIELD_POSITION_EMPTY;
    else if (gData->hit != HIT_MISS) {


        switch (gData->hit) {


        case HIT_SUNK_MEDIUM_BOAT:
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->mediumBoatLives = 0;
            break;
        case HIT_SUNK_SMALL_BOAT:
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->smallBoatLives = 0;
            break;
        case HIT_SUNK_LARGE_BOAT:
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->largeBoatLives = 0;
            break;
        case HIT_SUNK_HUGE_BOAT:
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->hugeBoatLives = 0;
            break;
        }
    }
    return temp;
}

uint8_t FieldGetBoatStates(const Field *f)
{
    // bitfield of lives of boats
    uint8_t temp = FIELD_BOAT_STATUS_HUGE | FIELD_BOAT_STATUS_LARGE |
            FIELD_BOAT_STATUS_MEDIUM | FIELD_BOAT_STATUS_SMALL;

    if (f->hugeBoatLives == 0) { // if boat type lives are 0
        // xor with the status of all 4 boats to set bit to 0 
        temp ^= FIELD_BOAT_STATUS_HUGE;
    }
    if (f->largeBoatLives == 0) {
        temp ^= FIELD_BOAT_STATUS_LARGE;
    }
    if (f->mediumBoatLives == 0) {
        temp ^= FIELD_BOAT_STATUS_MEDIUM;
    }
    if (f->smallBoatLives == 0) {
        temp ^= FIELD_BOAT_STATUS_SMALL;
    }
    return temp;
}
