/*
 * led.h
 *
 *  Created on: Jul 4, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_LED_H_
#define SVLIB_INC_LED_H_

#include "SvLib.h"
#include "timer.h"

#define MAX_LED_IN_GROUP 8

typedef enum {
	LED_POLARITY_POSITIVE = 0,
	LED_POLARITY_NEGATIVE = 1,
} LED_Polarity;

typedef enum {
	LED_OFF = 0,
	LED_ON = 1,
} LED_state;

typedef enum {
	LED_BLINK_STOP = 0,
	LED_BLINK_IN_PROGRESS = 2,
} LED_Blink_state;

typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	LED_state state;
	LED_Polarity polarity;
	int32_t blink;
	LED_Blink_state blinkState;
	Timer_t timer;
}LED_t;

typedef enum {
	LED_GROUP_MODE_SINGLE_ACTIVE = 0,
	LED_GROUP_MODE_MULTI_ACTIVE  = 1,
} LED_group_mode;

typedef enum {
	LED_GROUP_STATE_UPDATED     = 0,
	LED_GROUP_STATE_NEED_UPDATE = 1,
} LED_group_state;

typedef struct {
	LED_t*  (*led_array_ptr)[];
	uint32_t size; // count of leds
	int32_t index;
	_Bool active_array[MAX_LED_IN_GROUP]; // active led's array
	LED_group_mode mode;
	LED_group_state state;
}LED_Group_t;

void LED_Init(LED_t* led, GPIO_TypeDef* port, uint16_t GPIO_Pin, LED_state state, LED_Polarity polarity);
void LED_GroupInit(LED_Group_t* led_group, LED_t* (*led_array)[], uint32_t size, int32_t idex, LED_group_mode mode);
void LED_Service(LED_t* led);
void LED_GroupService(LED_Group_t* led_group);
void LED_On(LED_t* led);
void LED_Off(LED_t* led);
int32_t LED_Toggle(LED_t* led);
void LED_SetState(LED_t* out, LED_state state);
_Bool LED_GetState(LED_t* led);

void LED_Blink_Start(LED_t* led, uint32_t delay, int32_t blinks);
void LED_Blink_Stop(LED_t* led);

int32_t LED_Group_Increase(LED_Group_t* led_group);
int32_t LED_Group_Decrease(LED_Group_t* led_group);
void LED_Group_SetActiveIndex(LED_Group_t* led_group, int32_t index);
int32_t LED_Group_GetActiveIndex(LED_Group_t* led_group);
LED_t* LED_Group_GetActiveLed(LED_Group_t* led_group);

#endif /* SVLIB_INC_LED_H_ */
