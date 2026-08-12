/*
 * SvLib.h
 *
 *  Created on: Aug 24, 2022
 *      Author: rober
 */

#ifndef INC_SVLIB_H_
#define INC_SVLIB_H_

#include "main.h"
#include "sv_debug.h"

char msg[60];

typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
} IO_pin;


#endif /* INC_SVLIB_H_ */

