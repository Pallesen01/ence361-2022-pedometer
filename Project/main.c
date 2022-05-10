/**********************************************************
 *
 * main
 *
 * Contains the main function loop for the program
 *
 *    Ben Stewart and Daniel Pallesen
 *    31st of March 2022
 *
 **********************************************************/


// Imports
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h> // for sqrt() and pow() functions
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "../OrbitOLED/OrbitOLEDInterface.h"
#include "driverlib/interrupt.h"
#include "utils/ustdlib.h"
#include "acc.h"
#include "i2c_driver.h"
#include "buttons4.h"
#include "circBufT.h"
#include "readAcc.h"
#include "readRollPitch.h"
#include "driverlib/adc.h"
#include "adcInterrupts.h"

uint32_t g_state;
uint32_t g_units;
uint32_t g_testState;
uint32_t g_stepGoal;
uint32_t g_displayedStepGoal;
uint32_t g_potiVal;
circBuf_t g_poti_circ_buff;
uint8_t g_updateGoalScreen;

uint32_t g_totalDistance;
uint32_t g_totalSteps;
uint32_t g_startUpSteps;
uint32_t g_startUpDistance;

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


/********************************************************
 * main
 ********************************************************/
int
main (void)
{
    vector3_t acceleration_raw = {0,0,0};
    vector3_t acceleration_mean = {0,0,0};

    int32_t sum;
    uint16_t i;
    uint32_t buttonTimer = 0;
    uint8_t buttonStatus;
    uint8_t downButPressed=0;
    uint32_t prevPotiVal;

    static circBuf_t x_circ_buff;
    static circBuf_t y_circ_buff;
    static circBuf_t z_circ_buff;


    initClock ();
    initADC ();
    initAccl ();
    initDisplay ();
    initSwitches();
    initButtons ();
    initButtInt ();
    initSysTick();

    // Enable interrupts to the processor.
    IntMasterEnable();

    initCircBuf (&x_circ_buff, BUFF_SIZE); //Initializing circular buffers for each axis
    initCircBuf (&y_circ_buff, BUFF_SIZE);
    initCircBuf (&z_circ_buff, BUFF_SIZE);

    initCircBuf (&g_poti_circ_buff, BUFF_SIZE);

    g_startUpSteps = 1234;
    g_totalSteps = 5789;
    g_startUpDistance = 1000;
    g_totalDistance = 3200;
    g_stepGoal = 10000;
    uint16_t mainLoopHZ = 120;

    // Set reference orientation on start
    acceleration_raw = getAcclData();
    int8_t pitch = setReferencePitch(acceleration_raw);
    int8_t roll = setReferenceRoll(acceleration_raw);

    g_state = 8;
    g_units = 0;
    g_testState = 0;

    updateSwitches();

    updateDisplay(acceleration_mean, 0, 0);

    while (1)
    {
        delay_hz(mainLoopHZ); // ~ 120hz
        //displayAcc(g_state, acceleration_raw);
        acceleration_raw = getAcclData();

        // Write acceleration values to circular buffers
        writeCircBuf (&x_circ_buff, acceleration_raw.x);
        writeCircBuf (&y_circ_buff, acceleration_raw.y);
        writeCircBuf (&z_circ_buff, acceleration_raw.z);

        // Calculate mean acceleration along each axis
        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);

        g_potiVal = calcMean(sum, i, &g_poti_circ_buff);

        updateSwitches();
        updateButtons();

        buttonStatus = checkButton(DOWN);
        if (buttonStatus == RELEASED) {
            downButPressed = 0;
        }

        if (buttonStatus == PUSHED) {
                    buttonTimer = 0;
                    downButPressed = 1;
                }

        if (g_state != 6) {
            buttonTimer++;

          if (g_state != 6 && buttonStatus == NO_CHANGE && downButPressed) {
              // If down button has been held for 0.6 seconds
            if (buttonTimer >= (0.6 * mainLoopHZ)) {
                // Update step goal if button pressed for more than 2 seconds
                g_startUpSteps = 0;
                g_totalSteps = 0;
                g_startUpDistance = 0;
                g_totalDistance = 0;
                OLEDStringDraw ("  distance and  ", 0, 2);
                OLEDStringDraw ("   steps reset  ", 0, 3);
                buttonStatus = RELEASED;
                downButPressed = 0;
                }
            }
        }


        if (g_state == 6 && !similarValues(prevPotiVal,g_potiVal)) {
            updateDisplay(acceleration_mean, 0, 0);
        }

        prevPotiVal = g_potiVal;




    }

}
