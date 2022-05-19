/**********************************************************
 *
 * display.c
 *
 * Houses functions related to displaying data on
 * the OLED display
 *
 *    Ben Stewart and Daniel Pallesen
 *    19th of May 2022
 *
 **********************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "../OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "acc.h"
#include "i2c_driver.h"
#include "buttons4.h"
#include "circBufT.h"
#include <math.h>


/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    10
#define BUFF_SIZE 10
#define NUM_BITS 256
#define GRAVITY 9.81

//*****************************************************************************
// Function to display a changing message on the display.
// The display has 4 rows of 16 characters, with 0, 0 at top left.
//*****************************************************************************
void
displayUpdate (char *str1, char *str2, int32_t num, uint8_t charLine)
{
    char text_buffer[17];           //Display fits 16 characters wide.

    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%s %s %3d", str1, str2, num);
    // Update line on display.
    OLEDStringDraw (text_buffer, 0, charLine);
}

/**
 * Delays for specific number of milliseconds
 */
void delay_ms (uint32_t ms_delay)
{
    SysCtlDelay(ms_delay * (SysCtlClockGet() / 3 / 1000));
}

/**
 * Runs display at specific frequency
 */
void delay_hz (uint32_t hz_delay)
{
    delay_ms(1000/hz_delay);
}

/**
 * Calculates value to display for goal value
 */
int32_t goalDisplayVal ()
{
    int32_t returnVal = 100 * round(g_potiVal/20.425);
    if (returnVal < 0) {
        returnVal = 0;
    }
    g_displayedStepGoal = returnVal;
    return returnVal;
}

/*
 * Displays different information based on the global state variable
 */
void updateDisplay ()
{
    if (g_state == 6) {
        OLEDStringDraw ("    Set goal    ", 0, 0);
        OLEDStringDraw ("                ", 0, 1);
        displayUpdate ("","", goalDisplayVal(), 2);
        OLEDStringDraw ("Steps", 8, 2);
        OLEDStringDraw ("                ", 0, 3);

    } else if (g_state == 7)  {
        //Total distance
        OLEDStringDraw ("                ", 0, 3);
        if (g_units == 0) {
            //Display distance in Km
            uint16_t totalDistanceKm = g_totalDistance / 1000;
            uint16_t remainder = g_totalDistance % 1000;
            char text_buffer[17];
            if ((remainder / 100) == 0) {
                //If there is a 0 in the first decimal place spot
                usnprintf(text_buffer, sizeof(text_buffer),"%d.%d%d", totalDistanceKm, 0, remainder);
            } else {
                usnprintf(text_buffer, sizeof(text_buffer),"%d.%d", totalDistanceKm, remainder);
            }
            OLEDStringDraw (text_buffer, 5, 3);
            OLEDStringDraw ("Km", 12, 3);
        } else if (g_units == 1) {
            //Display distance in miles
            uint16_t totalDistanceMiles = g_totalDistance / 1609;
            uint16_t remainder = ((g_totalDistance*1000) / 1609) % 1000;
            char text_buffer[17];
            if ((remainder / 100) == 0) {
                usnprintf(text_buffer, sizeof(text_buffer),"%d.%d%d", totalDistanceMiles, 0, remainder);
            } else {
                usnprintf(text_buffer, sizeof(text_buffer),"%d.%d", totalDistanceMiles, remainder);
            }
            OLEDStringDraw (text_buffer, 3, 3);
            OLEDStringDraw ("miles", 9, 3);
        }
        OLEDStringDraw ("Total distance  ", 0, 0);
       OLEDStringDraw ("                ", 0, 1);
       OLEDStringDraw ("                ", 0, 2);

    } else if (g_state == 8) {
        //Total steps
        uint16_t percentageOfGoal;
        if (g_units == 1) {
            //Display as a percentage
            if (g_stepGoal != 0) {
                percentageOfGoal = ((g_totalSteps * 100 )/ g_stepGoal);
            } else {
                percentageOfGoal = 100;
            }

            if (percentageOfGoal >= 100) {
                percentageOfGoal = 100;
            }
            displayUpdate ("Percent", "=", percentageOfGoal, 3);
            OLEDStringDraw ("% ", 14, 3);

        } else if (g_units == 0) {
            displayUpdate ("", "", g_totalSteps, 3);
            OLEDStringDraw ("Steps", 8, 3);
        }

        OLEDStringDraw ("      Steps     ", 0, 0);
        displayUpdate ("  Goal", "=", g_stepGoal, 1);
        OLEDStringDraw ("                ", 0, 2);

    }

}

/**
 *Returns true if the two numbers entered are very close
 */
int similarValues(uint32_t val1, uint32_t val2) {
    uint8_t threshold = 2;
    if (val1 >= val2) {
        if (val1 - val2 <= threshold) {
            return 1;
        } else {
            return 0;
        }
    } else {
        if (val2 - val1 <= threshold) {
            return 1;
        } else {
            return 0;
        }
    }
}
