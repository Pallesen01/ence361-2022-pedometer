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

/********************************************************
 * Function to calculate an angle given an accelerometer value
 ********************************************************/
int8_t
calcPitch(vector3_t raw_units, vector3_t reference_units)
{
       double tan_pitch;
       double x_square = pow(raw_units.x, 2);
       double z_square =  pow(raw_units.z, 2);
       tan_pitch = raw_units.y/sqrt(x_square + z_square);
       return atan(tan_pitch);
}

int8_t
calcRoll(vector3_t raw_units, vector3_t reference_units)
{
    return 0; // placeholder
}


/********************************************************
 * main
 ********************************************************/
int
main (void)
{
    vector3_t acceleration_raw;
    vector3_t acceleration_mean;
    vector3_t reference_orientation;

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
    reference_orientation = getAcclData();

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
            reference_orientation = getAcclData(); //Resets reference orientation
        }


        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);

        //Display units = Radians
        displayUpdate ("Roll", "X", calcRoll(acceleration_raw, reference_orientation), 1);
        displayUpdate ("Pitch", "Y", calcPitch(acceleration_raw, reference_orientation), 2);
        // displayUpdate ("Yaw", "Z", acceleration_mean.z / NUM_BITS, 3); //by dividing each axis' value by the number of bits.
    }
}





