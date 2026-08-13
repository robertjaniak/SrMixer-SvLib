/*
 * sv_debug.c
 *
 *  Created on: 1 gru 2022
 *      Author: rober
 */

#include <debug.h>

void workingTime_Init(WorkingTime_t* wt, char* name){
	wt->name = name;
	wt->last = 0;
	wt->now = 0;
	wt->period = 0;
};
void workingTime_Start(WorkingTime_t* wt){
	wt->last = HAL_GetTick();
}
void workingTime_Print(WorkingTime_t* wt){
	char str[30];
	wt->now = HAL_GetTick();
	wt->period = wt->now - wt->last;
	sprintf(str,"WORKIN TIME [%s] :  %li \r\n", wt->name, wt->period);

	//DBG_PRINT(str);
};
