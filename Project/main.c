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

uint32_t g_state;
uint32_t g_units;
uint32_t g_testState;
uint32_t g_stepGoal;
uint32_t g_displayedStepGoal;
uint32_t g_potiVal;

uint32_t g_totalDistance;
uint32_t g_totalSteps;
uint32_t g_startUpSteps;
uint32_t g_startUpDistance;



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
    uint32_t ulValue;

    static circBuf_t x_circ_buff;
    static circBuf_t y_circ_buff;
    static circBuf_t z_circ_buff;
    static circBuf_t poti_circ_buff;


    initClock ();
    initAccl ();
    initDisplay ();
    initSwitches();
    initButtons ();
    initButtInt ();

    // Enable interrupts to the processor.
    IntMasterEnable();

    initCircBuf (&x_circ_buff, BUFF_SIZE); //Initializing circular buffers for each axis
    initCircBuf (&y_circ_buff, BUFF_SIZE);
    initCircBuf (&z_circ_buff, BUFF_SIZE);

    initCircBuf (&poti_circ_buff, BUFF_SIZE);

    g_startUpSteps = 1234;
    g_totalSteps = 5789;
    g_startUpDistance = 1000;
    g_totalDistance = 3200;
    g_stepGoal = 10000;

    // Set reference orientation on start
    acceleration_raw = getAcclData();
    int8_t pitch = setReferencePitch(acceleration_raw);
    int8_t roll = setReferenceRoll(acceleration_raw);

    g_state = 4;
    g_units = 0;
    g_testState = 0;

    updateSwitches();

    updateDisplay(acceleration_mean, 0, 0);

    while (1)
    {
        delay_ms(8); // ~ 120hz
        //displayAcc(g_state, acceleration_raw);
        acceleration_raw = getAcclData();

        ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);

        // Write acceleration values to circular buffers
        writeCircBuf (&x_circ_buff, acceleration_raw.x);
        writeCircBuf (&y_circ_buff, acceleration_raw.y);
        writeCircBuf (&z_circ_buff, acceleration_raw.z);

        // Potentiometer circular buffer
        writeCircBuf (&poti_circ_buff, ulValue);

        // Calculate mean acceleration along each axis
        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);

        g_potiVal = calcMean(sum, i, &poti_circ_buff);
        test = g_potiVal;

        updateSwitches();

        if (g_state == 6) {
            updateDisplay(acceleration_mean, 0, 0);
        }



    }

}
