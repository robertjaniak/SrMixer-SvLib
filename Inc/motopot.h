/*
 * potentiometr.h
 *
 *  Created on: Mar 14, 2021
 *      Author: Dom
 */

#ifndef INC_MOTOPOT_H_
#define INC_MOTOPOT_H_

#include "global.h"
#include "motor.h"
#include "pot.h"
#include "pid_custom.h"

typedef enum {

	MOTOPOT_EVENT_IDLE       = 0,
	MOTOPOT_EVENT_START      = 1,
	MOTOPOT_EVENT_STOP       = 2,
	MOTOPOT_EVENT_MOVING_BY_MOTOR  = 3,
	MOTOPOT_EVENT_MOVING_BY_HAND   = 4,
	MOTOPOT_EVENT_FORCE_STOP       = 5,

}Motopot_event;;

typedef enum {
	MOTOPOT_STATE_IDLE       = 0,
	MOTOPOT_STATE_START      = 1,
	MOTOPOT_STATE_STOP       = 2,
	MOTOPOT_MOVING_BY_MOTOR  = 3,
	MOTOPOT_MOVING_BY_HAND   = 4,
	MOTOPOT_FORCE_STOP       = 5,
}Motopot_state;

typedef enum {
	MOTOPOT_RAMP_STATE_IDLE   = 0,
	MOTOPOT_RAMP_STATE_ACCEL  = 1,
	MOTOPOT_RAMP_STATE_CONST  = 2,
	MOTOPOT_RAMP_STATE_DECEL  = 3,
	MOTOPOT_RAMP_STATE_FINISH = 4
}Motopot_ramp_state;

typedef enum {
	MOTOPOT_MOVE_ONE_SHOT   = 0,
	MOTOPOT_MOVE_CONTINOUS  = 1,
}Motopot_move_type;

typedef struct {
	Motopot_ramp_state state;
	Timer_t timer;
	uint16_t target_speed;
	uint16_t accel_time;
	uint16_t decel_time;
	uint32_t init_speed;
	int16_t full_position;
	int16_t stop_position;
}Motopot_ramp;

typedef struct{

	Pot_t* pot;
	Motor_t* motor;

	uint8_t serviceEnable;

	PID_t pid;

	Motopot_move_type moveType;

	Motopot_state state;
	Timer_t timer;
	Timer_t  speed_timer;

	uint32_t tolerance;

	uint32_t speed;
	uint32_t temp_position;

	Motopot_ramp ramp;

	 void (*Event_Idle_Callback)(void*);
	 uint8_t Event_Idle_Callback_Flag;

	 void (*Event_Start_Callback)(void*);
	 uint8_t Event_Start_Callback_Flag;

	 void (*Event_Stop_Callback)(void*);
	 uint8_t Event_Stop_Callback_Flag;

	 void (*Event_MovingByMotor_Callback)(void*);
	 uint8_t Event_MovingByMotor_Callback_Flag;

	 void (*Event_MovingByHand_Callback)(void*);
	 uint8_t Event_MovingByHand_Callback_Flag;

	 void (*Event_ForceStop_Callback)(void*);
	 uint8_t Event_ForceStop_Callback_Flag;

} Motopot_t;

void Motopot_Init(Motopot_t* motopot, Pot_t* pot, Motor_t* motor);
void Motopot_Service(Motopot_t* motopot);

Pot_t* Motopot_getPot(Motopot_t* motopot);
Motor_t Motopt_getMotor(Motopot_t* motopot);

void Motopot_setPosition(Motopot_t* motopot, uint16_t position);
void Motopot_setPositionToCenter();
void Motopot_MoveTo(Motopot_t* motopot, uint16_t position);

void Motopot_serviceEnable(Motopot_t* motopot, uint8_t enable);

#endif /* INC_MOTOPOT_H_ */
