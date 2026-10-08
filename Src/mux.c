/*
 * mux.c
 *
 *  Created on: 17 sie 2022
 *      Author: rober
 */

#include "mux.h"

void Mux_Init(Mux_t* mux) {

	for (int i = 0; i < MUX_STATE_BUFER_SIZE; i++){
		mux->states[i] = mux->polarity;
	}

	mux->ChangeStateCallback = NULL;
	mux->ChangeStateCallbackFlag = 1;

}

static void selectMuxPort(Mux_t* mux, int32_t adr){

	HAL_GPIO_WritePin(mux->address_1.port, mux->address_1.pin, adr & (1 << 0));
	HAL_GPIO_WritePin(mux->address_2.port, mux->address_2.pin, adr & (1 << 1));
	HAL_GPIO_WritePin(mux->address_3.port, mux->address_3.pin, adr & (1 << 2));
	if (mux->type == MUX_TYPE_16) HAL_GPIO_WritePin(mux->address_4.port, mux->address_4.pin, adr & (1 << 3));

	if (mux->mode == MUX_MODE_STANDARD){
		HAL_GPIO_WritePin(mux->enable.port, mux->enable.pin, GPIO_PIN_RESET);
	}
}

static inline _Bool readMuxState(Mux_t* mux){
	return HAL_GPIO_ReadPin(mux->out.port, mux->out.pin);
}

static inline void deselectMuxPort(Mux_t* mux){
	HAL_GPIO_WritePin(mux->enable.port, mux->enable.pin, GPIO_PIN_SET);
}

uint8_t Mux_read(Mux_t* mux, int32_t adr){

	_Bool result;

	selectMuxPort(mux, adr);
	result = readMuxState(mux);
	if (mux->mode == MUX_MODE_STANDARD) deselectMuxPort(mux);

	return result;
}

uint8_t Mux_GetState(Mux_t* mux, int32_t id){

	return mux->states[id-1];
}

void Mux_Service(Mux_t* mux){

	_Bool state;

	for (int i = 0; i < mux->type; i++){

		mux->pinId = i;
		state = Mux_read(mux, mux->pinId);

		if (mux->states[mux->pinId] != state){
			if (mux->ChangeStateCallback != NULL){
				(*mux->ChangeStateCallback)(mux);
			}
		}
		mux->states[mux->pinId] = state;
	}
}

void Mux_RegisterCallback(Mux_t* mux, void (*CallbackPtr)(void*), Mux_event event){

	switch (event){

		case MUX_EVENT_CHANGE_STATE:
			mux->ChangeStateCallback = CallbackPtr;
			mux->ChangeStateCallbackFlag =  1;
			break;

		default:
			break;
	}
}

