/*
 * osc.h
 *
 *  Created on: Jul 7, 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_OSC_H_
#define SVLIB_INC_OSC_H_

#include "SvLib.h"
#include "tinyosc.h"
#include "buffer.h"

#define OSC_BUFFER_SIZE 256

typedef tosc_message OSC_Message;
typedef tosc_bundle OSC_Bundle;

//typedef struct {
//	uint8_t data[OSC_BUFFER_SIZE];
//	uint32_t len;
//} Buffer_t;

typedef enum {
	OSC_EVENT_IDLE             = 0,
	OSC_EVENT_MESSAGE_RECEIVED = 1,
} OSC_Events;

typedef struct {
  Buffer_t tx;
  Buffer_t rx;
  char* deviceName;
  int32_t deviceId;
  char* oscPrefix;
  int8_t connectionState;

   int8_t (*OSC_Event_MessageReceived_Callback)(void*);
  _Bool OSC_Event_MessageReceived_Callback_Flag;

} OSC_t;

void OSC_Init(OSC_t* osc, char* pref, int32_t id);
void OSC_CallbackRegister(OSC_t* osc, int8_t(*CallbackPtr)(void*), OSC_Events event);

void osc_send_str_message(char* command, char* msg);
void osc_send_int_message(char* comman, int32_t value);
void osc_send_str_int_message(char* comman, char* msg, int32_t value);
void osc_send_str_str_message(char* comman, char* msg1, char* msg2);

void osc_read();
void osc_send();

void osc_read_message();
void osc_read_bundle();

void osc_service();

void osc_setPrefix(char* deviceName, int32_t id);
void osc_updatePrefix();

char * osc_cut_prefix(OSC_Message* message, uint8_t num);
size_t osc_get_prefix(OSC_Message* message, char* prefix, uint8_t num);

int32_t osc_getNextInt32(OSC_Message* msg);

uint8_t is_osc_prefix_correct(OSC_Message* message);

#endif /* SVLIB_INC_OSC_H_ */

