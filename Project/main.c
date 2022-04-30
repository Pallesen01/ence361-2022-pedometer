/**********************************************************
 *
 * main
 *
 * Contains the main function loop for the program
 *
 *    Ben Stewart and Daniel Pallesen
 *    31tst of March 2022
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

uint32_t g_state;
uint32_t g_units;

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

    static circBuf_t x_circ_buff;
    static circBuf_t y_circ_buff;
    static circBuf_t z_circ_buff;

    initClock ();
    initAccl ();
    initDisplay ();
    initButtons ();
    initButtInt ();

    initCircBuf (&x_circ_buff, BUFF_SIZE); //Initializing circular buffers for each axis
    initCircBuf (&y_circ_buff, BUFF_SIZE);
    initCircBuf (&z_circ_buff, BUFF_SIZE);

    uint32_t startUpSteps = 1234;
    uint32_t totalSteps = 5789;
    uint32_t startUpDistance = 1000;
    uint32_t totalDistance = 3200;

    // Set reference orientation on start
    acceleration_raw = getAcclData();
    int8_t pitch = setReferencePitch(acceleration_raw);
    int8_t roll = setReferenceRoll(acceleration_raw);

    g_state = 4;
    g_units = 0;

    displayAcc(acceleration_mean, 0, 0, startUpSteps, startUpDistance, totalSteps, totalDistance);


    while (1)
    {
        SysCtlDelay (SysCtlClockGet () / 6);    // Approx 2
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

        //displayAcc(g_state, acceleration_mean, 0, 0, startUpSteps, startUpDistance, totalSteps, totalDistance);
    }
}
