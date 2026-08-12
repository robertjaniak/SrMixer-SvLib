/*
 * timer.h
 *
 *  Created on: 13 lip 2022
 *      Author: rober
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "global.h"

typedef enum {
	TIMER_EVENT_STOP       = 0,
	TIMER_EVENT_START      = 1,
	TIMER_EVENT_WORKING    = 2,
	TIMER_EVENT_ELAPSED    = 3,
	TIMER_EVENT_HALF_TIME  = 4,
}Timer_event;

typedef enum {
	TIMER_STATE_STOP     = 0,
	TIMER_STATE_START    = 1,
	TIMER_STATE_WORKING  = 2,
	TIMER_STATE_ELAPSED  = 3,
}Timer_state;

typedef enum {
	TIMER_MODE_CONTINOUS,
	TIMER_MODE_ONE_SHOT,
}Timer_mode;

typedef struct {
	Timer_mode mode;
	uint32_t start;
	uint32_t time;
	uint32_t delay;
	uint32_t count;
	uint32_t elapsed;
	Timer_state state;

	void* callbackPtr;

	void (*StartCallback)(void*);
	_Bool StartCallbackFlag;
	void (*StopCallback)(void*);
	_Bool StopCallbackFlag;
	void (*WorkingCallback)(void*);
	_Bool WorkingCallbackFlag; // we no need, working callback is continous
	void (*ElapsedCallback)(void*);
	_Bool ElapsedCallbackFlag;
	void (*HalfTimeCallback)(void*);
	_Bool HalfTimeCallbackFlag;


}Timer_t;

void Timer_Init(void* CallbackPtr, Timer_t* timer, uint32_t delay, Timer_mode mode);
uint32_t TimerService(Timer_t* timer);

_Bool TimerIsEnabled(Timer_t* timer);
void TimerStart(Timer_t* timer);
void TimerStop(Timer_t* timer);
void TimerRestart(Timer_t* timer);
void TimerReset(Timer_t* timer);

uint32_t Timer_GetTime(Timer_t* timer);
uint32_t TimerElapsed(Timer_t* timer);

void TimerRegisterCallback(Timer_t* timer, void (*CallbackPtr)(void*), Timer_event event);

#endif /* INC_TIMER_H_ */
