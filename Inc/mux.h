/*
 * mux.h
 *
 *  Created on: 17 sie 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_MUX_H_
#define SVLIB_INC_MUX_H_

#include "global.h"

typedef enum {
	MUX_MODE_STANDARD = 0,
	MUX_MODE_INPUT_ONLY = 1,
	MUX_MODE_OUTPUT_ONLY = 2,
} Mux_mode;

typedef enum {
	MUX_EVENT_CHANGE_STATE = 0,

} Mux_event;

typedef struct {

	Mux_mode mode;

	IOPin address_1;
	IOPin address_2;
	IOPin address_3;
	IOPin address_4;
	IOPin enable;
	IOPin out;

	int32_t id;
	uint8_t states[16];

	void (*ChangeStateCallback)(void*);
	_Bool ChangeStateCallbackFlag;

}Mux16_t;

void Mux16_Init(Mux16_t* muxm, Mux_mode mode);

uint8_t Mux16_read(Mux16_t* mux, int32_t adr);
uint8_t Mux_GetState(Mux16_t* mux, int32_t id);

void Mux16_Service(Mux16_t* mux);

void Mux_RegisterCallback(Mux16_t* mux, void (*CallbackPtr)(void*), Mux_event event);

#endif /* SVLIB_INC_MUX_H_ */
