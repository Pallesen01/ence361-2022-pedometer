/**********************************************************
 *
 * main
 *
 * Contains the main function lopo for the program
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
#include "utils/ustdlib.h"
#include "acc.h"
#include "i2c_driver.h"
#include "buttons4.h"
#include "circBufT.h"
#include "readAcc.h"
#include "readRollPitch.h"


/********************************************************
 * main
 ********************************************************/
int
main (void)
{
    vector3_t acceleration_raw;
    vector3_t acceleration_mean;
    vector3_t reference_acceleration;
    int8_t relative_pitch;
    int8_t relative_roll;

    int32_t sum;
    uint8_t butState;
    uint16_t i;

    circBuf_t x_circ_buff;
    circBuf_t y_circ_buff;
    circBuf_t z_circ_buff;

    initClock ();
    initAccl ();
    initDisplay ();
    initButtons ();

    initCircBuf (&x_circ_buff, BUFF_SIZE); //Initializing circular buffers for each axis
    initCircBuf (&y_circ_buff, BUFF_SIZE);
    initCircBuf (&z_circ_buff, BUFF_SIZE);

    OLEDStringDraw ("Orientation", 0, 0);
    reference_acceleration = getAcclData();
    relative_pitch = calcPitch(reference_acceleration, 0);
    relative_roll = calcRoll(reference_acceleration, 0);

    while (1)
    {
        SysCtlDelay (SysCtlClockGet () / 6);    // Approx 2 Hz
        acceleration_raw = getAcclData();

        writeCircBuf (&x_circ_buff, acceleration_raw.x);
        writeCircBuf (&y_circ_buff, acceleration_raw.y);
        writeCircBuf (&z_circ_buff, acceleration_raw.z);

        updateButtons ();

        butState = checkButton (DOWN); //Gets the current state of the DOWN button

        if (butState == PUSHED) { /*Checks if the 'DOWN' button has been pushed
                                    Note, button has to be held for a short period to trigger.*/
            reference_acceleration = getAcclData();
            relative_pitch = calcPitch(reference_acceleration, 0);
            relative_roll = calcRoll(reference_acceleration, 0); //Resets reference orientation
        }


        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);

        //Display units = Degrees
        displayUpdate ("Pitch", "Y", calcPitch(acceleration_raw, relative_pitch), 1);
        displayUpdate ("Roll", "X", calcRoll(acceleration_raw, relative_roll), 2);

    }
}


/********************************************************
 * main
 ********************************************************/
/*int
main (void)
{
    vector3_t acceleration_raw;
    vector3_t acceleration_mean;

    uint8_t unitState = 0; //Initially display Raw units
    int32_t sum;
    uint8_t butState;
    uint16_t i;

    circBuf_t x_circ_buff;
    circBuf_t y_circ_buff;
    circBuf_t z_circ_buff;

    initClock ();
    initAccl ();
    initDisplay ();
    initButtons ();

    initCircBuf (&x_circ_buff, BUFF_SIZE); //Initializing circular buffers for each axis
    initCircBuf (&y_circ_buff, BUFF_SIZE);
    initCircBuf (&z_circ_buff, BUFF_SIZE);

    OLEDStringDraw ("Accelerometer", 0, 0);

    while (1)
    {
        SysCtlDelay (SysCtlClockGet () / 6);    // Approx 2 Hz
        acceleration_raw = getAcclData();

        writeCircBuf (&x_circ_buff, acceleration_raw.x);
        writeCircBuf (&y_circ_buff, acceleration_raw.y);
        writeCircBuf (&z_circ_buff, acceleration_raw.z);

        updateButtons ();

        butState = checkButton (UP); //Gets the current state of the 'UP button

        if (butState == PUSHED) { //Checks if the 'UP' button has been pushed
            //Note, button has to be held for a short period to trigger a change in units
            //Not sure of a way around this.
            unitState ++; //Changes the units
            if (unitState == 3) {
                //Checks to see if the variable has gone out of range as only 0, 1 and 2
                //are valid unitState values. More units could be added in future.
                unitState = 0;
            }
        }

        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);


        if (unitState == 0) {
            //Display units = raw
            displayUpdate ("Accl", "X", acceleration_mean.x, 1);
            displayUpdate ("Accl", "Y", acceleration_mean.y, 2);
            displayUpdate ("Accl", "Z", acceleration_mean.z, 3);
        } else if (unitState == 1) {
            //Display units = g
            displayUpdate ("Accl", "X", acceleration_mean.x / NUM_BITS, 1);
            displayUpdate ("Accl", "Y", acceleration_mean.y / NUM_BITS, 2); //Changing the mean data stored as the raw data units to g
            displayUpdate ("Accl", "Z", acceleration_mean.z / NUM_BITS, 3); //by dividing each axis' value by the number of bits.
        } else {
            //Display units = ms^-2
            displayUpdate ("Accl", "X", (acceleration_mean.x * GRAVITY) / NUM_BITS, 1); //Changing the mean data stored as raw data units to
            displayUpdate ("Accl", "Y", (acceleration_mean.y * GRAVITY) / NUM_BITS, 2); //ms^-2 by multiplying by gravity before dividing by the
            displayUpdate ("Accl", "Z", (acceleration_mean.z * GRAVITY) / NUM_BITS, 3); //number of bits.
        }
    }
}*/
