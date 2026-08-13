/*
 * potentiometer.c
 *
 *  Created on: Mar 14, 2021
 *      Author: Dom
 */


#include <motopot_old.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


//#include "adc.h"


#define MOTOPOT_CALIBATION_TIME 12000
#define MOTOPOT_TOLERANCE 10

#define KNEE_VALUE 74 //64
#define KNEE_POSITION  185 //166

#define MAX_POT_VALUE 255

#define ACTIVE_TIME 250
#define IDLE_TIME 250
#define MIN_SPEED 60
#define MAX_SPEED 255
#define LIN_STOP 23
#define LOG_STOP 12


#ifdef HAL_ADC_MODULE_ENABLED

//////////////////////////////////////////////////////////////////////////////////////////////

static void motorMoveCW_PWM(Motor_PWM_t* motor, uint16_t speed){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, speed);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, 0);
	HAL_TIM_PWM_Start(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Start(motor->B.htim, motor->B.channel);
}

static void motorMoveCCW_PWM(Motor_PWM_t* motor, uint16_t speed){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, 0);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, speed);
	HAL_TIM_PWM_Start(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Start(motor->B.htim, motor->B.channel);
}

static void motorStop_PWM(Motor_PWM_t* motor){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, 0);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, 0);
	HAL_TIM_PWM_Stop(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Stop(motor->B.htim, motor->B.channel);
}

//////////////////////////////////////////////////////////////////////////////////////////////

static void motorMoveCW(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_RESET);
}

static void motorMoveCCW(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_SET);
}

static void motorStop(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_RESET);
}

//////////////////////////////////////////////////////////////////////////////////////////////



#define MOTOPOT_DETECT_SPEED_TIME 65

static void PotSpeedDetectElapsedCallback(void* handle){

	Motopot2_t* motopot = (Motopot2_t*)handle;

	uint32_t dist = abs(Pot_getCurrentPosition(&motopot->pot) - motopot->temp_position);
	uint32_t time = Timer_GetTime(&motopot->speed_timer);

	if (time > 0){
		motopot->speed = (motopot->speed + ((dist * 1000) / time)) / 2;
		motopot->temp_position = Pot_getCurrentPosition(&motopot->pot);
	}
}

void Motopot2_Init(Pot_t* pot, Motor_t* motor){

};

void Motopot2_Init(Motopot2_t* motopot, char* name, Pot_type type, ADC_HandleTypeDef* hadc, uint32_t channel, TIM_HandleTypeDef* htim_A, uint32_t channel_A, TIM_HandleTypeDef* htim_B, uint32_t channel_B){


	Pot_Init(&motopot->pot, name, type, hadc, channel);

	Motor_PWM_Init(&motopot->motor, MOTOR_MODE_STANDARD, htim_A, channel_A, htim_B, channel_B);
	Motor_RampInit(&motopot->motor, 1000, 1000);

	motopot->name = motopot->pot.name;
	motopot->id = motopot->pot.channel;

	motopot->state = MOTOPOT_STATE_IDLE;
	motopot->calibrated_flag = false;
	motopot->tolerance = MOTOPOT_TOLERANCE;

	motopot->speed = 0;

	motopot->temp_position = Pot_GetCurrentPosition(&motopot->pot);

	Timer_Init(motopot, &motopot->timer, 0, TIMER_MODE_CONTINOUS);

	Timer_Init(motopot, &motopot->speed_timer, MOTOPOT_DETECT_SPEED_TIME, TIMER_MODE_CONTINOUS);
	TimerRegisterCallback(&motopot->speed_timer, PotSpeedDetectElapsedCallback, TIMER_EVENT_ELAPSED);
	TimerStart(&motopot->speed_timer);

	motopot->Event_Idle_Callback = NULL;
	motopot->Event_Idle_Callback_Flag = true;;

	motopot->Event_Start_Callback = NULL;
	motopot->Event_Start_Callback_Flag = true;

	motopot->Event_Stop_Callback = NULL;
	motopot->Event_Stop_Callback_Flag = true;

	motopot->Event_MovingByMotor_Callback = NULL;
	motopot->Event_MovingByMotor_Callback_Flag = true;

	motopot->Event_MovingByHand_Callback = NULL;
	motopot->Event_MovingByHand_Callback_Flag = true;

	motopot->Event_ForceStop_Callback = NULL;
	motopot->Event_ForceStop_Callback_Flag = true;

//	motopot->move = false;
//	motopot->saved = false;
//	motopot->to_send = false;
//	motopot->last_sent_value = -1;
};



static void  MotopotStateDetect(Motopot2_t* motopot){

	Pot_state pot_state = Pot_getState(&motopot->pot);
	Motor_state motor_state = Motor_GetState(&motopot->motor);

	if (pot_state == POT_STATE_IDLE && motor_state == MOTOR_STATE_STOP){
		motopot->state = MOTOPOT_STATE_IDLE;
	}

	if (pot_state == POT_STATE_IDLE && motor_state != MOTOR_STATE_STOP){

		if (motopot->state == MOTOPOT_STATE_IDLE){
			motopot->state = MOTOPOT_STATE_START;
		}

		if (motopot->state == MOTOPOT_MOVING_BY_MOTOR){
			motopot->state = MOTOPOT_FORCE_STOP;
		}
	}

	if (pot_state != POT_STATE_IDLE && motor_state != MOTOR_STATE_STOP ){
		motopot->state = MOTOPOT_MOVING_BY_MOTOR;
	}

	if (pot_state != POT_STATE_IDLE && motor_state == MOTOR_STATE_STOP ){
		motopot->state = MOTOPOT_MOVING_BY_HAND;
	}
}

static void Motopot2_Start(Motopot2_t* motopot){

	motopot->state = MOTOPOT_STATE_START;
	Motor_Start(&motopot->motor);

}

void Motopot2_Stop(Motopot2_t* motopot){

	motopot->state = MOTOPOT_STATE_STOP;
	Motor_Stop(&motopot->motor);

}

static void MotopotStartInit(Motopot2_t* motopot){

	if (motopot->motor.mode == MOTOR_MODE_STANDARD){
		motopot->ramp.state = MOTOPOT_RAMP_STATE_CONST;

		if (motopot->pot.desired_position > motopot->pot.current_position){
			Motor_Move(&motopot->motor, motopot->motor.maxSpeed);
		}
		else if (motopot->pot.desired_position < motopot->pot.current_position){
			Motor_Move(&motopot->motor, -motopot->motor.maxSpeed);
		}
	}

	if (motopot->motor.mode == MOTOR_MODE_RAMP){
		motopot->ramp.state = MOTOPOT_RAMP_STATE_ACCEL;

		if (motopot->pot.desired_position > motopot->pot.current_position){
			Motor_SoftStartInit(&motopot->motor, motopot->motor.maxSpeed);
		}
		else if (motopot->pot.desired_position < motopot->pot.current_position){
			Motor_SoftStartInit(&motopot->motor, -motopot->motor.maxSpeed);
		}
	}
}

static void MotopotRampConstService(Motopot2_t* motopot){

	int32_t distance = motopot->pot.desired_position - motopot->pot.current_position;

	switch (Motor_GetState(&motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:

			if (distance <= 4 * (motopot->tolerance / (motopot->motor.maxSpeed / motopot->motor.speed))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
				Motor_SoftStopInit(&motopot->motor);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:

			if (distance >= -4 * (motopot->tolerance / (motopot->motor.maxSpeed / abs(motopot->motor.speed)))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
				Motor_SoftStopInit(&motopot->motor);
			}
			break;

		default:
			break;
	}
}

static void MotopotStandardConstService(Motopot2_t* motopot){

	int32_t distance = motopot->pot.desired_position - motopot->pot.current_position;

	switch (Motor_GetState(&motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:

			if (distance <= motopot->tolerance / (motopot->motor.maxSpeed / motopot->motor.speed)){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
				Motor_SetSpeed(&motopot->motor, motopot->motor.min_speed);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:

			if (distance >= -(motopot->tolerance / (motopot->motor.maxSpeed / abs(motopot->motor.speed)))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
				Motor_SetSpeed(&motopot->motor, -motopot->motor.min_speed);
			}
			break;

		default:
			break;
	}
}

static void MotopotFinishService(Motopot2_t* motopot){


	switch (Motor_GetState(&motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:
			if (motopot->pot.current_position >= motopot->pot.desired_position){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_IDLE;
				motopot->motor.ramp.state = MOTOR_RAMP_IDLE;
				Motopot2_Stop(motopot);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:
			if (motopot->pot.current_position <= motopot->pot.desired_position){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_IDLE;
				motopot->motor.ramp.state = MOTOR_RAMP_IDLE;
				Motopot2_Stop(motopot);
			}
			break;

		default:
			break;
	}

}

void Motopot2_CallbackRegister(Motopot2_t* motopot, void (*CallbackPtr)(void*), Motopot2_event event){

	switch (event){

	case MOTOPOT_EVENT_IDLE:
		motopot->Event_Idle_Callback = CallbackPtr;
		motopot->Event_Idle_Callback_Flag = true;
		break;

	case MOTOPOT_EVENT_START:
		motopot->Event_Start_Callback = CallbackPtr;
		motopot->Event_Start_Callback_Flag = true;
		break;

	case MOTOPOT_EVENT_STOP:
		motopot->Event_Stop_Callback = CallbackPtr;
		motopot->Event_Stop_Callback_Flag = true;
		break;

	case MOTOPOT_EVENT_MOVING_BY_MOTOR:
		motopot->Event_MovingByMotor_Callback = CallbackPtr;
		motopot->Event_MovingByMotor_Callback_Flag = true;
		break;

	case MOTOPOT_EVENT_MOVING_BY_HAND:
		motopot->Event_MovingByHand_Callback = CallbackPtr;
		motopot->Event_MovingByHand_Callback_Flag = true;
		break;

	case MOTOPOT_EVENT_FORCE_STOP:
		motopot->Event_ForceStop_Callback = CallbackPtr;
		motopot->Event_ForceStop_Callback_Flag = true;
		break;

	default:
		break;
	}
}



//static void Motopot2_SetPosition(Motopot2_t* motopot, int32_t position){
//	motopot->pot.desired_position = position;
//}

void Motopot2_MoveTo(Motopot2_t* motopot, uint32_t position){
	Pot_setDesiredPosition(&motopot->pot, position);

	MotopotStartInit(motopot);

//	if (motopot->motor.mode == MOTOR_MODE_STANDARD){
//		Motopot_StandardInit(motopot);
//	}
//
//	if (motopot->motor.mode == MOTOR_MODE_RAMP){
//		Motopot_SoftStartInit(motopot);
//	}

}

void Motopot2_MoveTime(Motopot2_t* motopot, int32_t speed, uint32_t time){

	Motor_MoveTime_NonBlockingMode(&motopot->motor, speed, time);

}

static void MotopotMovingByMotorService(Motopot2_t* motopot){

	TimerService(&motopot->ramp.timer);
//	int32_t distance = motopot->pot.desired_position - motopot->pot.current_position;
//	uint32_t time_to_stop = distance / motopot->pot.speed;


	switch(motopot->ramp.state){

		case MOTOPOT_RAMP_STATE_IDLE:
			break;

		case MOTOPOT_RAMP_STATE_ACCEL:

			if (Motor_GetRampState(&motopot->motor) == MOTOR_RAMP_CONST){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_CONST;
			}

			if (Motor_GetRampState(&motopot->motor) == MOTOR_RAMP_DECEL){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
			}

//			if(time_to_stop <= motopot->ramp.decel_time){
//				Motopot_SoftStopInit(motopot);
//			}


			break;

		case MOTOPOT_RAMP_STATE_CONST:

//			if(time_to_stop <= motopot->ramp.decel_time){
//				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
//				Motopot_SoftStopInit(motopot);
//			}

			if (motopot->motor.mode == MOTOR_MODE_RAMP){
				MotopotRampConstService(motopot);
			}

			if (motopot->motor.mode == MOTOR_MODE_STANDARD){
				MotopotStandardConstService(motopot);
			}


			break;

		case MOTOPOT_RAMP_STATE_DECEL:

			if (Motor_GetRampState(&motopot->motor) == MOTOR_RAMP_FINISH){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
			}

			break;

		case MOTOPOT_RAMP_STATE_FINISH:
			MotopotFinishService(motopot);
			break;

		default:
			break;
	}
}

uint32_t Motopot2_Service(Motopot2_t* motopot){ // motopot2 motopot[]

	TimerService(&motopot->timer);
	TimerService(&motopot->speed_timer);

	Pot_Service(&motopot->pot);
	Motor_Service(&motopot->motor);

	MotopotMovingByMotorService(motopot);

	MotopotStateDetect(motopot);

	switch (motopot->state){

		case MOTOPOT_STATE_IDLE:

			if (motopot->Event_Idle_Callback_Flag == true){
				if (motopot->Event_Idle_Callback != NULL){
					(*motopot->Event_Idle_Callback)(motopot);
				}
			}

			break;

		case MOTOPOT_STATE_START:

			if (motopot->Event_Start_Callback_Flag == true){
				if (motopot->Event_Start_Callback != NULL){
					(*motopot->Event_Start_Callback)(motopot);
				}
				motopot->Event_Start_Callback_Flag = false;
			}

			break;

		case MOTOPOT_STATE_STOP:

			if (motopot->Event_Stop_Callback_Flag == true){
				if (motopot->Event_Stop_Callback != NULL){
					(*motopot->Event_Stop_Callback)(motopot);
				}
				motopot->Event_Stop_Callback_Flag = false;
			}
			break;

		case MOTOPOT_MOVING_BY_HAND:

			if (motopot->Event_MovingByMotor_Callback_Flag == true){
				if (motopot->Event_MovingByMotor_Callback != NULL){
					(*motopot->Event_MovingByMotor_Callback)(motopot);
				}
			}
			break;

		case MOTOPOT_MOVING_BY_MOTOR:

			if (motopot->Event_MovingByHand_Callback_Flag == true){
				if (motopot->Event_MovingByHand_Callback != NULL){
					(*motopot->Event_MovingByHand_Callback)(motopot);
				}
				motopot->Event_MovingByHand_Callback_Flag = false;
			}
			break;

		case MOTOPOT_FORCE_STOP:

			if (motopot->Event_ForceStop_Callback_Flag == true){
				if (motopot->Event_ForceStop_Callback != NULL){
					(*motopot->Event_ForceStop_Callback)(motopot);
				}
				motopot->Event_ForceStop_Callback_Flag = false;
			}
			break;

		default:
			break;
	}

	return TimerGetTime(&motopot->timer);

}


Motopot2_state Motopot2_GetState(Motopot2_t* motopot){
	return motopot->state;
}

void Motopot2_Calibration(Motopot2_t* motopot[], uint32_t count){

	PRINT_MSG("\r\n");
	PRINT_MSG("====== MOTOPOT CALIBRATION START======\r\n");

	uint32_t timeout_start, timeout, i;
	_Bool forced_all_pots;

	for (i = 0; i < count; i++){
		Motor_SetSpeed(&motopot[i]->motor, motopot[i]->motor.maxSpeed);
	    Motopot2_Start(motopot[i]);
	    // short version Motopot2_move() ????
	}

	timeout_start = HAL_GetTick();
	do {
		forced_all_pots = true;
		timeout = HAL_GetTick() - timeout_start;

		for (i = 0; i < count; i++){
			Motopot2_Service(motopot[i]);
			if (Motopot2_GetState(motopot[i]) == MOTOPOT_FORCE_STOP){
				Motopot2_Stop(motopot[i]);
				motopot[i]->calibrated_flag = true;
			}
		}
		for (i = 0; i < count; i++){
			if (motopot[i]->calibrated_flag == false){
				forced_all_pots = 0;
			}
		}
	} while (forced_all_pots != true && timeout < MOTOPOT_CALIBATION_TIME);

	for (i = 0; i < count; i++){
		Pot_setMaxValue(&motopot[i]->pot, Pot_getCurrentPosition(&motopot[i]->pot));
		motopot[i]->calibrated_flag = 0;

		sprintf(debugMessageBuffer, "%s # right time: %ld \r\n",motopot[i]->name, motopot[i]->timer.time);
		PRINT_MSG(debugMessageBuffer);
	}

	for (i = 0; i < count; i++){
		Motor_SetSpeed(&motopot[i]->motor, -motopot[i]->motor.maxSpeed);
		Motopot2_Start(motopot[i]);
		// short version Motopot2_move() ????
	}

	HAL_Delay(500);

	timeout_start = HAL_GetTick();
	do {
			timeout = HAL_GetTick() - timeout_start;
			forced_all_pots = true;

			for (i = 0; i < count; i++){
				Motopot2_Service(motopot[i]);
				if (Motopot2_GetState(motopot[i]) == MOTOPOT_FORCE_STOP){
					Motopot2_Stop(motopot[i]);
					motopot[i]->calibrated_flag = true;
				}
			}

			for (i = 0; i < count; i++){
				if (motopot[i]->calibrated_flag == false){
					forced_all_pots = 0;
				}
			}
		} while (forced_all_pots != true && timeout < MOTOPOT_CALIBATION_TIME);

	HAL_Delay(500);

	for (i = 0; i < count; i++){
		Pot_setMinValue(&motopot[i]->pot, Pot_getCurrentPosition(&motopot[i]->pot));
		sprintf(debugMessageBuffer, "%s # left time: %ld \r\n",motopot[i]->name, motopot[i]->timer.time);
		PRINT_MSG(debugMessageBuffer);
	}

	for (i = 0; i < count; i++){
		sprintf(debugMessageBuffer, "%s # min: %ld | max: %ld \r\n", motopot[i]->name, motopot[i]->pot.min_value, motopot[i]->pot.max_value);
		PRINT_MSG(debugMessageBuffer);

		Motopot2_Stop(motopot[i]);
	}

	PRINT_MSG("====== MOTOPOT CALIBRATION END ======\r\n");
}


//void Pot2_MoveTo(Pot_t* motopot, int32_t set_position, int16_t speed){
//
//	switch (motopot->motor.motor_type){
//
//	case MOTOR_PWM:
//
//		if (motopot->current_position < set_position)
//			motorMoveCW_PWM(motopot->motor.motor_pwm, speed);
//
//		if (motopot->current_position > set_position)
//			motorMoveCCW_PWM(motopot->motor.motor_pwm, speed);
//
//		if (motopot->current_position == set_position)
//			motorStop_PWM(motopot->motor.motor_pwm);
//
//		break;
//
//	case MOTOR_GPIO:
//
//		if (motopot->current_position < set_position)
//			motorMoveCW(motopot->motor.motor_gpio);
//
//		if (motopot->current_position > set_position)
//			motorMoveCCW(motopot->motor.motor_gpio);
//
//		if (motopot->current_position == set_position)
//			motorStop(motopot->motor.motor_gpio);
//
//		break;
//
//	default:
//		break;
//
//	}
//}
//
//void Pot2_Stop(Pot_t* motopot){
//
//	switch (motopot->motor.motor_type)
//		{
//			case MOTOR_GPIO:
//				motorStop(motopot->motor.motor_gpio);
//				break;
//
//			case MOTOR_PWM:
//				motorStop_PWM(motopot->motor.motor_pwm);
//				break;
//
//			default:
//				break;
//		}
//
//}
//
//uint16_t Pot2_ReadCurrentPosition(Pot_t* motopot){
//
//	motopot->current_position = adc_buffer.buffer[motopot->analog.channel - 1];
//
//	return motopot->current_position;
//
//}
//
//void Pot2_SaveLastPosition(Pot_t* motopot){
//
//	motopot->last_position = motopot->current_position;
//
//}
//
//void Pot2_Calibrate(Pot_t* motopot[], uint8_t numPots, uint32_t calibration_time){
//
//	uint32_t timeout_start = HAL_GetTick();
//	uint32_t timeout;
//	uint32_t timer_start = HAL_GetTick();
//	uint32_t timer;
//	int i;
//
//	HAL_Delay(5);
//
//	for (i = 0; i < numPots; i++){
//		Pot2_ReadCurrentPosition(motopot[i]);
//		Pot2_SaveLastPosition(motopot[i]);
//		Pot2_MoveTo(motopot[i], motopot[i]->max, motopot[i]->maxSpeed);
//	}
//
//	do {
//
//		timeout = HAL_GetTick() - timeout_start;
//		timer = HAL_GetTick() - timer_start;
//
//		// read position per 10 ms
//		if (timer >= 10){
//			timer_start = HAL_GetTick();
//			for (i = 0; i < numPots; i++){
//				Pot2_ReadCurrentPosition(motopot[i]);
//			}
//		}
//
//	} while (timeout < calibration_time);
//
//	for (i = 0; i < numPots; i++){
//		motopot[i]->max = Pot2_ReadCurrentPosition(motopot[i]);
//		Pot2_MoveTo(motopot[i], motopot[i]->min, motopot[i]->maxSpeed);
//	}
//
//	timeout_start = HAL_GetTick();
//	timer_start = HAL_GetTick();
//
//	do {
//		timeout = HAL_GetTick() - timeout_start;
//		timer = HAL_GetTick() - timer_start;
//
//		 //read position per 10 ms
//		if (timer >= 10){
//			timer_start = HAL_GetTick();
//			for (i = 0; i < numPots; i++){
//				Pot2_ReadCurrentPosition(motopot[i]);
//			}
//		}
//
//	} while (timeout < calibration_time);
//
//
//	for (i = 0; i < numPots; i++){
//		motopot[i]->min = motopot[i]->current_position;
//		Pot2_Stop(motopot[i]);
//	}
//}

//void setPotPosition(MotopotOld_t* p, int pos){
//
//	if (pos < p->min)
//		pos = p->min;
//
//	if (pos > p->max)
//		pos = p->max;
//
//	p->desired_position = pos;
//	p->total_distance = abs(p->current_position - p->desired_position);
//	p->current_speed = p->min_speed;
//
//	p->move = true;
//
//	if (p->desired_position > p->current_position)
//		p->direction = UP;
//	else if (p->desired_position < p->current_position)
//		p->direction = DOWN;
//	else
//		p->move = false;
//
//
//	switch(p->type){
//
//		case LINEAR:
//			createLinearRamp(p);
//			break;
//
//		case LOGARYTMIC:
//			createLogRamp(p);
//			break;
//	}
//}
//
//void createLogRamp(MotopotOld_t* p){
//
//	int32_t real_desired_pos, real_stop_pos;
//
//	real_desired_pos = valToPos(p->desired_position);
//
//	p->ramp.accel = 1;
//
//	p->ramp.target_speed = p->maxSpeed;
//
//	switch (p->direction){
//
//		case UP:
//			p->ramp.full_position = p->max;
//			real_stop_pos = real_desired_pos - ((MAX_POT_VALUE * 6)/100);
//			break;
//
//		case DOWN:
//			p->ramp.full_position = p->min;
//			real_stop_pos = real_desired_pos + ((MAX_POT_VALUE * 6)/100);
//			break;
//	}
//
//	p->ramp.stop_position = posToVal(real_stop_pos);
//
//	if (p->ramp.stop_position < 0)
//		p->ramp.stop_position = 0;
//
//	p->ramp.decel = 1;
//}
//
//void createLinearRamp(MotopotOld_t* p){
//	p->ramp.full_position = p->total_distance;
//	p->ramp.accel = 1;
//
//	p->ramp.target_speed = p->maxSpeed;
//
//	// 6% z maxymalnej wartości skali
//	p->ramp.stop_position = p->total_distance - ((MAX_POT_VALUE * 6)/100);
//
//	if (p->ramp.stop_position < 0)
//		p->ramp.stop_position = 0;
//
//	p->ramp.decel = 1;
//}
//
//uint32_t getRealDistance(uint32_t currentValue, uint32_t desiredValue){
//
//	uint32_t result;
//
//	if (
//			((currentValue < KNEE_VALUE) && (desiredValue > KNEE_VALUE)) ||
//			((currentValue > KNEE_VALUE) && (desiredValue < KNEE_VALUE))
//		){
//
//		result = abs(valToPos(currentValue) - valToPos(KNEE_VALUE)) + abs(valToPos(desiredValue) - valToPos(KNEE_VALUE));
//
//	}
//
//	else {
//
//		result = abs(valToPos(currentValue) - valToPos(desiredValue));
//	}
//
//	return result;
//}
//
//void pot_ramp_init(MotopotOld_t* p){
//}
//
//#define POT_MOTOR_RAMP_PERIOD 1
//void pot_ramp_process(MotopotOld_t* p){
//
//	if (p->move == false){
//		p->ramp_timer = 0;
//		p->total_distance = 0;
//		return;
//	}
//	else{
//
//		if (p->ramp_timer == 0){
//			p->ramp_timer = HAL_GetTick();
//		}
//
//		if((HAL_GetTick() - p->ramp_timer) < POT_MOTOR_RAMP_PERIOD){
//			return;
//		}
//
//		p->ramp_timer = HAL_GetTick();
//
//		switch (p->type){
//
//			case LINEAR:
//				linearRampProcess(p);
//				break;
//
//			case LOGARYTMIC:
//				logarytmicRampProcess(p);
//				break;
//		}
//	}
//}
//
//void linearRampProcess(MotopotOld_t* p){
//
//	p->current_distance = p->total_distance - abs(p->current_position - p->desired_position);
//	// acceleration
//	if(p->current_distance < p->ramp.full_position){
//
//		// peona rampa
//		if (p->current_distance < p->ramp.stop_position ){
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//
//			p->current_speed += p->ramp.accel;
//
//			if (p->current_speed > p->ramp.target_speed)
//				p->current_speed = p->ramp.target_speed;
//
//			if (p->current_speed > p->maxSpeed)
//				p->current_speed = p->maxSpeed;
//		}
//
//		// nie pełna rampa
//		else {
//
//			p->current_speed -= p->ramp.decel;
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//		}
//
//	}
//
//	// deceleration
//	else if (p->current_distance > p->ramp.stop_position ){
//
//		p->current_speed -= p->ramp.decel;
//
//		if (p->current_speed < p->min_speed){
//			p->current_speed = p->min_speed;
//		}
//	}
//
//	else {
//	//stała prędkość
//		p->current_speed = p->ramp.target_speed;
//	}
//}
//
//void logarytmicRampProcess(MotopotOld_t* p){
//
//	switch (p->direction){
//
//	case UP:
//		potGoingUp(p);
//		break;
//
//	case DOWN:
//		potGoingDown(p);
//		break;
//	}
//}
//
//void potGoingUp(MotopotOld_t* p){
//
//	// acceleration
//	if (p->current_position < p->ramp.full_position){
//
//		// pełna rampa
//		if (p->current_position < p->ramp.stop_position){
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//
//			p->current_speed += p->ramp.accel;
//
//			if (p->current_speed > p->ramp.target_speed)
//				p->current_speed = p->ramp.target_speed;
//
//			if (p->current_speed > p->maxSpeed;
//				p->current_speed = p->maxSpeed;
//		}
//
//		// nie pełna rampa
//		else{
//			p->current_speed -= p->ramp.decel;
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//		}
//	}
//
//	// deceleration
//	else if (p->current_position > p->ramp.stop_position){
//
//		p->current_speed -= p->ramp.decel;
//
//		if (p->current_speed < p->min_speed)
//			p->current_speed = p->min_speed;
//	}
//
//	// constans speed
//	else {
//		p->current_speed = p->ramp.target_speed;
//	}
//
//}
//
//void potGoingDown(MotopotOld_t* p){
//
//	// acceleration
//	if (p->current_position > p->ramp.full_position){
//
//		// pełna rampa
//		if (p->current_position > p->ramp.stop_position){
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//
//			p->current_speed += p->ramp.accel;
//
//			if (p->current_speed > p->ramp.target_speed)
//				p->current_speed = p->ramp.target_speed;
//
//			if (p->current_speed > p->maxSpeed)
//				p->current_speed = p->maxSpeed;
//		}
//
//		// nie pełna rampa
//		else{
//			p->current_speed -= p->ramp.decel;
//
//			if (p->current_speed < p->min_speed)
//				p->current_speed = p->min_speed;
//		}
//	}
//
//	// deceleration
//	else if (p->current_position < p->ramp.stop_position){
//
//		p->current_speed -= p->ramp.decel;
//
//		if (p->current_speed < p->min_speed)
//			p->current_speed = p->min_speed;
//	}
//
//	// constans speed
//	else {
//		p->current_speed = p->ramp.target_speed;
//	}
//}
//
//void testPot(MotopotOld_t* p){
//
//	static char debugMessageBuffer[32];
//
//	uint8_t values[] = {
//			p->min, p->max,
//			p->min, 100, 200, 100,
//			p->min, 50, 100, 150, 200, 250,
//			p->min, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240,
//			p->min, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 240, 250,
//			p->min, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125,
//	};
//
//	static int index = 0;
//
//	setPotPosition(p, values[index]);
//
//	sprintf(debugMessageBuffer,"Test motopot Value: %i:%i\r\n", index, values[index]);
//	PRINT_STR(debugMessageBuffer);
//
//	index++;
//	if (index >= sizeof(values))
//		index = 0;
//}
//
//uint16_t valToPos(uint16_t val){;
//
//	if (val < KNEE_VALUE) // 64 or 165
//		return (val * 10) / 4;
//	else
//		return ((val * 10) + (280 * 10)) / 21;
//}
//
//uint16_t posToVal(uint16_t pos){
//
//	if (pos < KNEE_POSITION) // 64 or 165
//		return (pos * 4) / 10;
//	else
//		return ((pos * 21) - (280 * 10)) / 10;
//}
//
//
//
//void potMoveCW(MotopotOld_t* motopot){
//
//	motopot->status = MOTOPOT_STATUS_MOTOR_RIGHT;
//
//	switch (motopot->motor_type)
//	{
//		case MOTOR_GPIO:
//			motorMoveCW(&motopot->motor_gpio);
//			break;
//
//		case MOTOR_PWM:
//			motorMoveCW_PWM(&motopot->motor_pwm, motopot->current_speed);
//			break;
//	}
//}
//
//void potMoveCCW(MotopotOld_t* motopot){
//
//	motopot->status = MOTOPOT_STATUS_MOTOR_LEFT;
//
//	switch (motopot->motor_type)
//	{
//		case MOTOR_GPIO:
//			motorMoveCCW(&motopot->motor_gpio);
//			break;
//
//		case MOTOR_PWM:
//			motorMoveCCW_PWM(&motopot->motor_pwm, motopot->current_speed);
//			break;
//	}
//}
//
//void potStopp(MotopotOld_t* motopot){
//
//	motopot->status = MOTOPOT_STATUS_MOTOR_STOP;
//	motopot->move = false;
//
//	switch (motopot->motor_type)
//	{
//		case MOTOR_GPIO:
//			motorStop(&motopot->motor_gpio);
//			break;
//
//		case MOTOR_PWM:
//			motorStop_PWM(&motopot->motor_pwm);
//			break;
//	}
//}
//
//void updatePotPosition(MotopotOld_t* motopot){
//
//}

#endif /* HAL_ADC_MODULE_ENABLED */

