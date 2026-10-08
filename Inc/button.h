/*
 * button.h
 *
 *  Created on: Jul 3, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_BUTTON_H_
#define SVLIB_INC_BUTTON_H_

#include "SVlib.h"
#include "timer.h"
#include "mux.h"

typedef enum {
	BTN_POLARITY_POSITIVE = 0,
	BTN_POLARITY_NEGATIVE = 1,
} BTN_Polarity;

typedef enum {
	BTN_TYPE_GPIO = 0,
	BTN_TYPE_MUX  = 1,
} BTN_Type;

typedef enum {
	BTN_MODE_STANDARD     = 1,  // short press (fast) only
	BTN_MODE_EXTRA        = 2,  // short press (slow) + long press + double press
} BTN_Modes;

typedef enum {
	BTN_STATE_RELEASED = 0,
	BTN_STATE_PRESSED  = 1,
} BTN_States;

typedef enum {
	BTN_EVENT_IDLE         = 0,
	BTN_EVENT_PRESSED      = 1,
	BTN_EVENT_HOLD         = 2,
	BTN_EVENT_RELEASED     = 3,
	BTN_EVENT_RELEASE      = 4,
	BTN_EVENT_LONG_PRESS   = 5,
	BTN_EVENT_SHORT_PRESS  = 6,
	BTN_EVENT_DOUBLE_PRESS = 7,
} BTN_Events;

typedef struct {
	 char* name;
	 GPIO_TypeDef* port;
	 uint16_t pin;
	 Mux_t* mux;
	 int32_t mux_id;
	 BTN_Type type;
	 BTN_Polarity polarity;
	 BTN_Modes  mode;
	 BTN_States state;
	 BTN_Events event;

	 _Bool press_flag;
	 _Bool release_flag;

	 _Bool    debounce_in_progress;
	 int8_t   debounce_state;
	 Timer_t  debounce_timmer;

	 Timer_t long_press_timer;
	 Timer_t double_press_timer;

	 uint8_t  press_count;

	 void (*BTN_Event_Pressed_Callback)(void*);
	 _Bool BTN_Event_Pressed_Callback_Flag;

	 void (*BTN_Event_Hold_Callback)(void*);
	 _Bool BTN_Event_Hold_Callback_Flag;

	 void (*BTN_Event_Released_Callback)(void*);
	 _Bool BTN_Event_Released_Callback_Flag;

	 void (*BTN_Event_Release_Callback)(void*);
	 _Bool BTN_Event_Release_Callback_Flag;

	 void (*BTN_Event_ShortPress_Callback)(void*);
	 _Bool BTN_Event_ShortPress_Callback_Flag;

	 void (*BTN_Event_LongPress_Callback)(void*);
	 _Bool BTN_Event_LongPress_Callback_Flag;

	 void (*BTN_Event_DoublePress_Callback)(void*);
	 _Bool BTN_Event_DoublePress_Callback_Flag;

} Button_t;

void Button_GPIO_Init(Button_t* btn, GPIO_TypeDef* port, uint16_t pin, char* name, BTN_Modes mode);
void Button_Mux_Init(Button_t* btn, Mux_t* mux, int32_t id, char* name, BTN_Modes mode);
void Button_StartDebouncing(Button_t* btn);
void Button_Service(Button_t* btn);
void Button_CallbackRegister(Button_t* btn, void (*CallbackPtr)(void*), BTN_Events event);

//Button Mux for ToDo


#endif /* SVLIB_INC_BUTTON_H_ */
