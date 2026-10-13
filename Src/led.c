/*
 * led.c
 *
 *  Created on: Jul 4, 2022
 *      Author: rober
 */

#include "led.h"

void LED_Init(LED_t* led, GPIO_TypeDef* port, uint16_t pin, LED_state state, LED_polarity polarity){

	led->port = port;
	led->pin = pin;
	led->polarity = polarity;

	led->blink = 0; // continous blinking
	led->blinkState = LED_BLINK_STOP;

	LED_SetState(led, state);

}

void LED_Reg_Init(LED_t* led, Reg_t *_reg, uint8_t _id, LED_state state, LED_polarity polarity){

	led->type = LED_TYPE_REG;
	led->reg = _reg;
	led->id = _id;
	led->polarity = polarity;

	LED_SetState(led, state);

}

void LED_Group_Init(LED_Group_t* led_group, LED_t* (*led_array)[], uint32_t size, int32_t index, LED_group_mode mode){

	if (size > MAX_LED_IN_GROUP) {
		size = MAX_LED_IN_GROUP;
	}

	for (int i = 0; i < size; i++) {
		led_group->active_array[i] = 0;
	}

	led_group->led_array_ptr = led_array;
	led_group->size = size;
	led_group->mode = mode;
	led_group->index = index;

	led_group->state = LED_GROUP_STATE_NEED_UPDATE;

}

void LED_Service(LED_t* led) {
	TimerService(&led->timer);
}

static void LedGroupClear(LED_Group_t* led_group) {

	for (int i = 0; i < led_group->size; i++){
		if ((*led_group->led_array_ptr)[i] != NULL) {
			LED_Off((*led_group->led_array_ptr)[i]);
		}
		led_group->active_array[i] = 0;
	}
}

static void LED_GroupUpdate(LED_Group_t* led_group) {

	LedGroupClear(led_group);

	switch (led_group->mode) {

		case LED_GROUP_MODE_SINGLE_ACTIVE:{

			if (led_group->index > led_group->size - 1) led_group->index = led_group->size - 1;
			if (led_group->index < 1) led_group->index = 1;

			if ((*led_group->led_array_ptr)[led_group->index] != NULL) {
				LED_On((*led_group->led_array_ptr)[led_group->index]);
			}

			led_group->active_array[led_group->index] = 1;
			break;
		}

		case LED_GROUP_MODE_MULTI_ACTIVE:
			break;

		case LED_GROUP_MODE_ENCODER_SINGLE_ACTIVE: {

			if (led_group->index > led_group->size - 1) led_group->index = led_group->size - 1;
			if (led_group->index < 1) led_group->index = 1;

			if ((*led_group->led_array_ptr)[led_group->index] != NULL) {
				LED_On((*led_group->led_array_ptr)[led_group->index]);
			}

			led_group->active_array[led_group->index] = 1;
			break;
		}

		case LED_GROUP_MODE_ENCODER_MULTI_ACTIVE:

			if (led_group->index > led_group->size - 1) led_group->index = led_group->size - 1;
			if (led_group->index < 1) led_group->index = 1;

			for (int i = 1; i <= led_group->index; i++) {
				if ((*led_group->led_array_ptr)[i] != NULL) {
					LED_On((*led_group->led_array_ptr)[i]);
				}
			}

			led_group->active_array[led_group->index] = 1;

			break;

		case LED_GROUP_MODE_ENCODER_PAN_SINGLE_ACTIVE:

			if (led_group->index < - (led_group->size / 2) + 1) led_group->index = - (led_group->size / 2) + 1;
			if (led_group->index >   (led_group->size / 2) - 1) led_group->index =   (led_group->size / 2) - 1;

			int16_t index = led_group->index + (led_group->size / 2);

			if ((*led_group->led_array_ptr)[index] != NULL) {
				LED_On((*led_group->led_array_ptr)[index]);
			}

			led_group->active_array[index] = 1;
			break;

		case LED_GROUP_MODE_ENCODER_PAN_MULTI_ACTIVE: {

			if (led_group->index < - (led_group->size / 2) + 1) led_group->index = - (led_group->size / 2) + 1;
			if (led_group->index >   (led_group->size / 2) - 1) led_group->index =   (led_group->size / 2) - 1;

			int16_t index = led_group->index + (led_group->size / 2);

			if (index > led_group->size / 2) {
				for (int i = led_group->size / 2; i <= index; i++) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
					}
				}
			}

			else if (index < led_group->size / 2) {
				for (int i = led_group->size / 2; i >= index; i--) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
					}
				}
			}

			else {
				if ((*led_group->led_array_ptr)[index] != NULL) {
					LED_On((*led_group->led_array_ptr)[index]);
				}
			}

			led_group->active_array[index] = 1;
			break;
		}

		case LED_GROUP_MODE_ENCODER_WING_SINGLE_ACTIVE: {

			if (led_group->index < - (led_group->size / 2) + 1) led_group->index = - (led_group->size / 2) + 1;
			if (led_group->index >   (led_group->size / 2) - 1) led_group->index =   (led_group->size / 2) - 1;

			int16_t index = index = (led_group->size / 2) + led_group->index;

			if ((*led_group->led_array_ptr)[index] != NULL) {
				LED_On((*led_group->led_array_ptr)[index]);
				led_group->active_array[index] = 1;
			}

			index = (led_group->size / 2) - led_group->index;

			if ((*led_group->led_array_ptr)[index] != NULL) {
				LED_On((*led_group->led_array_ptr)[index]);
				led_group->active_array[index] = 1;
			}

			break;
		}

		case LED_GROUP_MODE_ENCODER_WING_MULTI_ACTIVE: {

			if (led_group->index < - (led_group->size / 2) + 1) led_group->index = - (led_group->size / 2) + 1;
			if (led_group->index >   (led_group->size / 2) - 1) led_group->index =   (led_group->size / 2) - 1;

			int16_t index =  (led_group->size / 2) + led_group->index;

			if (index > led_group->size / 2) {

				for (int i = led_group->size / 2; i <= (led_group->size / 2) + led_group->index; i++) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
						led_group->active_array[index] = 1;
					}
				}

				for (int i = led_group->size / 2; i >= (led_group->size / 2) - led_group->index; i--) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
						led_group->active_array[index] = 1;
					}
				}
			}

			else if (index < led_group->size / 2) {

				for (int i = led_group->size / 2; i >= (led_group->size / 2) + led_group->index; i--) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
						led_group->active_array[index] = 1;
					}
				}

				for (int i = led_group->size / 2; i <= (led_group->size / 2) - led_group->index; i++) {
					if ((*led_group->led_array_ptr)[i] != NULL) {
						LED_On((*led_group->led_array_ptr)[i]);
						led_group->active_array[index] = 1;
					}
				}

			}

			else {
				if ((*led_group->led_array_ptr)[index] != NULL) {
					LED_On((*led_group->led_array_ptr)[index]);
					led_group->active_array[index] = 1;
				}
			}

			break;
		}

		default:
			break;
	}

	led_group->state = LED_GROUP_STATE_UPDATED;

}


void LED_GroupService(LED_Group_t* led_group){

	if (led_group->state == LED_GROUP_STATE_NEED_UPDATE) LED_GroupUpdate(led_group);

}


void LED_On(LED_t* led){

	led->state = LED_ON;

	switch (led->type) {

		case LED_TYPE_GPIO:

			switch (led->polarity){

				case LED_POLARITY_POSITIVE:
					HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
					break;

				case LED_POLARITY_NEGATIVE:
					HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
					break;
			}
			break;

		case LED_TYPE_REG:

			switch (led->polarity){

				case LED_POLARITY_POSITIVE:
					setRegBit(led->reg, led->id);
					break;

				case LED_POLARITY_NEGATIVE:
					resetRegBit(led->reg, led->id);
					break;
			}
			break;
	}
}

void LED_Off(LED_t* led){

	led->state = LED_OFF;

	switch (led->type) {

		case LED_TYPE_GPIO:

			switch (led->polarity){

				case LED_POLARITY_POSITIVE:
					HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
					break;

				case LED_POLARITY_NEGATIVE:
					HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
					break;
			}
			break;

		case LED_TYPE_REG:

			switch (led->polarity){

				case LED_POLARITY_POSITIVE:
					resetRegBit(led->reg, led->id - 1);
					break;

				case LED_POLARITY_NEGATIVE:
					setRegBit(led->reg, led->id - 1);
					break;
			}
			break;
	}
}

static void LedReadState(LED_t* led){

	switch (led->polarity){

		case LED_POLARITY_POSITIVE:

			if (HAL_GPIO_ReadPin(led->port, led->pin) == GPIO_PIN_SET){
				led->state = LED_ON;
			}
			else {
				led->state = LED_OFF;
			}
			break;

		case LED_POLARITY_NEGATIVE:

			if (HAL_GPIO_ReadPin(led->port, led->pin) == GPIO_PIN_RESET){
				led->state = LED_ON;
			}
			else {
				led->state = LED_OFF;
			}
			break;
	}
}

int32_t LED_Toggle(LED_t* led){

	switch (led->type) {

		case LED_TYPE_GPIO:

			 HAL_GPIO_TogglePin(led->port, led->pin);
			 LedReadState(led);
			 break;

		case LED_TYPE_REG:

			if (LED_GetState(led) > 0){
				LED_Off(led);
			}
			else {
				LED_On(led);
			}
			break;
		}

	return led->state;
}

_Bool LED_GetState(LED_t* led){
	return led->state;
}

void LED_SetState(LED_t* led, LED_state state){

	led->state = state;

	switch (led->state){

		case LED_OFF:
			LED_Off(led);
			break;

		case LED_ON:
			LED_On(led);
			break;

		default:
			break;
	}
}


static void LedBlinkCallback(void* ptr){

	LED_t* led = (LED_t*)ptr;

	if (led != NULL){

		LED_Toggle(led);

		if (led->blinkState == LED_BLINK_IN_PROGRESS){

			if (led->blink > 0){
				led->blink--;
				if (led->blink == 0){
					TimerStop(&led->timer);
					led->blinkState = LED_BLINK_STOP;
				}
			}
		}
	}
}

void LED_Blink_Start(LED_t* led, uint32_t delay, int32_t blinks){


	if (led->blinkState == LED_BLINK_STOP){

		Timer_Init(led, &led->timer, delay, TIMER_MODE_CONTINOUS);
		TimerRegisterCallback(&led->timer, LedBlinkCallback, TIMER_EVENT_ELAPSED);
		TimerStart(&led->timer);

		if (blinks > 0){
			led->blink = blinks * 2;
		}

		led->blinkState = LED_BLINK_IN_PROGRESS;
	}





}

void LED_Blink_Stop(LED_t* led){
	TimerStop(&led->timer);
}

int32_t LED_Group_Increase(LED_Group_t* led_group){

	led_group->index++;
	led_group->state = LED_GROUP_STATE_NEED_UPDATE;

	return led_group->index;
}

int32_t LED_Group_Decrease(LED_Group_t* led_group){

	led_group->index--;
	led_group->state = LED_GROUP_STATE_NEED_UPDATE;

	return led_group->index;
}

void LED_Group_SetActiveIndex(LED_Group_t* led_group, int32_t index){

	switch (led_group->mode) {

		case LED_GROUP_MODE_SINGLE_ACTIVE:
		case LED_GROUP_MODE_MULTI_ACTIVE:
		case LED_GROUP_MODE_ENCODER_SINGLE_ACTIVE:
		case LED_GROUP_MODE_ENCODER_MULTI_ACTIVE:

			if(index > led_group->size - 1) index = led_group->size - 1;
			if(index < 0) index = 0;
			break;


		case LED_GROUP_MODE_ENCODER_PAN_SINGLE_ACTIVE:
		case LED_GROUP_MODE_ENCODER_PAN_MULTI_ACTIVE:
		case LED_GROUP_MODE_ENCODER_WING_SINGLE_ACTIVE:
		case LED_GROUP_MODE_ENCODER_WING_MULTI_ACTIVE:

			if(index > (led_group->size / 2) - 1) index = (led_group->size / 2) - 1;
			if(index < - (led_group->size / 2) + 1) index = - (led_group->size / 2) + 1;
			break;

		default:
			break;
	}

	if (index != led_group->index){
		led_group->index = index;
		led_group->state = LED_GROUP_STATE_NEED_UPDATE;
	}
}

LED_t* LED_Group_GetActiveLed(LED_Group_t* led_group){
	return (*led_group->led_array_ptr)[led_group->index];
}

int32_t LED_Group_GetActiveIndex(LED_Group_t* led_group){
	return led_group->index;
}



