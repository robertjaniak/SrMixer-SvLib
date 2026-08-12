/*
 * osc.c
 *
 *  Created on: Jul 7, 2022
 *      Author: rober
 */

#include "osc.h"
#include "stdio.h"
#include "ethernet.h"
#include "string.h"
#include "stdio.h"



OSC_t* osc_ptr;


#define MAX_OSC_ADDRESS_SIZE 50
char osc_address[MAX_OSC_ADDRESS_SIZE];

#define MAX_OSC_PREFIX_SIZE 30
char prefix[MAX_OSC_PREFIX_SIZE];

#define MAX_DEVICE_NAME_SIZE 16
char osc_device_name[MAX_DEVICE_NAME_SIZE];

void OSC_Init(OSC_t* osc, char* device_name, int32_t id){

	osc_ptr = osc;

	Buffer_erase(&osc->tx);
	Buffer_erase(&osc->rx);

	osc->oscPrefix = prefix;
	osc->deviceName = osc_device_name;
	osc->deviceId = id;
	osc->connectionState = 0;

	osc_setPrefix(device_name, id);

	osc->OSC_Event_MessageReceived_Callback = NULL;
	osc->OSC_Event_MessageReceived_Callback_Flag = 1;
}

void OSC_CallbackRegister(OSC_t* osc, int8_t(*CallbackPtr)(void*), OSC_Events event){

	switch (event){

	case OSC_EVENT_MESSAGE_RECEIVED:
		osc->OSC_Event_MessageReceived_Callback = CallbackPtr;
		osc->OSC_Event_MessageReceived_Callback_Flag = 1;
		break;

	default:
		break;
	}
}

void osc_setPrefix(char* deviceName, int32_t id){
	strcpy(osc_ptr->deviceName, deviceName);
	osc_ptr->deviceId = id;
	osc_updatePrefix();
}

void osc_updatePrefix(){
	sprintf(osc_ptr->oscPrefix, "/%s/%li", osc_ptr->deviceName, osc_ptr->deviceId);
	//DBG_PRINT("OSC Prefix  \r\n");
}


void osc_read(){
	if(isLinked())
		udp_read(&osc_ptr->rx);
}

void osc_send(){
	if(isLinked())
		udp_send(&osc_ptr->tx);
}

void osc_read_message(){

	OSC_Message message;
	//char * address;

	tosc_parseMessage(&message, (char*)osc_ptr->rx.data, osc_ptr->rx.elements);

	//address = tosc_getAddress((tosc_message*)&message);
	//DBG_PRINT("\r\n OSC MESSAGE RECEIVED \r\n");
	//DBG_PRINT(address);
	//DBG_PRINT("\r\n");

	if (is_osc_prefix_correct(&message) == 1){

		if (osc_ptr->OSC_Event_MessageReceived_Callback_Flag == 1){
			if (osc_ptr->OSC_Event_MessageReceived_Callback != NULL){
				if((*osc_ptr->OSC_Event_MessageReceived_Callback)(&message) == 0){
					Buffer_clear(&osc_ptr->rx);
				}
			}
		}
	}
	else {
		DBG_PRINT("Wrong Prefix");
	}
};

void osc_read_bundle(){

	tosc_bundle bundle;
	tosc_message message;

	DBG_PRINT("OSC BUNDLE RECEIVED \r\n");

	tosc_parseBundle(&bundle, (char*)osc_ptr->rx.data, osc_ptr->rx.elements);

	while (tosc_getNextMessage(&bundle, &message)){

		DBG_PRINT(tosc_getAddress(&message))
		DBG_VALUE(tosc_getNextInt32(&message))

	}
};

void osc_service(){

	osc_read();

	if (osc_ptr->rx.elements > 0){

		switch ((int)tosc_isBundle((char*)osc_ptr->rx.data)) {

			case true:
				//osc_read_bundle();
				break;

			case false:
				osc_read_message();
				break;

			default:
				break;
		}
	}

	if (osc_ptr->tx.elements > 0){
		osc_send();
	}

}

char* osc_cut_prefix(OSC_Message* message, uint8_t num){

	char* address = tosc_getAddress(message);

	if (num > strlen(address)){
		num = strlen(address);
	}

	size_t i = 0;

	while ( *address != '\0' )
	{
		if (*address == '/') {
			i++;
			if (i == num + 1){
				break;
			}
		}
		++address;
	}
	return address;
}

size_t osc_get_prefix(OSC_Message* message, char* prefix, uint8_t num){

	char* address = tosc_getAddress(message);
	size_t size = osc_cut_prefix(message, num) - address;

	strncpy(prefix, address, size);

	return size;
}

uint8_t is_osc_prefix_correct(OSC_Message* message){


	char incommingOscPrefix[MAX_OSC_PREFIX_SIZE] = {0};

	osc_get_prefix(message, incommingOscPrefix, 2);

	if (strcmp(incommingOscPrefix, osc_ptr->oscPrefix) == 0){

		return 1;
	}

	osc_get_prefix(message, incommingOscPrefix, 1);

	if (strcmp(incommingOscPrefix, "/connect") == 0){

			return 1;
	}

	return 0;
}

void osc_send_str_message(char* command, char* msg){

	char address[MAX_OSC_ADDRESS_SIZE] = {0};

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, command);

	osc_ptr->tx.elements = tosc_writeMessage((char*)osc_ptr->tx.data, osc_ptr->tx.size, address, "s", msg);

	osc_send();
}

void osc_send_int_message(char* command, int32_t value){

	char address[MAX_OSC_ADDRESS_SIZE] = {0};

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, command);

	osc_ptr->tx.elements = tosc_writeMessage((char*)osc_ptr->tx.data, osc_ptr->tx.size, address, "i", value);

	osc_send();
}

void osc_send_str_int_message(char* command, char* key, int32_t value){

	char address[MAX_OSC_ADDRESS_SIZE] = {0};

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, command);

	osc_ptr->tx.elements = tosc_writeMessage((char*)osc_ptr->tx.data, osc_ptr->tx.size, address, "si", key, value);

	osc_send();
}



void osc_send_str_str_message(char* command, char* key, char* value){

	char address[MAX_OSC_ADDRESS_SIZE] = {0};

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, command);

	osc_ptr->tx.elements = tosc_writeMessage((char*)osc_ptr->tx.data, osc_ptr->tx.size, address, "ss", key, value);

	osc_send();
}

int32_t osc_getNextInt32(OSC_Message* msg){

	return tosc_getNextInt32(msg);

}
