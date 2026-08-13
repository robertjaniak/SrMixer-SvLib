/*
 * out.h
 *
 *  Created on: Jul 4, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_OUT_H_
#define SVLIB_INC_OUT_H_

#include "global.h"
#include "timer.h"
#include "reg.h"
#include "stdBool.h"

#define MAX_OUT_IN_GROUP 8

typedef enum {
	OUT_POLARITY_POSITIVE = 0,
	OUT_POLARITY_NEGATIVE = 1,
} Out_polarity;

typedef enum {
	OUT_TYPE_GPIO = 0,
	OUT_TYPE_REG = 1,
} Out_type;

typedef enum {
	OUT_OFF = 0,
	OUT_ON = 1,
} Out_state;

typedef struct {
	char* name;
	uint8_t id;
	Reg_t *reg;
	Out_type type;
	GPIO_TypeDef* port;
	uint16_t pin;
	Out_state state;
	Out_polarity polarity;
	Timer_t timer;

	 void (*OUT_Event_ChangeState_Callback)(void*);
	 _Bool OUT_Event_ChangeState_Callback_Flag;

} Out_t;

typedef enum {
	OUT_GROUP_MODE_SINGLE_ACTIVE = 0,
	OUT_GROUP_MODE_MULTI_ACTIVE  = 1,
} Out_Group_mode;

typedef enum {
	OUT_GROUP_STATE_UPDATED     = 0,
	OUT_GROUP_STATE_NEED_UPDATE = 1,
} Out_Group_state;

typedef struct {
	Out_t*  (*out_array_ptr)[];
	uint32_t size; // count of outs
	int32_t index;
	_Bool active_array[MAX_OUT_IN_GROUP]; // active out's array
	Out_Group_mode mode;
	Out_Group_state state;

	 void (*OUTGROUP_Event_ChangeState_Callback)(void*);
	 _Bool OUTGROUP_Event_ChangeState_Callback_Flag;

}Out_Group_t;

typedef enum {
	OUT_EVENT_IDLE         = 0,
	OUT_EVENT_CHANGE_STATE     = 1,
} Out_events;

typedef enum {
	OUT_GROUP_EVENT_IDLE         = 0,
	OUT_GROUP_EVENT_CHANGE_STATE     = 1,
} Out_Group_events;

void OUT_GPIO_Init(Out_t* out, GPIO_TypeDef* port, uint16_t GPIO_Pin, const char* name, Out_state state, Out_polarity polarity);
void OUT_Reg_Init(Out_t* out, Reg_t *reg, uint8_t id, const char* name, Out_state state,  Out_polarity polarity);
void OUT_Group_Init(Out_Group_t* out_group, Out_t* (*out_array)[], uint32_t size, int32_t idex, Out_Group_mode mode);
void OUT_Service(Out_t* out);
void OUT_GroupService(Out_Group_t* out_group);
void OUT_Mode(Out_t* out, Out_polarity polarity);
void OUT_On(Out_t* out);
void OUT_Off(Out_t* out);
int32_t OUT_Toggle(Out_t* out);
void OUT_SetState(Out_t* out, Out_state state, bool executeCallback);
Out_state OUT_GetState(Out_t* out);

void OUT_Blink_Start(Out_t* out, uint32_t delay);
void OUT_Blink_Stop(Out_t* out);

int32_t OUT_Group_Increase(Out_Group_t* out_group);
int32_t OUT_Group_Decrease(Out_Group_t* out_group);
void OUT_Group_SetActiveIndex(Out_Group_t* out_group, int32_t index);
int32_t OUT_Group_GetActiveIndex(Out_Group_t* out_group);
Out_t* OUT_Group_GetActiveOut(Out_Group_t* out_group);


void OUT_CallbackRegister(Out_t* out, void (*CallbackPtr)(void*), Out_events event);
void OUTGROUP_CallbackRegister(Out_Group_t* outGroup, void (*CallbackPtr)(void*), Out_Group_events event);

#endif /* SVLIB_INC_OUT_H_ */
