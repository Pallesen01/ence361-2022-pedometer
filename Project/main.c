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
