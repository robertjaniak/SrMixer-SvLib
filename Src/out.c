/*
 * out.c
 *
 *  Created on: Jul 4, 2022
 *      Author: rober
 */


#include "out.h"


void OUT_GPIO_Init(Out_t* out, GPIO_TypeDef* port, uint16_t pin, const char* name, Out_state state, Out_polarity polarity){

	out->type = OUT_TYPE_GPIO;
	out->port = port;
	out->pin = pin;
	out->polarity = polarity;

	out->name = name;

	out->OUT_Event_ChangeState_Callback = NULL;
	out->OUT_Event_ChangeState_Callback_Flag = 0;

	OUT_SetState(out, state, false);

}

void OUT_Reg_Init(Out_t* out, Reg_t *_reg, uint8_t _id, const char* name,  Out_state state, Out_polarity polarity){

	out->type = OUT_TYPE_REG;
	out->reg = _reg;
	out->id = _id;
	out->polarity = polarity;

	out->name = name;

	out->OUT_Event_ChangeState_Callback = NULL;
	out->OUT_Event_ChangeState_Callback_Flag = 0;

	OUT_SetState(out, state, false);

}

void OUT_Group_Init(Out_Group_t* out_group, Out_t* (*out_array)[], uint32_t size, int32_t index, Out_Group_mode mode){

	if (size > MAX_OUT_IN_GROUP){
		size = MAX_OUT_IN_GROUP;
	}

	for (int i = 0; i < size; i++){
		out_group->active_array[i] = 0;
	}

	out_group->out_array_ptr = out_array;
	out_group->size = size;
	out_group->mode = mode;
	out_group->index = index;

	out_group->state = OUT_GROUP_STATE_NEED_UPDATE;

	 void (*OUTGROP_Event_ChangeState_Callback)(void*) = NULL;
	 bool OUTGROP_Event_ChangeState_Callback_Flag = 0;

}

void OUT_Service(Out_t* out){
	TimerService(&out->timer);
}

static void OutGroupClear(Out_Group_t* out_group){

	for (int i = 0; i < out_group->size; i++){
		if ((*out_group->out_array_ptr)[i] != NULL){
			OUT_Off((*out_group->out_array_ptr)[i]);
		}
		out_group->active_array[i] = 0;
	}
}


static void OUT_GroupUpdate(Out_Group_t* out_group){

	switch (out_group->mode){

		case OUT_GROUP_MODE_SINGLE_ACTIVE:

			OutGroupClear(out_group);

			if ((*out_group->out_array_ptr)[out_group->index] != NULL){
				OUT_On((*out_group->out_array_ptr)[out_group->index]);
			}

			out_group->active_array[out_group->index] = 1;


			break;

		case OUT_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;
	}

	if (out_group->OUTGROUP_Event_ChangeState_Callback_Flag == 1){
		if (out_group->OUTGROUP_Event_ChangeState_Callback != NULL){
			(*out_group->OUTGROUP_Event_ChangeState_Callback)(out_group);
		}
	}

	out_group->state = OUT_GROUP_STATE_UPDATED;
}


void OUT_GroupService(Out_Group_t* out_group){

	if (out_group->state == OUT_GROUP_STATE_NEED_UPDATE){
		OUT_GroupUpdate(out_group);
	}

}


void OUT_On(Out_t* out){

	out->state = OUT_ON;

	switch (out->type) {

		case OUT_TYPE_GPIO:

			switch (out->polarity){

				case OUT_POLARITY_POSITIVE:
					HAL_GPIO_WritePin(out->port, out->pin, GPIO_PIN_SET);
					break;

				case OUT_POLARITY_NEGATIVE:
					HAL_GPIO_WritePin(out->port, out->pin, GPIO_PIN_RESET);
					break;
			}
			break;

		case OUT_TYPE_REG:

			switch (out->polarity){

				case OUT_POLARITY_POSITIVE:
					setRegBit(out->reg, out->id);
					break;

				case OUT_POLARITY_NEGATIVE:
					resetRegBit(out->reg, out->id);
					break;
			}
			break;
	}

	if (out->OUT_Event_ChangeState_Callback_Flag == 1){
		if (out->OUT_Event_ChangeState_Callback != NULL){
			(*out->OUT_Event_ChangeState_Callback)(out);
		}
	}

}

void OUT_Off(Out_t* out){

	out->state = OUT_OFF;

	switch (out->type) {

		case OUT_TYPE_GPIO:

			switch (out->polarity){

				case OUT_POLARITY_POSITIVE:
					HAL_GPIO_WritePin(out->port, out->pin, GPIO_PIN_RESET);
					break;

				case OUT_POLARITY_NEGATIVE:
					HAL_GPIO_WritePin(out->port, out->pin, GPIO_PIN_SET);
					break;
			}
			break;

		case OUT_TYPE_REG:

			switch (out->polarity){

				case OUT_POLARITY_POSITIVE:
					resetRegBit(out->reg, out->id);
					break;

				case OUT_POLARITY_NEGATIVE:
					setRegBit(out->reg, out->id);
					break;
			}
			break;
	}

	if (out->OUT_Event_ChangeState_Callback_Flag == 1){
		if (out->OUT_Event_ChangeState_Callback != NULL){
			(*out->OUT_Event_ChangeState_Callback)(out);
		}
	}

}

static void OutReadState(Out_t* out){

	switch (out->polarity){

		case OUT_POLARITY_POSITIVE:

			if (HAL_GPIO_ReadPin(out->port, out->pin) == GPIO_PIN_SET){
				out->state = OUT_ON;
			}
			else {
				out->state = OUT_OFF;
			}
			break;

		case OUT_POLARITY_NEGATIVE:

			if (HAL_GPIO_ReadPin(out->port, out->pin) == GPIO_PIN_RESET){
				out->state = OUT_ON;
			}
			else {
				out->state = OUT_OFF;
			}
			break;
	}
}

int32_t OUT_Toggle(Out_t* out){

	switch (out->type) {

		case OUT_TYPE_GPIO:

		 	 HAL_GPIO_TogglePin(out->port, out->pin);
		 	 OutReadState(out);
		 	 break;

		case OUT_TYPE_REG:

			if (OUT_GetState(out) > 0){
				OUT_Off(out);
			}
			else {
				OUT_On(out);
			}
			break;

	}


	return out->state;

}

void OUT_SetState(Out_t* out, Out_state state, bool executeCallback){ // false - no execute, true - like before

	out->state = state;

	bool currentExecuteCallbackFlag = out->OUT_Event_ChangeState_Callback_Flag;

	out->OUT_Event_ChangeState_Callback_Flag = executeCallback;

	switch (out->state){

		case OUT_OFF:
			OUT_Off(out);
			break;

		case OUT_ON:
			OUT_On(out);
			break;

		default:
			break;
	}

	out->OUT_Event_ChangeState_Callback_Flag = currentExecuteCallbackFlag;
}

Out_state OUT_GetState(Out_t* out){
	return out->state;
};


static void OutBlinkCallback(void* ptr){

	Out_t* out = (Out_t*)ptr;

	if (out != NULL){
		OUT_Toggle(out);
	}
}

void OUT_Blink_Start(Out_t* out, uint32_t delay){
	Timer_Init(out, &out->timer, delay, TIMER_MODE_CONTINOUS);
	TimerRegisterCallback(&out->timer, OutBlinkCallback, TIMER_EVENT_ELAPSED);
	TimerStart(&out->timer);
}

void OUT_Blink_Stop(Out_t* out){
	TimerStop(&out->timer);
}

int32_t OUT_Group_Increase(Out_Group_t* out_group){

	out_group->index++;
	out_group->state = OUT_GROUP_STATE_NEED_UPDATE;

	switch (out_group->mode){

		case OUT_GROUP_MODE_SINGLE_ACTIVE:

			if (out_group->index > out_group->size - 1){
					out_group->index = 0;
			}

			break;

		case OUT_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;

	}

	return out_group->index;

}

int32_t OUT_Group_Decrease(Out_Group_t* out_group){

	out_group->index--;
	out_group->state = OUT_GROUP_STATE_NEED_UPDATE;

	switch (out_group->mode){

		case OUT_GROUP_MODE_SINGLE_ACTIVE:
			if (out_group->index < 0){
				out_group->index = out_group->size - 1;
			}

			break;

		case OUT_GROUP_MODE_MULTI_ACTIVE:
			break;

		default:
			break;

	}

	return out_group->index;
}

void OUT_Group_SetActiveIndex(Out_Group_t* out_group, int32_t index){

	if(index > out_group->size - 1){
		index = out_group->size - 1;
	}

	if(index < 0){
		index = 0;
	}

	if (index != out_group->index){
		out_group->index = index;
		out_group->state = OUT_GROUP_STATE_NEED_UPDATE;
	}
}

Out_t* OUT_Group_GetActiveOut(Out_Group_t* out_group){
	return (*out_group->out_array_ptr)[out_group->index];
}

int32_t OUT_Group_GetActiveIndex(Out_Group_t* out_group){
	return out_group->index;
}


void OUT_CallbackRegister(Out_t* out, void (*CallbackPtr)(void*), Out_events event){

	switch (event){

		case OUT_EVENT_CHANGE_STATE:
			out->OUT_Event_ChangeState_Callback = CallbackPtr;
			out->OUT_Event_ChangeState_Callback_Flag = 1;
			break;

		default:
			break;
	}

}
void OUTGROUP_CallbackRegister(Out_Group_t* outGroup, void (*CallbackPtr)(void*), Out_Group_events event){

	switch (event){

		case OUT_GROUP_EVENT_CHANGE_STATE:
			outGroup->OUTGROUP_Event_ChangeState_Callback = CallbackPtr;
			outGroup->OUTGROUP_Event_ChangeState_Callback_Flag = 1;
			break;

		default:
			break;
	}

}
