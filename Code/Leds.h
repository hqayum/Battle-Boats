//Kameron Gill
//kgill2@ucsc.edu

//Leds.h

#ifndef LEDS_H
#define LEDS_H

//including libraries
#include "BOARD.h"
#include <xc.h>


//LEDS_INIT
#define LEDS_INIT() do{ \
    TRISE = 0; \
    LATE = 0; \
} while(0)
	




#define LEDS_GET() LATE

#define LEDS_SET(x) (LATE = x)

#endif
