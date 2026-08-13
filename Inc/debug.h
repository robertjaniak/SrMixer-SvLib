/*
 * debug_defines.h
 *
 *  Created on: Jul 7, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_DEBUG_DEF_H_
#define SVLIB_INC_DEBUG_DEF_H_

//#include "usart.h"
#include "global.h"
#include "string.h"
#include "stdio.h"

//#define DEBUD_MESSAGE_SIZE 64 - should be declared in file where debug is used
//char debugMessage[DEBUG_MESSAGE_SIZE]; - should be declared in file where debug is used

//#define FIRMWARE_VERSION __DATE__
#define FIRMWARE_VERSION CURRENT_DATE
#define FIRMWARE_CONFIG CURRENT_CONFIG

#define PRINT_MSG(msg) //HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
#define PRINT_STR(msg) PRINT_MSG(msg)

#define PRINT_MSG_STR(msg, str) { 			\
  sprintf(debugMessage, msg, str);		\
  PRINT_MSG(messageBuffer)						\
}

#define PRINT_MSG_INT(msg, val) { 			\
  sprintf(debugMessage, msg, val);		\
  PRINT_MSG(messageBuffer)						\
}

#define VALUE_MSG "value: %ld \r\n"
#define PRINT_VALUE(value) { 				\
  sprintf(debugMessage, VALUE_MSG, (uint32_t)value);		\
  PRINT_MSG(messageBuffer)						\
}

#define ERROR_MSG "ERROR !!! : %d \r\n"
#define PRINT_ERROR(error) { 				\
  sprintf(debugMessage, ERROR_MSG, error);		\
  PRINT_MSG(messageBuffer)						\
}

#ifdef DEBUG
	#define DBG_PRINT(msg)    PRINT_MSG(msg);
	#define DBG_PRINT_STR(msg, str) PRINT_MSG_STR(msg, str)
	#define DBG_PRINT_INT(msg, val) PRINT_MSG_INT(msg, val)
	#define DBG_VALUE(value)  PRINT_VALUE(value);
	#define DBG_ERROR(error)  PRINT_ERROR(error);
#else
	#define DBG_PRINT(msg)
	#define DBG_PRINT_STR(msg, str)
	#define DBG_PRINT_INT(msg, val)
	#define DBG_VALUE(value)
	#define DBG_ERROR(error)
#endif

typedef struct {
	char* name;
	uint32_t last;
	uint32_t now;
	uint32_t period;
} WorkingTime_t;

void workingTime_Init(WorkingTime_t* wt, char* name);
void workingTime_Start(WorkingTime_t* wt);
void workingTime_Print(WorkingTime_t* wt);

#endif /* SVLIB_INC_DEBUG_DEF_H_ */
