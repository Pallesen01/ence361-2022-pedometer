/**********************************************************
 *
 * readRollPitch
 *
 * Calculates the reference orientation of the board when the program is
 * first loaded and then displays pitch and roll in radians.
 * Reference orientation can be reset by holding DOWN.
 *
 *    Ben Stewart and Daniel Pallesen
 *    18th of March 2022
 *
 **********************************************************/

#include <stdint.h>
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
#include "../OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "acc.h"
#include "i2c_driver.h"
#include "buttons4.h"
#include "circBufT.h"
#include "readAcc.h"

/********************************************************
 * Function to calculate an angle given an accelerometer value
 ********************************************************/
int8_t
calcAngle(int32_t raw_units, int32_t reference_units)
{
    return raw_units/NUM_BITS*10
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
            //Display units = degrees
            OLEDStringDraw ("Orientation", 0, 0);
            displayUpdate ("Roll", "X", calcAngle(acceleration_raw.x, reference_orientation.x), 1);
            displayUpdate ("Pitch", "Y", calcAngle(acceleration_raw.y, reference_orientation.y), 2);
            // displayUpdate ("Yaw", "Z", acceleration_mean.z / NUM_BITS, 3); //by dividing each axis' value by the number of bits.
    }
}
