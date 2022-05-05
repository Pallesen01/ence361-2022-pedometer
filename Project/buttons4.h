#ifndef BUTTONS_H_
#define BUTTONS_H_

// *******************************************************
// buttons4.h
//
// Support for a set of FOUR specific buttons on the Tiva/Orbit.
// ENCE361 sample code.
// The buttons are:  UP and DOWN (on the Orbit daughterboard) plus
// LEFT and RIGHT on the Tiva.
//
// P.J. Bones UCECE
// Last modified:  7.2.2018
// 
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "circBufT.h"

extern uint32_t g_state;
extern uint32_t g_units;
extern uint32_t g_testState;
extern uint32_t g_stepGoal;
extern uint32_t g_displayedStepGoal;
extern uint32_t g_potiVal;
extern circBuf_t g_poti_circ_buff;

extern uint32_t g_totalDistance;
extern uint32_t g_totalSteps;
extern uint32_t g_startUpSteps;
extern uint32_t g_startUpDistance;

//*****************************************************************************
// Constants
//*****************************************************************************
enum butNames {UP = 0, DOWN, LEFT, RIGHT, NUM_BUTS};
enum butStates {RELEASED = 0, PUSHED, NO_CHANGE};

enum swNames {SW_LEFT = 0, SW_RIGHT, NUM_SW};
enum swStates {SW_DOWN = 0, SW_UP};
// UP button
#define UP_BUT_PERIPH  SYSCTL_PERIPH_GPIOE
#define UP_BUT_PORT_BASE  GPIO_PORTE_BASE
#define UP_BUT_PIN  GPIO_PIN_0
#define UP_BUT_NORMAL  false
// DOWN button
#define DOWN_BUT_PERIPH  SYSCTL_PERIPH_GPIOD
#define DOWN_BUT_PORT_BASE  GPIO_PORTD_BASE
#define DOWN_BUT_PIN  GPIO_PIN_2
#define DOWN_BUT_NORMAL  false
// LEFT button
#define LEFT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define LEFT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define LEFT_BUT_PIN  GPIO_PIN_4
#define LEFT_BUT_NORMAL  true
// RIGHT button
#define RIGHT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define RIGHT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define RIGHT_BUT_PIN  GPIO_PIN_0
#define RIGHT_BUT_NORMAL  true

//Switch 1

#define SW1_PERIPH SYSCTL_PERIPH_GPIOA
#define SW1_PORT_BASE GPIO_PORTA_BASE
#define SW1_PIN GPIO_PIN_7
#define SW1_NORMAL false

//Switch 2

#define SW2_PERIPH SYSCTL_PERIPH_GPIOA
#define SW2_PORT_BASE GPIO_PORTA_BASE
#define SW2_PIN GPIO_PIN_6
#define SW2_NORMAL false

#define NUM_BUT_POLLS 3
#define NUM_SW_POLLS 1

void
initSwitches (void);

void
updateSwitches (void);

uint8_t
checkSwitch (uint8_t swName);

// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.

// *******************************************************
// initButtons: Initialise the variables associated with the set of buttons
// defined by the constants above.
void
initButtons (void);

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
void
updateButtons (void);

// *******************************************************
// checkButton: Function returns the new button state if the button state
// (PUSHED or RELEASED) has changed since the last call, otherwise returns
// NO_CHANGE.  The argument butName should be one of constants in the
// enumeration butStates, excluding 'NUM_BUTS'. Safe under interrupt.
uint8_t
checkButton (uint8_t butName);

void initButtInt (void);
void upButtonIntHandler (void);
void downButtonIntHandler (void);
void leftButtonHandler (void);
void rightButtonHandler (void);
void switchOneIntHandler (void);

#endif /*BUTTONS_H_*/
