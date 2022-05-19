/**********************************************************
 *
 * display.h
 *
 * Houses functions related to displaying data on
 * the OLED display
 *
 *    Ben Stewart and Daniel Pallesen
 *    19th of May 2022
 *
 **********************************************************/

#ifndef DSIPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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


void
delay_ms (uint32_t ms_delay);

void
delay_hz (uint32_t hz_delay);

void
displayUpdate (char *str1, char *str2, int32_t num, uint8_t charLine);

void
updateDisplay ();

int
similarValues(uint32_t val1, uint32_t val2)

#endif /* READACC_H_ */
