/*
 * SvLib.h
 *
 *  Created on: Aug 24, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_GLOBAL_H_
#define SVLIB_INC_GLOBAL_H_

#include "main.h"

#define OFF 0U
#define ON 1U
#define DEISABLE 0U
#define ENABLE 1U


typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
} IOPin;

int32_t scaleValue(int32_t value, int32_t oldMin, int32_t oldMax, int32_t newMin, int32_t newMax);

// Function to set the kth bit of n
int32_t setBit(int32_t n, uint8_t k);

// Function to clear the kth bit of n
int32_t clearBit(int32_t n, uint8_t k);

// Function to toggle the kth bit of n
int32_t toggleBit(int32_t n, uint8_t k);

// Function to modify k-th bit with p
int32_t modifyBit(int32_t n, uint8_t k, uint8_t p);

// Function to find the kth bit of n
int32_t findBit(int32_t n, uint8_t k);

void loopValue(uint8_t* value, uint8_t min, uint8_t max);
void toggleValue(uint32_t* value);

#endif /* SVLIB_INC_GLOBAL_H_ */
