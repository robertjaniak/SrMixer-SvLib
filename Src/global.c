/*
 * global.c
 *
 *  Created on: 31 lip 2023
 *      Author: rober
 */

#include "global.h"


int32_t scaleValue(int32_t value, int32_t oldMin, int32_t oldMax, int32_t newMin, int32_t newMax) {

	 return (((value - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
      //return (value - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
}

int32_t setBit(int32_t n, uint8_t k) {
    return (n | (1 << (k - 1)));
}

int32_t clearBit(int32_t n, uint8_t k) {
    return (n & (~(1 << (k - 1))));
}

int32_t toggleBit(int32_t n, uint8_t k){
    return (n ^ (1 << (k - 1)));
}

int32_t modifyBit(int32_t n, uint8_t k, uint8_t p){
    return (n | (p << k));
}

int32_t findBit(int32_t n, uint8_t k){
    return ((n >> (k - 1)) & 1);
}

void loopValue(uint8_t* value, uint8_t min, uint8_t max){

	(*value)++;

	if ((*value) > max) (*value) = min;

}

void toggleValue(uint32_t* value){

	if ((*value) > 0) {
		(*value) = 0;
	}
	else {
		(*value) = 1;
	}

}
