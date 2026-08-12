/*
 * hui.c
 *
 *  Created on: 11 paź 2022
 *      Author: rober
 */


#include "hui.h"
#include "osc.h"
#include "tinyosc.h"
#include "stdio.h"

OSC_t* osc_ptr;

tosc_bundle bundle;


void Hui_Init_OSC(OSC_t* o){
	osc_ptr = o;
}

void Hui_Ping(){

	char address[100];
	sprintf(address, "%s/%s", osc_ptr->oscPrefix, HUI_PING_ADDRESS);
	osc_ptr->tx.elements += tosc_writeMessage((char*)osc_ptr->tx.data, sizeof(osc_ptr->tx.data), address, "i", 127);
	osc_send(); // czy potrzebne???? w communication cały czas się wysyła jeśli bufer wiekszy od 0;
}


void Hui_Port_On(uint8_t zone, uint8_t port){

	char address[100];

	tosc_writeBundle(&bundle, 0, (char*)osc_ptr->tx.data, osc_ptr->tx.size);

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, HUI_ZONE_ADDRESS);
	tosc_writeNextMessage(&bundle, address, "i", zone);

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, HUI_PORT_ADDRESS);
	tosc_writeNextMessage(&bundle, address, "i", 64 + port);

	osc_ptr->tx.elements = tosc_getBundleLength(&bundle); // if > 0 sending bundle
}

void Hui_Port_Off(uint8_t zone, uint8_t port){

	char address[100];

	tosc_writeBundle(&bundle, 0, (char*)osc_ptr->tx.data, osc_ptr->tx.size);

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, HUI_ZONE_ADDRESS);
	tosc_writeNextMessage(&bundle, address, "i", zone);

	sprintf(address, "%s/%s", osc_ptr->oscPrefix, HUI_PORT_ADDRESS);
	tosc_writeNextMessage(&bundle, address, "i", port);

	osc_ptr->tx.elements = tosc_getBundleLength(&bundle); // if > 0 sending bundle
}

void Hui_Write_Bundle_Test(){

	tosc_writeBundle(&bundle, 0, (char*)osc_ptr->tx.data, osc_ptr->tx.size);
	tosc_writeNextMessage(&bundle, "/test_1", "i", 123);
	tosc_writeNextMessage(&bundle, "/test_2", "i", 234);
	tosc_writeNextMessage(&bundle, "/test_3", "i", 345);
	osc_ptr->tx.elements = tosc_getBundleLength(&bundle);
}
