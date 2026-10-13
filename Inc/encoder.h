/*
 * pot.h
 *
 *  Created on: 10 lip 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_ENCODER_H_
#define SVLIB_INC_ENCODER_H_

#include "SvLib.h"
#include "mux.h"

typedef enum {
	ENC_TYPE_INCREMENTAL,
} ENC_Type;

typedef enum {
	ENC_MODE_STANDARD,
	ENC_MODE_PAN,
	ENC_MODE_WING,
} ENC_Mode;

typedef enum {
	ENC_EVENT_DECREMENT,
	ENC_EVENT_INCREMENT,
} ENC_Event;

typedef struct {

	 Mux_t* muxA;
	 Mux_t* muxB;
	 int32_t id;
	 ENC_Type  type;
	 ENC_Mode  mode;

	 volatile int lastRawValue;
	 volatile int currentRawValue;

	 volatile int value;
	 volatile int minValue;
	 volatile int maxValue;

	 volatile uint8_t lastReadOff;
	 volatile uint8_t currentReadOff;


	 void (*ENC_Increment_Callback)(void*);
	 _Bool ENC_Increment_Callback_Flag;

	 void (*ENC_Decrement_Callback)(void*);
	 _Bool ENC_Decrement_Callback_Flag;

} Encoder_t;


void Encoder_Init(Encoder_t* enc);
void Encoder_Mux_Init(Encoder_t* enc, Mux_t* muxA, Mux_t* muxB, int32_t id, ENC_Type type, ENC_Mode mode);
void Encoder_Service(Encoder_t* enc);
void Encoder_CallbackRegister(Encoder_t* enc, void (*CallbackPtr)(void*), ENC_Event event);
int Encoger_getValue(Encoder_t* enc);

#endif /* SVLIB_INC_ENCODER_H_ */
