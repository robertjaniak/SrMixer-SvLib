/*
 * device.h
 *
 *  Created on: Jul 30, 2023
 *      Author: rober
 */

#ifndef SVLIB_INC_DEVICE_H_
#define SVLIB_INC_DEVICE_H_


#include "main.h"
#include "global.h"

typedef enum {
	DEVICE_STATE_IDLE,
	DEVICE_STATE_INIT,
	DEVICE_STATE_NORMAL,
	DEVICE_STATE_ERROR,
	DEVICE_STATE_TEST,
	DEVICE_STATE_SETUP,
	DEVICE_STATE_MUTE,
	DEVICE_STATE_SOLO,
}DeviceState;

typedef enum {
	TEST_OFF,
	TEST_SHIFTS,
	TEST_VEGAS,
	TEST_MOTORS_ALL,
	TEST_MOTORS_SEPARATE,
	TEST_BUTTONS,
	TEST_POTS,
	TEST_FADER,
	TEST_LCD,
	TEST_COMMUNICATION,

}DeviceTestType;

typedef enum {

	NO_DATA_TO_SEND,
	NEW_DATA_READY_TO_SEND,

}DataStatus;


typedef struct {

	uint32_t id;
	char name[16];
	DeviceState state;
	DeviceTestType test;
	uint8_t needUpdateSettings;
	DataStatus dataStatus;
	uint32_t settings;
	uint8_t communicationActive;
	uint8_t autosaveEnable;
	uint8_t communicationIndicatorEnable;
	uint8_t communicationIndicatorState;
	uint8_t potValueOnLcdEnable;
	uint8_t showStandardScreen;
	uint8_t showWorkingIndicator;
	uint32_t pingCount;
	uint8_t updateScreen;

} Device_t;

void Device_Init(Device_t* device, char* name, uint32_t id);

void setDeviceID(Device_t* device, uint32_t number);
void setDeviceName(Device_t* device, char* name);
const char* getDeviceName(Device_t* device);
uint32_t getDeviceID(Device_t* device);


DeviceState getDeviceState(Device_t* device);
DeviceTestType getDeviceTest(Device_t* device);


void setDeviceState(Device_t* device, DeviceState state);
void setDeviceTest(Device_t* device, DeviceTestType testType);
void setDeviceAutosave(Device_t* device, uint8_t value);

#endif /* SVLIB_INC_DEVICE_H_ */
