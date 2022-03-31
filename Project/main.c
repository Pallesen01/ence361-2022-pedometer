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

    uint8_t unitState = 3; // Start displaying reference orientation
    uint8_t prevState = 0;
    uint32_t slow_timer = 0;
    int32_t sum;
    uint8_t upButState;
    uint8_t downButState;
    uint16_t i;
    int8_t relative_pitch;
    int8_t relative_roll;

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

    // Set reference orientation on start
    acceleration_raw = getAcclData();
    setReferenceOrientation(acceleration_raw, &relative_pitch, &relative_roll);

    while (1)
    {
        SysCtlDelay (SysCtlClockGet () / 6);    // Approx 2 Hz
        acceleration_raw = getAcclData();

        // Write acceleration values to circular buffers
        writeCircBuf (&x_circ_buff, acceleration_raw.x);
        writeCircBuf (&y_circ_buff, acceleration_raw.y);
        writeCircBuf (&z_circ_buff, acceleration_raw.z);

        // Calculate mean acceleration along each axis
        acceleration_mean.x = calcMean(sum, i, &x_circ_buff); //Calculates the mean for each axis using the values stored
        acceleration_mean.y = calcMean(sum, i, &y_circ_buff); //in each circular buffer
        acceleration_mean.z = calcMean(sum, i, &z_circ_buff);

        // TODO Fix mean acceleration, using raw for now
        acceleration_mean = acceleration_raw;

        updateButtons ();

        upButState = checkButton (UP); //Gets the current state of the 'UP button
        downButState = checkButton (DOWN); //Gets the current state of the DOWN button

        if (upButState == PUSHED) { //Checks if the 'UP' button has been pushed
            //Note, button has to be held for a short period to trigger a change in units
            //Not sure of a way around this.
            if (unitState == 2) {
                //Checks to see if the variable has gone out of range as only 0, 1 and 2
                //are valid unitState values. More units could be added in future.
                unitState = 0;
            } else {
                unitState ++; //Changes the units
            }
        }

        // Update timer for reference orientation screen
        slow_timer++;
        if (downButState == PUSHED) {
            setReferenceOrientation(acceleration_raw, &relative_pitch, &relative_roll);
            prevState = unitState;
            unitState = 3;
            slow_timer = 0;
            // TODO Update display for 3 seconds
        }
        // Approx 3 seconds
        if (slow_timer >= 6 && unitState == 3) {
            unitState = prevState;
        }


        if (unitState == 0) {
            OLEDStringDraw ("                ", 0, 0);
            OLEDStringDraw ("Accl raw", 0, 0);
            //Display units = raw
            displayUpdate ("Accl", "X", acceleration_mean.x, 1);
            displayUpdate ("Accl", "Y", acceleration_mean.y, 2);
            displayUpdate ("Accl", "Z", acceleration_mean.z, 3);
        } else if (unitState == 1) {
            OLEDStringDraw ("                ", 0, 0);
            OLEDStringDraw ("Accl g", 0, 0);
            //Display units = g
            displayUpdate ("Accl", "X", acceleration_mean.x / NUM_BITS, 1);
            displayUpdate ("Accl", "Y", acceleration_mean.y / NUM_BITS, 2); //Changing the mean data stored as the raw data units to g
            displayUpdate ("Accl", "Z", acceleration_mean.z / NUM_BITS, 3); //by dividing each axis' value by the number of bits.
        } else if (unitState == 2) {
            OLEDStringDraw ("                ", 0, 0);
            OLEDStringDraw ("Accl ms^-2", 0, 0);
            //Display units = ms^-2
            displayUpdate ("Accl", "X", (acceleration_mean.x * GRAVITY) / NUM_BITS, 1); //Changing the mean data stored as raw data units to
            displayUpdate ("Accl", "Y", (acceleration_mean.y * GRAVITY) / NUM_BITS, 2); //ms^-2 by multiplying by gravity before dividing by the
            displayUpdate ("Accl", "Z", (acceleration_mean.z * GRAVITY) / NUM_BITS, 3); //number of bits.
        } else {
            OLEDStringDraw ("                ", 0, 0);
            OLEDStringDraw ("Ref Ori", 0, 0);
            displayUpdate ("Pitch", "Y", relative_pitch, 1);
            displayUpdate ("Roll", "X", relative_roll, 2);
            OLEDStringDraw ("                ", 0, 3);


        }
    }
}
