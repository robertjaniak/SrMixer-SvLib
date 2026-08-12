/*
 * pot.h
 *
 *  Created on: 10 lip 2022
 *      Author: rober
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

//#define POT_TURN_TOLERANCE 0 // for Turn detect
//#define POT_IDLE_TOLERANCE 1 // for Idle detect
//#define POT_DEFAULT_REFRESH_TIME  100
//
//#define FILTER_BUFFER_SIZE 64
//
//#define POT_SPEED_ARRAY_SIZE   50
//
//#define POT_ACTIVE_TIMER_TIME  10
//#define POT_SPEED_DETECT_TIME 100
//#define POT_IDLE_TIMER_TIME  1000
//
//#define POT_DEFAULT_MAX_VALUE 255
//#define POT_DEFAULT_MIN_VALUE 0
//
//typedef enum {
//	POT_POLARITY_POSITIVE = 0,
//	POT_POLARITY_NEGATIVE = 1,
//} Pot_Polarity;
//
//typedef struct {
//	uint32_t data[POT_SPEED_ARRAY_SIZE];
//	uint32_t count;
//	uint32_t sum;
//} Pot_Speed_Buffer;
//
//typedef enum {
//	POT_TYPE_LINEAR,
//	POT_TYPE_LOGARYTMIC,
//}Pot_type;
//
//typedef enum {
//	POT_STATE_DISABLE     = 0U,
//	POT_STATE_IDLE,
//	POT_STATE_STOP,
//	POT_STATE_TURN_LEFT,
//	POT_STATE_TURN_RIGHT,
//}Pot_state;
//
//typedef enum {
//	POT_NOT_IN_PLACE      = 0,
//	POT_IN_PLACE          = 1,
//}Pot_position_state;
//
//typedef enum {
//	POT_EVENT_IDLE        = 0,
//	POT_EVENT_STOP        = 1,
//	POT_EVENT_TURN        = 2,
//	POT_EVENT_TURN_RIGHT  = 3,
//	POT_EVENT_TURN_LEFT   = 4,
//	POT_EVENT_IN_PACE     = 5,
//	POT_EVENT_NOT_IN_PLACE = 6,
//}Pot_event;
//
//typedef struct {
//
//	uint32_t id;
//	const char* name;
//	uint32_t* adcBuffer;
//	uint32_t filterBuffer[FILTER_BUFFER_SIZE];
//	Filter_t averageFilter;
//	Pot_type type;
//	Pot_state state;
//	Pot_Polarity polarity;
//
//    volatile int32_t currentPosition;
//    int32_t lastPosition;
//    int32_t desiredPosition;
//    int32_t tempPosition;
//    int32_t totalDistance;
//
//    Pot_position_state positionState;
//
//	uint32_t minValue;
//	uint32_t maxValue;
//	uint32_t tolerance;
//	uint32_t refreshRate;
//	int32_t speed;
//	Pot_Speed_Buffer speedBuffer;
//
//	Timer_t activeTimer;
//	Timer_t idleTimer;
//	Timer_t speedTimer;
//
////	_Bool updateNeeded;
////	_Bool potInPlace;
//
//	_Bool saved;
//	_Bool toSend;
//	int32_t lastSentValue;
//
//	void (*IdleCallback)(void*);
//	_Bool IdleCallbackFlag;
//
//	void (*TurnRightCallback)(void*);
//	_Bool TurnRightCallbackFlag;
//
//	void (*TurnLeftCallback)(void*);
//	_Bool TurnLeftCallbackFlag;
//
//	void (*StopCallback)(void*);
//	_Bool StopCallbackFlag;
//
//	void (*TurnCallback)(void*);
//	_Bool TurnCallbackFlag;
//
//	void (*InPlaceCallback)(void*);
//	_Bool InPlaceCallbackEnable;
//
//	void (*NotInPlaceCallback)(void*);
//	_Bool NotInPlaceCallbackEnable;
//
//} Pot_t;
//
//

void Encoder_Init();
void Encoder_Service();

//void Pot_Init(Pot_t* pot, const char* name, Pot_type type, uint32_t adcBuffer[],  uint32_t id, Pot_Polarity polarity, int32_t tolerance, int32_t position);
//void Pot_Service(Pot_t* pot);
//
//char* Pot_getName(Pot_t* pot);
//
//void Pot_setMaxValue(Pot_t* pot, uint32_t value);
//void Pot_setMinValue(Pot_t* pot, uint32_t value);
//
//void Pot_setCurrentPosition(Pot_t* pot, int32_t position);
//int32_t Pot_getCurrentPosition(Pot_t* pot);
//int32_t Pot_getLastPosition(Pot_t* pot);
//int32_t Pot_getTotalDistance(Pot_t* pot);
//int32_t Pot_getSpeed(Pot_t* pot);
//Pot_state Pot_getState(Pot_t* pot);
//Pot_position_state Pot_getPositionState(Pot_t* pot);
//int32_t Pot_getMaxValue(Pot_t* pot);
//int32_t Pot_getMinValue(Pot_t* pot);
//
//void Pot_disable(Pot_t* pot);
//void Pot_enable(Pot_t* pot);
//
//
//void Pot_setDesiredPosition(Pot_t* pot, uint32_t position);
//void Pot_callbackRegister(Pot_t* pot, void (*CallbackPtr)(void*), Pot_event event);
//
//void positionAverage(Pot_t* pot);

#endif /* INC_ENCODER_H_ */
