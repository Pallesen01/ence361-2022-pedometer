/*
 * adcInterrupts.h
 *
 *  Created on: 5 May 2022
 *      Author: Daniel Pallesen
 */

#ifndef ADCINTERRUPTS_H_
#define ADCINTERRUPTS_H_

void
SysTickIntHandler(void);

void
ADCIntHandler(void);

void
initClock (void);

void
initADC (void);

void
initSysTick (void);

void
initDisplay (void);



#endif /* ADCINTERRUPTS_H_ */
