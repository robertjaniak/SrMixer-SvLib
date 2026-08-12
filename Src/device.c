/*
 * device.c
 *
 *  Created on: Jul 30, 2023
 *      Author: rober
 */

#include "device.h"


void Device_Init(Device_t* device, char* name, uint32_t id){

	//device->name = name;

	strcpy(device->name, name);

	device->id = id;
	device->state = DEVICE_STATE_IDLE;
	device->dataStatus = NO_DATA_TO_SEND;
	device->communicationActive = 0;
	device->autosaveEnable = 0;
	device->communicationIndicatorEnable = 0;
	device->communicationIndicatorState = 3;
	device->updateScreen = 0;

}

const char* getDeviceName(Device_t* device){
	return device->name;
}

uint32_t getDeviceID(Device_t* device){
	return device->id;
}

void setDeviceID(Device_t* device, uint32_t number){
	device->id = number;
}

void setDeviceName(Device_t* device, char* name){
	strcpy(device->name, name);
}

DeviceState getDeviceState(Device_t* device){
	return device->state;
}

DeviceTestType getDeviceTest(Device_t* device){
	return device->test;
}

void setDeviceState(Device_t* device, DeviceState state){
	device->state = state;
}

void setDeviceTest(Device_t* device, DeviceTestType testType){
	device->test = testType;
}

void setDeviceAutosave(Device_t* device, uint8_t value){
	device->autosaveEnable = value;
}
