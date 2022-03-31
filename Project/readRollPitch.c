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

// CONSTANTS

#define PI 3.1415
#define RAD_TO_DEG 57.3

/********************************************************
 * Function to return the sign of an integer
 ********************************************************/
int8_t
getSign(int32_t x)
{
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}


/********************************************************
 * Function to calculate pitch given an accelerometer values
 ********************************************************/
int8_t
calcPitch(vector3_t acceleration, int8_t relative_pitch)
{
    double x_square = pow(acceleration.x, 2);
    double z_square =  pow(acceleration.z, 2);

    return (atan2(acceleration.y, sqrt(x_square + z_square))*RAD_TO_DEG) - relative_pitch;

}

/********************************************************
 * Function to calculate roll given an accelerometer values
 ********************************************************/
int8_t
calcRoll(vector3_t acceleration, int8_t relative_roll)
{
    double x_square = pow(acceleration.x, 2);
    double z_square =  pow(acceleration.z, 2);

    return (atan2(-acceleration.x, acceleration.z)*RAD_TO_DEG) - relative_roll;

}

/********************************************************
 * resetReferenceOrientation
 ********************************************************/
void
setReferenceOrientation(vector3_t acceleration, int8_t* pitch_ptr, int8_t* roll_ptr)
{
    *pitch_ptr = calcPitch(acceleration, 0);
    *roll_ptr = calcRoll(acceleration, 0);
}




