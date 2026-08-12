/*
 * pot.c
 *
 *  Created on: 10 lip 2022
 *      Author: rober
 */

#include "pot.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "timer.h"

void positionAverage(Pot_t* pot) {

	//pot->currentPosition = Filter_SMA(&pot->averageFilter, pot->adcBuffer[pot->id] >> 4);

	//int32_t position = Filter_Average(&pot->averageFilter, pot->adcBuffer[pot->id] >> 4);

	int32_t position = pot->adcBuffer[pot->id] >> 4;

	if (position > 0) {
		pot->currentPosition = position;
	}

}

char* Pot_getName(Pot_t* pot){
	return pot->name;
}

void Pot_setCurrentPosition(Pot_t* pot, int32_t position){
	pot->currentPosition = position;
}

int32_t Pot_getCurrentPosition(Pot_t* pot){
	return pot->currentPosition;
}

void Pot_setLastPosition(Pot_t* pot){
	pot->lastPosition = pot->currentPosition;
}

int32_t Pot_getLastPosition(Pot_t* pot){
	return pot->lastPosition;
}

static void potInPlace(Pot_t* pot){

	pot->positionState = POT_IN_PLACE;

	if (pot->InPlaceCallbackEnable == true){
		pot->InPlaceCallbackEnable = false;
		pot->NotInPlaceCallbackEnable = true;
		if (pot->InPlaceCallback != NULL){
			(*pot->InPlaceCallback)(pot);
		}
	}
}

static void potNotInPlace(Pot_t* pot){

	pot->positionState = POT_NOT_IN_PLACE;

	if (pot->NotInPlaceCallbackEnable == true){
		pot->NotInPlaceCallbackEnable = false;
		pot->InPlaceCallbackEnable = true;
		if (pot->NotInPlaceCallback != NULL){
			(*pot->NotInPlaceCallback)(pot);
		}
	}

}

void Pot_setDesiredPosition(Pot_t* pot, uint32_t position){

	pot->desiredPosition = position;
	pot->totalDistance = pot->desiredPosition - pot->currentPosition;

	if ((pot->totalDistance < pot->tolerance) && (pot->totalDistance > - pot->tolerance)) {
		potInPlace(pot);
	}
	else {
		potNotInPlace(pot);
	}

}

int32_t Pot_getTotalDistance(Pot_t* pot){
	return pot->totalDistance;
}

void Pot_setMaxValue(Pot_t* pot, uint32_t value){
	pot->maxValue = value;
}

void Pot_setMinValue(Pot_t* pot, uint32_t value){
	pot->minValue = value;
}

Pot_state Pot_getState(Pot_t* pot){
	return pot->state;
}

int32_t Pot_getSpeed(Pot_t* pot){
	return pot->speed;
}

static void speedDetect(Pot_t* pot){

	uint32_t dist = abs(pot->currentPosition - pot->tempPosition);
	uint32_t time = Timer_GetTime(&pot->speedTimer);

	uint32_t speed = (dist * 1000 / time);

	if (pot->speedBuffer.count >= POT_SPEED_ARRAY_SIZE){
		pot->speedBuffer.count = 0;
	}

	pot->speedBuffer.sum -= pot->speedBuffer.data[pot->speedBuffer.count];
	pot->speedBuffer.data[pot->speedBuffer.count] = speed;
	pot->speedBuffer.sum += speed;
	pot->speed = pot->speedBuffer.sum / POT_SPEED_ARRAY_SIZE;

	pot->speedBuffer.count++;

}

static void stateDetect(Pot_t* pot){

	pot->IdleCallbackFlag = true;
	pot->StopCallbackFlag = true;
	pot->TurnCallbackFlag = true;
	pot->TurnLeftCallbackFlag = true;
	pot->TurnRightCallbackFlag = true;

	uint32_t currentPosition = pot->currentPosition;

	int32_t distance = currentPosition - pot->lastPosition;

	pot->lastPosition = currentPosition;


	if ( distance > POT_TURN_TOLERANCE ){

		if (pot->TurnRightCallbackFlag == true){
			pot->TurnRightCallbackFlag = false;
			if(pot->TurnRightCallback != NULL){
				(*pot->TurnRightCallback)(pot);
			}
		}

		if (pot->TurnCallbackFlag == true){
			pot->TurnCallbackFlag = false;
			if(pot->TurnCallback != NULL){
				(*pot->TurnCallback)(pot);
			}
		}

		pot->state = POT_STATE_TURN_RIGHT;
	}

	else if ( distance < - POT_TURN_TOLERANCE ){

		if (pot->TurnLeftCallbackFlag == true){
			pot->TurnLeftCallbackFlag = false;
			if(pot->TurnLeftCallback != NULL){
				(*pot->TurnLeftCallback)(pot);
			}
		}

		if (pot->TurnCallbackFlag == true){
			pot->TurnCallbackFlag = false;
			if(pot->TurnCallback != NULL){
				(*pot->TurnCallback)(pot);
			}
		}

		pot->state = POT_STATE_TURN_LEFT;
	}

	else {

		if (pot->state != POT_STATE_IDLE){

			if (pot->StopCallbackFlag == true){
				pot->StopCallbackFlag = false;
				if (pot->StopCallback != NULL){
					(*pot->StopCallback)(pot);
				}
			}

			pot->state = POT_STATE_STOP;
		}
	}
}


static void PotIdleTimerStopCallback(void* handle){
	Pot_t* pot = (Pot_t*)handle;
	pot->state = POT_STATE_IDLE;

	if (pot->IdleCallbackFlag == true){
		pot->IdleCallbackFlag = false;
		if (pot->IdleCallback != NULL){
			(*pot->IdleCallback)(pot);
		}
	}

}

static void PotActiveTimerStopCallback(void* handle){
	Pot_t* pot = (Pot_t*)handle;
	stateDetect(pot);
}

static void PotSpeedDetectElapsedCallback(void* handle){
	Pot_t* pot = (Pot_t*)handle;
	speedDetect(pot);
}

void Pot_callbackRegister(Pot_t* pot, void (*CallbackPtr)(void*), Pot_event event){

	switch (event){

		case POT_EVENT_IDLE:
			pot->IdleCallback = CallbackPtr;
			pot->IdleCallbackFlag =  true;
			break;

		case POT_EVENT_STOP:
			pot->StopCallback = CallbackPtr;
			pot->StopCallbackFlag =  true;
			break;

		case POT_EVENT_TURN:
			pot->TurnCallback = CallbackPtr;
			pot->TurnCallbackFlag =  true;
			break;

		case POT_EVENT_TURN_LEFT:
			pot->TurnLeftCallback = CallbackPtr;
			pot->TurnLeftCallbackFlag =  true;
			break;

		case POT_EVENT_TURN_RIGHT:
			pot->TurnRightCallback = CallbackPtr;
			pot->TurnRightCallbackFlag =  true;
			break;

		case POT_EVENT_IN_PACE:
			pot->InPlaceCallback = CallbackPtr;
			pot->InPlaceCallbackEnable =  true;
			break;

		case POT_EVENT_NOT_IN_PLACE:
			pot->NotInPlaceCallback = CallbackPtr;
			pot->NotInPlaceCallbackEnable =  true;
			break;

		default:
			break;
	}
}

Pot_position_state Pot_getPositionState(Pot_t* pot) {
	return pot->positionState;
}

int32_t Pot_getMaxValue(Pot_t* pot){
	return pot->maxValue;
}

int32_t Pot_getMinValue(Pot_t* pot){
	return pot->minValue;
}

void Pot_disable(Pot_t* pot) {
	pot->state = POT_STATE_DISABLE;
	TimerStop(&pot->activeTimer);
	TimerStop(&pot->idleTimer);
	TimerStop(&pot->speedTimer);
}

void Pot_enable(Pot_t* pot) {
	pot->state = POT_STATE_IDLE;
	TimerStart(&pot->activeTimer);
	TimerStart(&pot->idleTimer);
	//TimerStart(&pot->speedTimer);
}

void Pot_Service(Pot_t* pot){

	if (pot->state != POT_STATE_DISABLE){

		TimerService(&pot->activeTimer);
		TimerService(&pot->idleTimer);
		//TimerService(&pot->speedTimer);

		switch (pot->state){

			case POT_STATE_IDLE:

				if ((pot->currentPosition < pot->desiredPosition + pot->tolerance) && (pot->currentPosition > pot->desiredPosition - pot->tolerance)){
					pot->positionState = POT_IN_PLACE;
				}
				else {
					pot->positionState = POT_NOT_IN_PLACE;
				}
				break;

			case POT_STATE_TURN_RIGHT:

				if(abs(pot->currentPosition - pot->lastPosition) > POT_IDLE_TOLERANCE){
					TimerStop(&pot->idleTimer);
				}

				potNotInPlace(pot);

				if ((pot->currentPosition > (pot->desiredPosition - pot->tolerance)) && (pot->currentPosition < (pot->desiredPosition + pot->tolerance))){
					potInPlace(pot);
				}
				break;

			case POT_STATE_STOP:

				if (TimerIsEnabled(&pot->idleTimer) == 0){
					TimerStart(&pot->idleTimer);
					pot->StopCallbackFlag = true;
				}
				break;

			case POT_STATE_TURN_LEFT:

				if(abs(pot->currentPosition - pot->lastPosition) > POT_IDLE_TOLERANCE){
					TimerStop(&pot->idleTimer);
				}

				potNotInPlace(pot);

				if ((pot->currentPosition > (pot->desiredPosition - pot->tolerance)) && (pot->currentPosition < (pot->desiredPosition + pot->tolerance))){
					potInPlace(pot);
				}

				break;

			default:
				break;
		}
	}
}

void Pot_Init(Pot_t* pot, const char* name, Pot_type type, uint32_t adcBuffer[], uint32_t id, Pot_Polarity polarity, int32_t tolerance, int32_t position){

	pot->name = name;

	pot->adcBuffer = adcBuffer;
	pot->id = id;
	pot->type = type;

	pot->state = POT_STATE_IDLE;
	pot->polarity = polarity;

	Filter_Init(&pot->averageFilter, pot->filterBuffer, FILTER_BUFFER_SIZE);

	pot->lastPosition = pot->currentPosition;
	pot->tempPosition = pot->currentPosition;
	pot->desiredPosition = position;
	pot->totalDistance = pot->desiredPosition - pot->currentPosition;

	if (pot->totalDistance == 0){
		potInPlace(pot);
	}
	else {
		potNotInPlace(pot);
	}

	pot->minValue = POT_DEFAULT_MIN_VALUE;
	pot->maxValue = POT_DEFAULT_MAX_VALUE;
	pot->tolerance = tolerance;

	pot->speedBuffer.count = 0;

	pot->saved = false;
	pot->toSend = false;
	pot->lastSentValue = -1;

	pot->IdleCallback = NULL;
	pot->IdleCallbackFlag = true;

	pot->StopCallback = NULL;
	pot->StopCallbackFlag = true;

	pot->TurnCallback = NULL;
	pot->TurnCallbackFlag = true;

	pot->TurnLeftCallback = NULL;
	pot->TurnLeftCallbackFlag = true;

	pot->TurnRightCallback = NULL;
	pot->TurnRightCallbackFlag = true;

	pot->InPlaceCallback = NULL;
	pot->InPlaceCallbackEnable = true;

	pot->NotInPlaceCallback = NULL;
	pot->NotInPlaceCallbackEnable = true;


	Timer_Init(pot, &pot->activeTimer, POT_ACTIVE_TIMER_TIME, TIMER_MODE_CONTINOUS);
	Timer_Init(pot, &pot->idleTimer, POT_IDLE_TIMER_TIME, TIMER_MODE_ONE_SHOT);
	Timer_Init(pot, &pot->speedTimer, POT_SPEED_DETECT_TIME, TIMER_MODE_CONTINOUS);

	TimerRegisterCallback(&pot->activeTimer, PotActiveTimerStopCallback, TIMER_EVENT_ELAPSED);
	TimerRegisterCallback(&pot->idleTimer, PotIdleTimerStopCallback, TIMER_EVENT_ELAPSED);
	TimerRegisterCallback(&pot->speedTimer, PotSpeedDetectElapsedCallback, TIMER_EVENT_ELAPSED);

	TimerStart(&pot->activeTimer);
	//TimerStart(&pot->speedTimer);

}

