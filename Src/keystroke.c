/*
 * keystroke.c
 *
 *  Created on: 1 lis 2022
 *      Author: rober
 */

#include "keystroke.h"
#include "osc.h"


void keyPress(char* key){
	osc_send_str_int_message(KEY_ADDRESS, key, 1);
}

void keyRelease(char* key){
	osc_send_str_int_message(KEY_ADDRESS, key, 0);
}


void schortcutSend(char* sht){
	osc_send_str_message(SHORTCUT_ADDRESS, sht);
}
