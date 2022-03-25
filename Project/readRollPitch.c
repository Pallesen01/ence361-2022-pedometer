/**********************************************************
 *
 * readRollPitch
 *
 * Calculates the reference orientation of the board when the program is
 * first loaded and then displays pitch and roll in radians.
 * Reference orientation can be reset by holding DOWN.
 *
 *    Ben Stewart and Daniel Pallesen
 *    24th of March 2022
 *
 **********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h> // for sqrt() function
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

// CONSTANTS

#define PI 3.1415
#define RAD_TO_DEG 57.3

/********************************************************
 * Function to calculate pitch given an accelerometer values
 ********************************************************/
int8_t
calcPitch(vector3_t raw_acceleration, int8_t relative_pitch)
{
    double x_square = pow(raw_acceleration.x, 2);
    double z_square =  pow(raw_acceleration.z, 2);
    return (atan2(raw_acceleration.y, sqrt(x_square + z_square))*RAD_TO_DEG) - relative_pitch;
}

/********************************************************
 * Function to calculate roll given an accelerometer values
 ********************************************************/
int8_t
calcRoll(vector3_t raw_acceleration, int8_t relative_roll)
{
    return (atan2(-raw_acceleration.x, raw_acceleration.z)*RAD_TO_DEG) - relative_roll;
}

/********************************************************
 * Function to calculate the current orientation given the current and reference orientation
 ********************************************************/
vector3_t
calcRelativeOrientation(vector3_t raw_units, vector3_t reference_units)
{
    vector3_t relative_orientation;
    relative_orientation.x = raw_units.x - reference_units.x;
    relative_orientation.y = raw_units.y - reference_units.y;
    relative_orientation.z = raw_units.z - reference_units.z;
    return relative_orientation;
}

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

        //Display units = Degrees (CHANGE TO RADIANS)
        // relative_orientation = calcRelativeOrientation(acceleration_raw, reference_orientation);
        displayUpdate ("Roll", "X", calcRoll(acceleration_raw, relative_roll), 1);
        displayUpdate ("Pitch", "Y", calcPitch(acceleration_raw, relative_pitch), 2);
    }
}





