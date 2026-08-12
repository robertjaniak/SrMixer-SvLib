/*
 * timer.c
 *
 *  Created on: 13 lip 2022
 *      Author: rober
 */


#include "timer.h"

void Timer_Init(void* ptr, Timer_t* timer, uint32_t delay, Timer_mode mode){

	timer->mode = mode;
	timer->state = TIMER_STATE_STOP;
	timer->start = 0;
	timer->time  = 0;
	timer->delay = delay;
	timer->count = 0;
	timer->elapsed = 0;

	timer->callbackPtr = ptr;

	timer->StartCallback = NULL;
	timer->StartCallbackFlag = 1;

	timer->StopCallback = NULL;
	timer->StopCallbackFlag = 1;

	timer->WorkingCallback = NULL;
	timer->WorkingCallbackFlag = 1;

	timer->ElapsedCallback = NULL;
	timer->ElapsedCallbackFlag = 1;

	timer->HalfTimeCallback = NULL;
	timer->HalfTimeCallbackFlag = 1;
}

uint32_t TimerService(Timer_t* timer){

	if (timer->state != TIMER_STATE_STOP){

		if (timer->state == TIMER_STATE_START){
			timer->state = TIMER_STATE_WORKING;
			if (timer->StartCallback != NULL){
				(*timer->StartCallback)(timer->callbackPtr);
				timer->StartCallbackFlag = 0;
			}
		}

		timer->time = HAL_GetTick() - timer->start;

		if (timer->WorkingCallback != NULL){
			(*timer->WorkingCallback)(timer->callbackPtr);
		}


		if (timer->delay != 0){

			if (timer->time >= timer->delay / 2){
				if (timer->HalfTimeCallbackFlag == 1){
					if (timer->HalfTimeCallback != NULL){
						timer->HalfTimeCallbackFlag = 0;
						(*timer->HalfTimeCallback)(timer->callbackPtr);
					}
				}
			}

			if (timer->time >= timer->delay){

				if (timer->ElapsedCallback != NULL){
					(*timer->ElapsedCallback)(timer->callbackPtr);
					timer->ElapsedCallbackFlag = 0;
				}

				timer->elapsed++;

				switch (timer->mode){

					case TIMER_MODE_CONTINOUS:
						TimerRestart(timer);
						break;

					case TIMER_MODE_ONE_SHOT:
						TimerStop(timer);
						break;

					default:
						break;
				}
			}
		}
	}

	return timer->time;
}

_Bool TimerIsEnabled(Timer_t* timer){
	return (timer->state != TIMER_STATE_STOP) ? 1 : 0;
}

void TimerStart(Timer_t* timer){
	timer->state = TIMER_STATE_START;
	timer->start = HAL_GetTick();
	timer->time = 0;
	timer->elapsed = 0;
	timer->StartCallbackFlag = 1;
	timer->StopCallbackFlag = 1;
	timer->ElapsedCallbackFlag = 1;
	timer->WorkingCallbackFlag = 1;
	timer->HalfTimeCallbackFlag = 1;
}

void TimerStop(Timer_t* timer){
	timer->state = TIMER_STATE_STOP;
	if (timer->StopCallback != NULL){
		(*timer->StopCallback)(timer->callbackPtr);
	}
	timer->StopCallbackFlag = 0;
}

void TimerRestart(Timer_t* timer){
	timer->time = 0;
	timer->start = HAL_GetTick();
}

void TimerReset(Timer_t* timer){
	timer->time = 0;
	timer->elapsed = 0;
}

uint32_t Timer_GetTime(Timer_t* timer){
	return timer->time;
}

uint32_t TimerElapsed(Timer_t* timer){ // 0 not elapsed, 1,2,3... elapsed counds
	return timer->elapsed;
}

void TimerRegisterCallback(Timer_t* timer, void (*CallbackPtr)(void*), Timer_event event){

	switch (event){

		case TIMER_EVENT_START:
			timer->StartCallback = CallbackPtr;
			timer->StartCallbackFlag =  1;
			break;

		case TIMER_EVENT_WORKING:
			timer->WorkingCallback = CallbackPtr;
			timer->WorkingCallbackFlag = 1;
			break;

		case TIMER_EVENT_STOP:
			timer->StopCallback = CallbackPtr;
			timer->StopCallbackFlag = 1;
			break;

		case TIMER_EVENT_ELAPSED:
			timer->ElapsedCallback = CallbackPtr;
			timer->ElapsedCallbackFlag = 1;
			break;

		case TIMER_EVENT_HALF_TIME:
			timer->HalfTimeCallback = CallbackPtr;
			timer->HalfTimeCallbackFlag = 1;
			break;

		default:
			break;
	}
}

