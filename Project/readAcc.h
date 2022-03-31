/**********************************************************
 *
 * readAcc.h
 *
 * Reads acceleration in three dimensions and
 * displays the resulting data on the Orbit OLED display.
 * The data can be displayed in 3 different units which can
 * be changed using the "UP" button.
 *
 *    Ben Stewart and Daniel Pallesen
 *    24th of March 2022
 *
 **********************************************************/

#ifndef READACC_H_
#define READACC_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "circBufT.h"

/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    10
#define BUFF_SIZE 10
#define NUM_BITS 256
#define GRAVITY 9.81

typedef struct vector{
    int32_t x;
    int32_t y;
    int32_t z;
} vector3_t;


void refDelay (void);

void initClock (void);

void initDisplay (void);

void displayUpdate (char *str1, char *str2, int16_t num, uint8_t charLine);

void initAccl (void);

vector3_t getAcclData (void);

uint16_t calcMean(int32_t sum, uint16_t i, circBuf_t *buffer);

void displayAcc (uint32_t state, vector3_t acceleration_mean, int8_t relative_pitch, int8_t relative_roll);

#endif /* READACC_H_ */
