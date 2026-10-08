/*
 * button.c
 *
 *  Created on: Jul 3, 2022
 *      Author: rober
 */


#include "button.h"
#include "stdio.h"

#define DEBOUCE_TIME 20
#define DOUBLE_PRESS_TIME 200
#define LONG_PRESS_TIME 500

void static Button_Init(Button_t* btn){

	btn->polarity = BTN_POLARITY_POSITIVE;
	btn->state = BTN_STATE_RELEASED;
	btn->event = BTN_EVENT_IDLE;

	btn->debounce_in_progress = 0;
	btn->debounce_state = -1;

	btn->press_flag = 1;
	btn->release_flag = 0;

	btn->press_count = 0;

	Timer_Init(&btn, &btn->debounce_timmer, DEBOUCE_TIME, TIMER_MODE_ONE_SHOT);
	Timer_Init(&btn, &btn->long_press_timer, LONG_PRESS_TIME, TIMER_MODE_ONE_SHOT);
	Timer_Init(&btn, &btn->double_press_timer, DOUBLE_PRESS_TIME, TIMER_MODE_ONE_SHOT);

	btn->BTN_Event_Pressed_Callback = NULL;
	btn->BTN_Event_Pressed_Callback_Flag = 1;

	btn->BTN_Event_Hold_Callback = NULL;
	btn->BTN_Event_Hold_Callback_Flag = 1;

	btn->BTN_Event_Released_Callback = NULL;
	btn->BTN_Event_Released_Callback_Flag = 0;

	btn->BTN_Event_Release_Callback = NULL;
	btn->BTN_Event_Release_Callback_Flag = 1;

	btn->BTN_Event_ShortPress_Callback = NULL;
	btn->BTN_Event_ShortPress_Callback_Flag = 1;

	btn->BTN_Event_LongPress_Callback = NULL;
	btn->BTN_Event_LongPress_Callback_Flag = 1;

	btn->BTN_Event_DoublePress_Callback = NULL;
	btn->BTN_Event_DoublePress_Callback_Flag = 1;
}

void Button_GPIO_Init(Button_t* btn, GPIO_TypeDef* port, uint16_t pin, char* name, BTN_Modes mode){

		btn->type = BTN_TYPE_GPIO;

		btn->port = port;
		btn->pin = pin;
		btn->name = name;
		btn->mode = mode;
		btn->mux = NULL;
		btn->mux_id = 0;


		Button_Init(btn);
}

void Button_Mux_Init(Button_t* btn, Mux_t* mux, int32_t id, char* name, BTN_Modes mode){

		btn->type = BTN_TYPE_MUX;

		btn->port = mux->out.port;
		btn->pin = mux->out.pin;
		btn->name = name;
		btn->mux = mux;
		btn->mux_id = id;
		btn->mode = mode;

		Button_Init(btn);
}

void Button_CallbackRegister(Button_t* btn, void (*CallbackPtr)(void*), BTN_Events event){

	switch (event){

	case BTN_EVENT_PRESSED:
		btn->BTN_Event_Pressed_Callback = CallbackPtr;
		btn->BTN_Event_Pressed_Callback_Flag = 1;
		break;

	case BTN_EVENT_HOLD:
		btn->BTN_Event_Hold_Callback = CallbackPtr;
		btn->BTN_Event_Hold_Callback_Flag = 1;
		break;

	case BTN_EVENT_RELEASED:
		btn->BTN_Event_Released_Callback = CallbackPtr;
		btn->BTN_Event_Released_Callback_Flag = 0;
		break;

	case BTN_EVENT_RELEASE:
		btn->BTN_Event_Release_Callback = CallbackPtr;
		btn->BTN_Event_Release_Callback_Flag = 1;
		break;

	case BTN_EVENT_SHORT_PRESS:
		btn->BTN_Event_ShortPress_Callback = CallbackPtr;
		btn->BTN_Event_ShortPress_Callback_Flag = 1;
		break;

	case BTN_EVENT_LONG_PRESS:
		btn->BTN_Event_LongPress_Callback = CallbackPtr;
		btn->BTN_Event_LongPress_Callback_Flag = 1;
		break;

	case BTN_EVENT_DOUBLE_PRESS:
		btn->BTN_Event_DoublePress_Callback = CallbackPtr;
		btn->BTN_Event_DoublePress_Callback_Flag = 1;
		break;

	default:
		break;
	}
}

void Button_StartDebouncing(Button_t* btn){
	//if (btn->debounce_in_progress == 0){
		TimerStart(&btn->debounce_timmer);
		btn->debounce_in_progress = 1;
		btn->debounce_state = HAL_GPIO_ReadPin(btn->port, btn->pin);
	//}
}


static void Button_StateDetect(Button_t* btn){

	TimerService(&btn->debounce_timmer);

	if(btn->debounce_in_progress == 1){

		if (TimerElapsed(&btn->debounce_timmer) == 1){
			TimerReset(&btn->debounce_timmer);
			btn->debounce_in_progress = 0;

			switch (btn->type){

				case BTN_TYPE_GPIO:
					if (HAL_GPIO_ReadPin(btn->port, btn->pin) == btn->debounce_state){
						(btn->debounce_state == GPIO_PIN_RESET)	 ? (btn->state = BTN_STATE_PRESSED) : (btn->state = BTN_STATE_RELEASED);
						btn->debounce_state = -1;
					}
					break;

				case BTN_TYPE_MUX:
					if (Mux_GetState(btn->mux, btn->mux_id) == btn->debounce_state){
						(btn->debounce_state == GPIO_PIN_RESET)	 ? (btn->state = BTN_STATE_PRESSED) : (btn->state = BTN_STATE_RELEASED);
						btn->debounce_state = -1;
					}
					break;

				default:
					break;

			}

		}
	}
}

static void PressedService(Button_t* btn){

	if (btn->press_flag == 1){
		btn->press_flag = 0;
		btn->release_flag = 1;

		TimerStart(&btn->long_press_timer);
		TimerStart(&btn->double_press_timer);

		btn->BTN_Event_Pressed_Callback_Flag = 1;
		btn->BTN_Event_Released_Callback_Flag = 1;
		btn->BTN_Event_ShortPress_Callback_Flag = 1;
		btn->BTN_Event_DoublePress_Callback_Flag = 1;
		btn->BTN_Event_LongPress_Callback_Flag = 1;

		btn->press_count++;
	}

	//BTN_EVENT_PRESSED, one shoot
	if (btn->BTN_Event_Pressed_Callback_Flag == 1){
		btn->BTN_Event_Pressed_Callback_Flag = 0;
		if (btn->BTN_Event_Pressed_Callback != NULL){
			(*btn->BTN_Event_Pressed_Callback)(btn);
		}
	}

	//BTN_EVENT_HOLD, continous
	if (btn->BTN_Event_Hold_Callback_Flag == 1){
		if (btn->BTN_Event_Hold_Callback != NULL){
			(*btn->BTN_Event_Hold_Callback)(btn);
		}
	}

	switch (btn->mode){

		case BTN_MODE_STANDARD:

			btn->event = BTN_EVENT_SHORT_PRESS;

			//BTN_EVENT_SHORT_PRESS - immediately in standard mode,
			if (btn->BTN_Event_ShortPress_Callback_Flag == 1){
				if (btn->BTN_Event_ShortPress_Callback != NULL){
					(*btn->BTN_Event_ShortPress_Callback)(btn);
				}
				btn->BTN_Event_ShortPress_Callback_Flag = 0;
			}

			break;

		case BTN_MODE_EXTRA:

			if (btn->press_count >= 2) {

				btn->event = BTN_EVENT_DOUBLE_PRESS;

				btn->press_count = 0;

				TimerStop(&btn->long_press_timer);
				TimerStop(&btn->double_press_timer);

				//BTN_DOUBLE_PRESS - in extra mode
				if (btn->BTN_Event_DoublePress_Callback_Flag == 1){
					if (btn->BTN_Event_DoublePress_Callback != NULL){
						(*btn->BTN_Event_DoublePress_Callback)(btn);
					}
					btn->BTN_Event_DoublePress_Callback_Flag = 0;
				}
			}

			if (TimerElapsed(&btn->long_press_timer)){
				TimerReset(&btn->long_press_timer);
				TimerStop(&btn->double_press_timer);
				TimerReset(&btn->double_press_timer);

				btn->event = BTN_EVENT_LONG_PRESS;

				btn->press_count = 0;

				//BTN_LONG_PRESS - in extra mode
				if (btn->BTN_Event_LongPress_Callback_Flag == 1){
					if (btn->BTN_Event_LongPress_Callback != NULL){
						(*btn->BTN_Event_LongPress_Callback)(btn);
					}
					btn->BTN_Event_LongPress_Callback_Flag = 0;
				}
			}

			break;
	}

}

static void ReleasedService(Button_t* btn){

	if (btn->release_flag == 1){
		btn->release_flag = 0;
		btn->press_flag = 1;
		TimerStop(&btn->long_press_timer);
	}

	//btn->event = BTN_EVENT_RELEASED, one shoot
	if (btn->BTN_Event_Released_Callback_Flag == 1){
		btn->BTN_Event_Released_Callback_Flag = 0;

		if (btn->BTN_Event_Released_Callback != NULL){
			(*btn->BTN_Event_Released_Callback)(btn);
		}
	}

	//btn->event = BTN_EVENT_RELEASE, continous
	if (btn->BTN_Event_Release_Callback_Flag == 1){
		if (btn->BTN_Event_Release_Callback != NULL){
			(*btn->BTN_Event_Release_Callback)(btn);
		}
	}


	switch (btn->mode){

		case BTN_MODE_STANDARD:
			if (btn->BTN_Event_ShortPress_Callback_Flag == 0){
				btn->BTN_Event_ShortPress_Callback_Flag = 1;
			}
			break;

		case BTN_MODE_EXTRA:
			if (TimerElapsed(&btn->double_press_timer)) {
				TimerReset(&btn->double_press_timer);
				TimerStop(&btn->long_press_timer);

				btn->event = BTN_EVENT_SHORT_PRESS;

				btn->press_count = 0;

				//btn->event = BTN_EVENT_SHORT_PRESS in Extra mode
				if (btn->BTN_Event_ShortPress_Callback_Flag == 1){
					if (btn->BTN_Event_ShortPress_Callback != NULL){
						(*btn->BTN_Event_ShortPress_Callback)(btn);
					}
					btn->BTN_Event_ShortPress_Callback_Flag = 0;
				}
			}
			break;

		default:
			break;

	}
}


static void Button_EventDetect(Button_t* btn){

	TimerService(&btn->long_press_timer);
	TimerService(&btn->double_press_timer);

	switch (btn->state) {

		case BTN_STATE_PRESSED:
			PressedService(btn);
			break;

		case BTN_STATE_RELEASED:
			ReleasedService(btn);
			break;

		default:
			break;
	}
}

void Button_Service(Button_t* btn){
	Button_StateDetect(btn);
	Button_EventDetect(btn);
}
