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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "circBufT.h"

extern uint32_t g_state;
extern uint32_t g_testState;
extern uint32_t g_units;

extern uint32_t g_totalDistance;
extern uint32_t g_totalSteps;
extern uint32_t g_startUpSteps;
extern uint32_t g_startUpDistance;

/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    10
#define BUFF_SIZE 10
#define NUM_BITS 256
#define GRAVITY 9.81

typedef struct vector{
    int16_t x;
    int16_t y;
    int16_t z;
} vector3_t;


void refDelay (void);

void initClock (void);

void initDisplay (void);

void delay_ms (uint32_t ms_delay);

void delay_hz (uint32_t hz_delay);

void displayUpdate (char *str1, char *str2, int32_t num, uint8_t charLine);

void initAccl (void);

vector3_t getAcclData (void);

int32_t calcMean(int32_t sum, uint16_t i, circBuf_t *buffer);

void updateDisplay (vector3_t acceleration_mean, int8_t relative_pitch, int8_t relative_roll);

#endif /* READACC_H_ */
