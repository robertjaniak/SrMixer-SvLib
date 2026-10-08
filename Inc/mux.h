/*
 * mux.h
 *
 *  Created on: 17 sie 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_MUX_H_
#define SVLIB_INC_MUX_H_

#include "SVlib.h"

#define MUX_STATE_BUFER_SIZE 16

typedef enum {
	MUX_TYPE_8 = 8,
	MUX_TYPE_16 = 16,
} Mux_type;

typedef enum {
	MUX_MODE_STANDARD = 0,
	MUX_MODE_INPUT_ONLY = 1,
	MUX_MODE_OUTPUT_ONLY = 2,
} Mux_mode;

typedef enum {
	MUX_POLARITY_POSITIVE = 0,
	MUX_POLARITY_NEGATIVE = 1,
} Mux_polarity;

typedef enum {
	MUX_EVENT_CHANGE_STATE = 0,
} Mux_event;

typedef struct {

	uint32_t id;
	Mux_type type;
	Mux_mode mode;
	Mux_polarity polarity;

	IOPin address_1;
	IOPin address_2;
	IOPin address_3;
	IOPin address_4;
	IOPin enable;
	IOPin out;

	int32_t pinId;
	uint8_t states[MUX_STATE_BUFER_SIZE];

	void (*ChangeStateCallback)(void*);
	_Bool ChangeStateCallbackFlag;

}Mux_t;

void Mux_Init(Mux_t* muxm);
void Mux_Service(Mux_t* mux);

uint8_t Mux_read(Mux_t* mux, int32_t adr);
uint8_t Mux_GetState(Mux_t* mux, int32_t id);

void Mux_RegisterCallback(Mux_t* mux, void (*CallbackPtr)(void*), Mux_event event);

#endif /* SVLIB_INC_MUX_H_ */
