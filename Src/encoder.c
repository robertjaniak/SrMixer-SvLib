/*
 * encoder.c
 *
 *  Created on: 10 lip 2022
 *      Author: rober
 */

#include "encoder.h"

int8_t encoderLUT[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};


void Encoder_Init(Encoder_t* enc) {

	enc->lastRawValue = 0;
	enc->currentRawValue = 0;

	enc->value = 0;

	switch (enc->mode) {

		case ENC_MODE_STANDARD:
			 ENC_MODE_WING:

			enc->minValue = 0;
			enc->maxValue = 255;
			break;

		case ENC_MODE_PAN:
			enc->minValue = -127;
			enc->maxValue = 127;
			break;

		default:
			break;
	}

	enc->lastReadOff = 0;
	enc->currentReadOff = 0;

	enc->ENC_Increment_Callback = NULL;
	enc->ENC_Increment_Callback_Flag= 1;

	enc->ENC_Decrement_Callback = NULL;
	enc->ENC_Decrement_Callback_Flag = 1;
}


void Encoder_Mux_Init(Encoder_t* enc, Mux_t* muxA, Mux_t* muxB, int32_t id, ENC_Type type, ENC_Mode mode){

	enc->type = type;
	enc->mode = mode;
	enc->muxA = muxA;
	enc->muxB = muxB;
	enc->id = id;

	Encoder_Init(enc);
}

void Encoder_Service(Encoder_t* enc) {

	uint8_t	A = Mux_read(enc->muxA, enc->id);
	uint8_t	B = Mux_read(enc->muxB, enc->id);

	enc->currentReadOff = ((A & 1) << 1) | (B & 1);
	uint8_t index = (enc->lastReadOff << 2) | enc->currentReadOff;
	enc->currentRawValue += encoderLUT[index];
	enc->lastReadOff = enc->currentReadOff;

	if (enc->currentRawValue != enc->lastRawValue) {



		if (enc->currentRawValue > enc->lastRawValue) {

			enc->value++;
			if (enc->value > enc->maxValue) enc->value = enc->maxValue;

			if (enc->ENC_Increment_Callback_Flag == 1){
				if (enc->ENC_Increment_Callback != NULL){
					(*enc->ENC_Increment_Callback)(enc);
				}
			}
		}

		if (enc->currentRawValue < enc->lastRawValue) {

			enc->value--;
			if (enc->value < enc->minValue) enc->value = enc->minValue;

			if (enc->ENC_Decrement_Callback_Flag == 1){
				if (enc->ENC_Decrement_Callback != NULL){
					(*enc->ENC_Decrement_Callback)(enc);
				}
			}
		}

		enc->lastRawValue = enc->currentRawValue;
	}
}

void Encoder_CallbackRegister(Encoder_t* enc, void (*CallbackPtr)(void*), ENC_Event event) {

	switch (event){

	case ENC_EVENT_INCREMENT:
		enc->ENC_Increment_Callback = CallbackPtr;
		enc->ENC_Increment_Callback_Flag = 1;
		break;

	case ENC_EVENT_DECREMENT:
		enc->ENC_Decrement_Callback = CallbackPtr;
		enc->ENC_Decrement_Callback_Flag = 1;
		break;

	default:
		break;
	}
}

int Encoger_getValue(Encoder_t* enc) {
	return enc->value;
}
