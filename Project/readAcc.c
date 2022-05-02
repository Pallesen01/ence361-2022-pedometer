/**********************************************************
 *
 * readAcc.c
 *
 * Reads acceleration in three dimensions and
 * displays the resulting data on the Orbit OLED display.
 * The data can be displayed in 3 different units which can
 * be changed using the "UP" button.
 *
 *    Ben Stewart and Daniel Pallesen
 *    18th of March 2022
 *
 **********************************************************/

#include <stdint.h>
#include <stdio.h>
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
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "../OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "acc.h"
#include "i2c_driver.h"
#include "buttons4.h"
#include "circBufT.h"

typedef struct vector{
    int16_t x;
    int16_t y;
    int16_t z;
} vector3_t;



/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    10
#define BUFF_SIZE 10
#define NUM_BITS 256
#define GRAVITY 9.81
/*******************************************
 *      Local prototypes
 *******************************************/

void initClock (void);
void initDisplay (void);
void displayUpdate (char *str1, char *str2, int32_t num, uint8_t charLine);
void initAccl (void);
vector3_t getAcclData (void);

/***********************************************************
 * Initialisation functions: clock, SysTick, PWM
 ***********************************************************
 * Clock
 ***********************************************************/
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
}

/*********************************************************
 * initDisplay
 *********************************************************/
void
initDisplay (void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise ();
}

void
refDelay (void)
{

    uint32_t slow_timer = 0;
    while (slow_timer < 7) {
        SysCtlDelay (SysCtlClockGet () / 6);
        slow_timer ++;
    }

}

//*****************************************************************************
// Function to display a changing message on the display.
// The display has 4 rows of 16 characters, with 0, 0 at top left.
//*****************************************************************************
void
displayUpdate (char *str1, char *str2, int32_t num, uint8_t charLine)
{
    char text_buffer[17];           //Display fits 16 characters wide.

    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%s %s %3d", str1, str2, num);
    // Update line on display.
    OLEDStringDraw (text_buffer, 0, charLine);
}



void updateDisplay (vector3_t acceleration_mean, int8_t relative_pitch, int8_t relative_roll)
{

    uint32_t defaultGoal = 10000;

    /*if (g_state == 0) {
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("Accl raw", 0, 0);
        //Display units = raw
        displayUpdate ("Accl", "X", acceleration_mean.x, 1);
        displayUpdate ("Accl", "Y", acceleration_mean.y, 2);
        displayUpdate ("Accl", "Z", acceleration_mean.z, 3);
    } else if (g_state == 1) {
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("Accl g", 0, 0);
        //Display units = g
        displayUpdate ("Accl", "X", acceleration_mean.x / NUM_BITS, 1);
        displayUpdate ("Accl", "Y", acceleration_mean.y / NUM_BITS, 2); //Changing the mean data stored as the raw data units to g
        displayUpdate ("Accl", "Z", acceleration_mean.z / NUM_BITS, 3); //by dividing each axis' value by the number of bits.
    } else if (g_state == 2) {
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("Accl ms^-2", 0, 0);
        //Display units = ms^-2
        displayUpdate ("Accl", "X", (acceleration_mean.x * GRAVITY) / NUM_BITS, 1); //Changing the mean data stored as raw data units to
        displayUpdate ("Accl", "Y", (acceleration_mean.y * GRAVITY) / NUM_BITS, 2); //ms^-2 by multiplying by gravity before dividing by the
        displayUpdate ("Accl", "Z", (acceleration_mean.z * GRAVITY) / NUM_BITS, 3); //number of bits.
    } else if (g_state == 3) {
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("Ref Ori", 0, 0);
        displayUpdate ("Pitch", "Y", relative_pitch, 1);
        displayUpdate ("Roll", "X", relative_roll, 2);
        OLEDStringDraw ("                ", 0, 3);
        refDelay();*/

    if (g_state == 4)  {
        OLEDStringDraw ("   Steps since  ", 0, 0);
        OLEDStringDraw ("   last time    ", 0, 1);
        OLEDStringDraw ("                ", 0, 2);
        displayUpdate ("", "", g_startUpSteps, 3);
        OLEDStringDraw ("Steps", 8, 3);
    } else if (g_state == 5) {
        OLEDStringDraw (" distance since ", 0, 0);
        OLEDStringDraw ("   last time    ", 0, 1);
        OLEDStringDraw ("                ", 0, 2);
        displayUpdate ("", "", g_startUpDistance, 3);
        OLEDStringDraw ("meters", 10, 3);
    } else if (g_state == 6) {
        OLEDStringDraw (" Set goal TODO ", 0, 0);
        OLEDStringDraw ("                ", 0, 1);
        OLEDStringDraw ("                ", 0, 2);
        OLEDStringDraw ("                ", 0, 3);
    } else if (g_state == 7)  {
        //Total distance
        OLEDStringDraw ("                ", 0, 3);
        if (g_units == 0) {
            //Display distance in Km
            uint16_t totalDistanceKm = g_totalDistance / 1000;
            uint16_t remainder = g_totalDistance % 1000;
            char text_buffer[17];
            usnprintf(text_buffer, sizeof(text_buffer),"%d.%d", totalDistanceKm, remainder);
            OLEDStringDraw (text_buffer, 5, 3);
            OLEDStringDraw ("Km", 12, 3);
        } else if (g_units == 1) {
            //Display distance in miles (ew)
            uint16_t totalDistanceMiles = g_totalDistance / 1609;
            uint16_t remainder = ((g_totalDistance*1000) / 1609) % 1000;
            char text_buffer[17];
            usnprintf(text_buffer, sizeof(text_buffer),"%d.%d", totalDistanceMiles, remainder);
            OLEDStringDraw (text_buffer, 3, 3);
            OLEDStringDraw ("miles", 9, 3);
        }
        OLEDStringDraw (" total distance ", 0, 0);
       OLEDStringDraw ("                ", 0, 1);
       OLEDStringDraw ("                ", 0, 2);

    } else if (g_state == 8) {
        //Total steps
        if (g_units == 0) {
            //Display as a percentage
            uint16_t percentageOfGoal = ((g_totalSteps * 100 )/ defaultGoal);
            if (percentageOfGoal >= 100) {
                percentageOfGoal = 100;
            }
            displayUpdate ("Percent", "=", percentageOfGoal, 3);
            OLEDStringDraw ("%", 15, 3);

        } else if (g_units == 1) {
            displayUpdate ("", "", g_totalSteps, 3);
            OLEDStringDraw ("Steps", 8, 3);
        }

        OLEDStringDraw ("   Total steps  ", 0, 0);
        OLEDStringDraw ("                ", 0, 1);
        OLEDStringDraw ("                ", 0, 2);

    }

}


/*********************************************************
 * initAccl
 *********************************************************/
void
initAccl (void)
{
    char    toAccl[] = {0, 0};  // parameter, value

    /*
     * Enable I2C Peripheral
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    /*
     * Set I2C GPIO pins
     */
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    /*
     * Setup I2C
     */
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);

    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    //Initialize ADXL345 Acceleromter

    // set +-2g, 13 bit resolution, active low interrupts
    toAccl[0] = ACCL_DATA_FORMAT;
    toAccl[1] = (ACCL_RANGE_2G | ACCL_FULL_RES);
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_PWR_CTL;
    toAccl[1] = ACCL_MEASURE;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);


    toAccl[0] = ACCL_BW_RATE;
    toAccl[1] = ACCL_RATE_100HZ;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_INT;
    toAccl[1] = 0x00;       // Disable interrupts from accelerometer.
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_X;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Y;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Z;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);
}

/********************************************************
 * Function to read accelerometer
 ********************************************************/
vector3_t
getAcclData (void)
{
    char    fromAccl[] = {0, 0, 0, 0, 0, 0, 0}; // starting address, placeholders for data to be read.
    vector3_t acceleration;
    uint8_t bytesToRead = 6;

    fromAccl[0] = ACCL_DATA_X0;
    I2CGenTransmit(fromAccl, bytesToRead, READ, ACCL_ADDR);

    acceleration.x = (fromAccl[2] << 8) | fromAccl[1]; // Return 16-bit acceleration readings.
    acceleration.y = (fromAccl[4] << 8) | fromAccl[3];
    acceleration.z = (fromAccl[6] << 8) | fromAccl[5];

    return acceleration;
}


/********************************************************
 * Function to calculate the mean value
 ********************************************************/

int32_t
calcMean(int32_t sum, uint16_t i, circBuf_t *buffer)
{
    sum = 0; //Resets the sum each time function is called
    for (i = 0; i < BUFF_SIZE; i++)
    {
        sum = sum + readCircBuf (buffer); //Adding all the values in the buffer together
    }
    return (2 * sum + BUFF_SIZE) / 2 / BUFF_SIZE; //Mean calculation avoiding floating numbers.

}
