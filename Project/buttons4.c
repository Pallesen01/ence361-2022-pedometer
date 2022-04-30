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
    int i;
    SysCtlPeripheralEnable (SW1_PERIPH);
    GPIOPinTypeGPIOInput (SW1_PORT_BASE, SW1_PIN);
    GPIOPadConfigSet (SW1_PORT_BASE, SW1_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    sw_normal[SW_RIGHT] = SW1_NORMAL;

    SysCtlPeripheralEnable (SW2_PERIPH);
    GPIOPinTypeGPIOInput (SW2_PORT_BASE, SW2_PIN);
    GPIOPadConfigSet (SW2_PORT_BASE, SW2_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    sw_normal[SW_LEFT] = SW2_NORMAL;

    for (i = 0; i < NUM_SW; i++) {
        sw_state[i] = sw_normal[i];
        sw_count[i] = 0;
    }
}

void
updateSwitches (void)
{
    bool sw_value[NUM_SW];
    int i;

    sw_value[SW_LEFT] = (GPIOPinRead (SW2_PORT_BASE, SW2_PIN) == SW2_PIN);
    sw_value[SW_RIGHT] = (GPIOPinRead (SW1_PORT_BASE, SW1_PIN) == SW1_PIN);

    // Iterate through the buttons, updating button variables as required
    for (i = 0; i < NUM_SW; i++)
    {
        if (sw_value[i] != sw_state[i])
        {
            sw_count[i]++;
            if (sw_count[i] >= NUM_SW_POLLS)
            {
                sw_state[i] = sw_value[i];
                but_count[i] = 0;
            }
        }
        else
            sw_count[i] = 0;
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
    uint32_t totalSteps = 5789;
    uint32_t totalDistance = 3200;
    uint32_t startUpSteps = 1234;
    uint32_t startUpDistance = 1000;
    //change units
    g_units += 1;
    if (g_units > 1) {
        g_units = 0;
    }

    vector3_t accData = getAcclData();
    displayAcc(accData,0,0,startUpSteps,startUpDistance,totalSteps,totalDistance);
    //display accel
    GPIOIntClear(UP_BUT_PORT_BASE, UP_BUT_PIN);
}

void downButtonIntHandler (void)
{
    GPIOIntDisable(UP_BUT_PORT_BASE, UP_BUT_PIN);
    //display acc

    g_state = 3;
    vector3_t accData = getAcclData();
    int8_t pitch = setReferencePitch(accData);
    int8_t roll = setReferenceRoll(accData);
    displayAcc(accData, pitch, roll,0,0,0,0);

    GPIOIntClear(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
}

void (sidewaysButtonHandler) (void)
{
    if (GPIOPinRead(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN)) {
        GPIOIntDisable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
        uint32_t totalSteps = 5789;
        uint32_t totalDistance = 3200;

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

        displayAcc(accData, 0, 0,0,0,totalSteps,totalDistance);

        SysCtlDelay (SysCtlClockGet () / 15);
        GPIOIntClear(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
        GPIOIntClear(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
        GPIOIntEnable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    } else if (GPIOPinRead(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN)) {
        GPIOIntDisable(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
        uint32_t startUpSteps = 1234;
        uint32_t startUpDistance = 1000;
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

        displayAcc(accData, 0, 0,startUpSteps,startUpDistance,0,0);

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
