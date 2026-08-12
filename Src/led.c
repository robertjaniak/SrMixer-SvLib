/*
 * led.c
 *
 *  Created on: Jul 4, 2022
 *      Author: rober
 */

#include "main.h"
#include "led.h"


void LED_Init(LED_t* led, GPIO_TypeDef* port, uint16_t pin, LED_state state, LED_Polarity polarity){

	led->port = port;
	led->pin = pin;
	led->polarity = polarity;

	led->blink = 0; // continous blinking
	led->blinkState = LED_BLINK_STOP;

	LED_SetState(led, state);

}

void LED_GroupInit(LED_Group_t* led_group, LED_t* (*led_array)[], uint32_t size, int32_t index, LED_group_mode mode){

	if (size > MAX_LED_IN_GROUP){
		size = MAX_LED_IN_GROUP;
	}

	for (int i = 0; i < size; i++){
		led_group->active_array[i] = 0;
	}

	led_group->led_array_ptr = led_array;
	led_group->size = size;
	led_group->mode = mode;
	led_group->index = index;

	led_group->state = LED_GROUP_STATE_NEED_UPDATE;

}

void LED_Service(LED_t* led){
	TimerService(&led->timer);
}

static void LedGroupClear(LED_Group_t* led_group){

	for (int i = 0; i < led_group->size; i++){
		if ((*led_group->led_array_ptr)[i] != NULL){
			LED_Off((*led_group->led_array_ptr)[i]);
		}
		led_group->active_array[i] = 0;
	}
}

static void LED_GroupUpdate(LED_Group_t* led_group){

	switch (led_group->mode){

		case LED_GROUP_MODE_SINGLE_ACTIVE:

			LedGroupClear(led_group);

			if ((*led_group->led_array_ptr)[led_group->index] != NULL){
				LED_On((*led_group->led_array_ptr)[led_group->index]);
			}

			led_group->active_array[led_group->index] = 1;


			break;

		case LED_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;
	}

	led_group->state = LED_GROUP_STATE_UPDATED;

}


void LED_GroupService(LED_Group_t* led_group){

	if (led_group->state == LED_GROUP_STATE_NEED_UPDATE){
		LED_GroupUpdate(led_group);
	}

}


void LED_On(LED_t* led){

	led->state = LED_ON;

	switch (led->polarity){

	case LED_POLARITY_POSITIVE:
		HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
		break;

	case LED_POLARITY_NEGATIVE:
		HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
		break;

	}
}

void LED_Off(LED_t* led){

	led->state = LED_OFF;

	switch (led->polarity){

	case LED_POLARITY_POSITIVE:
		HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
		break;

	case LED_POLARITY_NEGATIVE:
		HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
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
	HAL_GPIO_TogglePin(led->port, led->pin);
	LedReadState(led);

	return led->state;

}

_Bool LED_GetState(LED_t* led){
	return led->state;
};

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

	switch (led_group->mode){

		case LED_GROUP_MODE_SINGLE_ACTIVE:

			if (led_group->index > led_group->size - 1){
					led_group->index = 0;
				}

			break;

		case LED_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;

	}

	return led_group->index;

}

int32_t LED_Group_Decrease(LED_Group_t* led_group){

	led_group->index--;
	led_group->state = LED_GROUP_STATE_NEED_UPDATE;

	switch (led_group->mode){

		case LED_GROUP_MODE_SINGLE_ACTIVE:
			if (led_group->index < 0){
				led_group->index = led_group->size - 1;
			}

			break;

		case LED_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;

	}

	return led_group->index;

}

void LED_Group_SetActiveIndex(LED_Group_t* led_group, int32_t index){

	if(index > led_group->size - 1){
		index = led_group->size - 1;
	}

	if(index < 0){
		index = 0;
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



