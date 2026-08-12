/*
 * mux.c
 *
 *  Created on: 17 sie 2022
 *      Author: rober
 */

#include "mux.h"


void Mux16_Init(Mux16_t* mux, Mux_mode mode) {

	mux->mode = mode;

	for (int i = 0; i < 16; i++){
		mux->states[i] = 1;
	}

	mux->ChangeStateCallback = NULL;
	mux->ChangeStateCallbackFlag = 1;

}


static void selectMuxPort(Mux16_t* mux, int32_t adr){

	HAL_GPIO_WritePin(mux->address_1.port, mux->address_1.pin, adr & (1 << 0));
	HAL_GPIO_WritePin(mux->address_2.port, mux->address_2.pin, adr & (1 << 1));
	HAL_GPIO_WritePin(mux->address_3.port, mux->address_3.pin, adr & (1 << 2));
	HAL_GPIO_WritePin(mux->address_4.port, mux->address_4.pin, adr & (1 << 3));

	if (mux->mode == MUX_MODE_STANDARD){
		HAL_GPIO_WritePin(mux->enable.port, mux->enable.pin, GPIO_PIN_RESET);
	}
}

static inline _Bool readMuxState(Mux16_t* mux){
	return HAL_GPIO_ReadPin(mux->out.port, mux->out.pin);
}

static inline void deselectMuxPort(Mux16_t* mux){
	if (mux->mode == MUX_MODE_STANDARD){
		HAL_GPIO_WritePin(mux->enable.port, mux->enable.pin, GPIO_PIN_SET);
	}
}


uint8_t Mux16_read(Mux16_t* mux, int32_t adr){

	_Bool result;

	selectMuxPort(mux, adr);
	result = readMuxState(mux);
	deselectMuxPort(mux);

	return result;
}

uint8_t Mux_GetState(Mux16_t* mux, int32_t id){

	return mux->states[id-1];
}


void Mux16_Service(Mux16_t* mux){

	_Bool state;

	for (int i = 0; i < 16; i++){

		mux->id = i;
		state = Mux16_read(mux, mux->id);

		if (mux->states[mux->id] != state){
			if (mux->ChangeStateCallback != NULL){
				(*mux->ChangeStateCallback)(mux);
			}
		}
		mux->states[mux->id] = state;
	}
}

void Mux_RegisterCallback(Mux16_t* mux, void (*CallbackPtr)(void*), Mux_event event){

	switch (event){

		case MUX_EVENT_CHANGE_STATE:
			mux->ChangeStateCallback = CallbackPtr;
			mux->ChangeStateCallbackFlag =  1;
			break;

		default:
			break;
	}
}

