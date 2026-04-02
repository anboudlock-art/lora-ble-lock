/**
 * Copyright (c) 2020, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

extern uint8_t DoorState;

void DoorStateInit(void);
uint8_t GetDoorState(void);
uint8_t GetInputStopF(void);
uint8_t GetInputStopR(void);

void hall_check(void);
#endif

