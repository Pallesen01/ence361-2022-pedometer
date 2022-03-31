/*
 * readRollPitch.h
 *
 *  Created on: 31/03/2022
 *      Author: Daniel Pallesen & Ben Stewart
 */

#ifndef READROLLPITCH_H_
#define READROLLPITCH_H_

// Constants
#define PI 3.1415
#define RAD_TO_DEG 57.3

// Functions

int8_t getSign (int32_t x);

int8_t calcPitch(vector3_t acceleration, int8_t relative_pitch);

int8_t calcRoll(vector3_t acceleration, int8_t relative_roll);

int8_t setReferencePitch(vector3_t acceleration);

int8_t setReferenceRoll(vector3_t acceleration);

#endif /* READROLLPITCH_H_ */
