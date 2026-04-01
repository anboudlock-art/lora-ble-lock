/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#define		MOTOR_STOP			0
#define		MOTOR_FORWARD		1
#define		MOTOR_REVERSE		2

extern uint8_t MotorState;

extern void MotorInit(void);
extern void MotorStartForward(void);
extern void MotorStartReverse(void);
extern void MotorStop(void);

#endif


