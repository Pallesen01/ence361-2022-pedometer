// *******************************************************
// 
// buttons4.c
//
// Support for a set of FOUR specific buttons on the Tiva/Orbit.
// ENCE361 sample code.
// The buttons are:  UP and DOWN (on the Orbit daughterboard) plus
// LEFT and RIGHT on the Tiva.
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// P.J. Bones UCECE
// Last modified:  7.2.2018
// 
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "inc/tm4c123gh6pm.h"  // Board specific defines (for PF0)
#include "buttons4.h"
#include "readAcc.h"
#include "acc.h"
#include "readRollPitch.h"

// *******************************************************
// Globals to module
// *******************************************************
static bool but_state[NUM_BUTS];	// Corresponds to the electrical state
static uint8_t but_count[NUM_BUTS];
static bool but_flag[NUM_BUTS];
static bool but_normal[NUM_BUTS];   // Corresponds to the electrical state

static bool sw_state[NUM_SW];
static uint8_t sw_count[NUM_SW];
static bool sw_normal[NUM_SW];

// *******************************************************
// initButtons: Initialise the variables associated with the set of buttons
// defined by the constants in the buttons2.h header file.
void
initButtons (void)
{
	int i;

	// UP button (active HIGH)
    SysCtlPeripheralEnable (UP_BUT_PERIPH);
    GPIOPinTypeGPIOInput (UP_BUT_PORT_BASE, UP_BUT_PIN);
    GPIOPadConfigSet (UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    but_normal[UP] = UP_BUT_NORMAL;
	// DOWN button (active HIGH)
    SysCtlPeripheralEnable (DOWN_BUT_PERIPH);
    GPIOPinTypeGPIOInput (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    GPIOPadConfigSet (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    but_normal[DOWN] = DOWN_BUT_NORMAL;
    // LEFT button (active LOW)
    SysCtlPeripheralEnable (LEFT_BUT_PERIPH);
    GPIOPinTypeGPIOInput (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    GPIOPadConfigSet (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    but_normal[LEFT] = LEFT_BUT_NORMAL;
    // RIGHT button (active LOW)
      // Note that PF0 is one of a handful of GPIO pins that need to be
      // "unlocked" before they can be reconfigured.  This also requires
      //      #include "inc/tm4c123gh6pm.h"
    SysCtlPeripheralEnable (RIGHT_BUT_PERIPH);
    //---Unlock PF0 for the right button:
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0; //PF0 unlocked
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;
    GPIOPinTypeGPIOInput (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    GPIOPadConfigSet (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    but_normal[RIGHT] = RIGHT_BUT_NORMAL;

	for (i = 0; i < NUM_BUTS; i++)
	{
		but_state[i] = but_normal[i];
		but_count[i] = 0;
		but_flag[i] = false;
	}
}

void
initSwitches (void)
{
    //int i;
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOInput (GPIO_PORTA_BASE, GPIO_PIN_7);
    GPIOPadConfigSet (GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    sw_normal[SW_LEFT] = SW1_NORMAL;


    GPIOPinTypeGPIOInput (GPIO_PORTA_BASE, GPIO_PIN_6);
        GPIOPadConfigSet (GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA,
           GPIO_PIN_TYPE_STD_WPD);
    sw_normal[SW_RIGHT] = SW2_NORMAL;


}

void
updateSwitches (void)
{
    bool sw_value[NUM_SW];
    int i;

    sw_value[SW_LEFT] = (GPIOPinRead (SW1_PORT_BASE, SW1_PIN) == SW1_PIN);
    sw_value[SW_RIGHT] = (GPIOPinRead (SW2_PORT_BASE, SW2_PIN) == SW2_PIN);


    // Iterate through the buttons, updating button variables as required
    for (i = 0; i < NUM_SW; i++)
    {
        if (sw_value[i] != sw_state[i])
        {
            sw_count[i]++;
            if (sw_count[i] >= NUM_SW_POLLS)
            {
                sw_state[i] = sw_value[i];

                sw_count[i] = 0;
            }
        }
        else
            sw_count[i] = 0;
    }

    if (sw_value[SW_RIGHT]) {
        //Switch has been flicked
        g_testState = 1;
    } else {
        g_testState = 0;
    }
}

uint8_t
checkSwitch (uint8_t swName)
{
    if (sw_state[swName] == sw_normal[swName]) {
        return SW_DOWN;
    } else {
        return SW_UP;
    }
}

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.
void
updateButtons (void)
{
	bool but_value[NUM_BUTS];
	int i;
	
	// Read the pins; true means HIGH, false means LOW
	but_value[UP] = (GPIOPinRead (UP_BUT_PORT_BASE, UP_BUT_PIN) == UP_BUT_PIN);
	but_value[DOWN] = (GPIOPinRead (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN) == DOWN_BUT_PIN);
    but_value[LEFT] = (GPIOPinRead (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN) == LEFT_BUT_PIN);
    but_value[RIGHT] = (GPIOPinRead (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN) == RIGHT_BUT_PIN);
	// Iterate through the buttons, updating button variables as required
	for (i = 0; i < NUM_BUTS; i++)
	{
        if (but_value[i] != but_state[i])
        {
        	but_count[i]++;
        	if (but_count[i] >= NUM_BUT_POLLS)
        	{
        		but_state[i] = but_value[i];
        		but_flag[i] = true;	   // Reset by call to checkButton()
        		but_count[i] = 0;
        	}
        }
        else
        	but_count[i] = 0;
	}
}

// *******************************************************
// checkButton: Function returns the new button logical state if the button
// logical state (PUSHED or RELEASED) has changed since the last call,
// otherwise returns NO_CHANGE.
uint8_t
checkButton (uint8_t butName)
{
	if (but_flag[butName])
	{
		but_flag[butName] = false;
		if (but_state[butName] == but_normal[butName])
			return RELEASED;
		else
			return PUSHED;
	}
	return NO_CHANGE;
}

void upButtonIntHandler (void)
{
    GPIOIntDisable(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    if (g_testState == 0 && g_state != 6) {
        //Switch is off
        // Not on step goal screen
        //change units
        g_units += 1;
        if (g_units > 1) {
            g_units = 0;
        }
    } else if (g_testState == 1 && g_state != 6) {
        //Switch is on
        // Not on step goal screen
        //Incriment steps and distance
        g_totalSteps += 100;
        g_totalDistance += 90;
    }

    if (g_state != 6) {
        vector3_t accData = {0,0,0};
        updateDisplay(accData,0,0);
    }


    GPIOIntEnable(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    GPIOIntClear(UP_BUT_PORT_BASE, UP_BUT_PIN);
}

void downButtonIntHandler (void)
{
    GPIOIntDisable(UP_BUT_PORT_BASE, UP_BUT_PIN);
    //display acc

    if (g_testState == 0) {
        //TODO Set goal state
        g_stepGoal = g_displayedStepGoal;

    } else if (g_testState == 1) {
        //Test mode is active

        if (g_totalSteps < 500) {
            g_totalSteps = 0;
        } else {
            g_totalSteps -= 500;
        }

        if (g_totalDistance < 450) {
            g_totalDistance = 0;
        } else {
            g_totalDistance -= 450;
        }
    }
    vector3_t accData = getAcclData();
    updateDisplay(accData, 0,0);

    GPIOIntEnable(UP_BUT_PORT_BASE, UP_BUT_PIN);
    GPIOIntClear(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
}

void (sidewaysButtonHandler) (void)
{
    if (GPIOPinRead(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN)) {
        GPIOIntDisable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);

        vector3_t accData = getAcclData();

        if (g_state == 4) {
            g_state = 6;
        } else if (g_state == 5) {
            g_state = 8;
        } else if (g_state == 6) {
            g_state = 7;
        } else if (g_state == 7) {
            g_state = 8;
        } else if (g_state == 8) {
            g_state = 6;
        }

        updateDisplay(accData, 0, 0);

        SysCtlDelay (SysCtlClockGet () / 15);
        GPIOIntClear(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
        GPIOIntClear(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
        GPIOIntEnable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    } else if (GPIOPinRead(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN)) {
        GPIOIntDisable(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);

        vector3_t accData = getAcclData();

        if (g_state == 4) {
            g_state = 5;
        } else if (g_state == 5) {
            g_state = 6;
        } else if (g_state == 6) {
            g_state = 4;
        } else if (g_state == 7) {
            g_state = 6;
        } else if (g_state == 8) {
            g_state = 5;
        }

        updateDisplay(accData, 0, 0);

        SysCtlDelay (SysCtlClockGet () / 15);
        GPIOIntClear(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
        GPIOIntClear(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
        GPIOIntEnable(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    }
}

void initButtInt (void)
{
    GPIOIntRegister(UP_BUT_PORT_BASE, upButtonIntHandler);
    GPIOIntEnable(UP_BUT_PORT_BASE, UP_BUT_PIN);

    GPIOIntRegister(DOWN_BUT_PORT_BASE, downButtonIntHandler);
    GPIOIntEnable(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);

    GPIOIntRegister(RIGHT_BUT_PORT_BASE, sidewaysButtonHandler);
    GPIOIntEnable(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    GPIOIntEnable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
}


