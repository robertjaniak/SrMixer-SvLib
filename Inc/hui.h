/*
 * hui.h
 *
 *  Created on: 11 paź 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_HUI_H_
#define SVLIB_INC_HUI_H_

#include "global.h"
#include "osc.h"

#define HUI_PING_ADDRESS "NoteOn/1/0"
#define HUI_ZONE_ADDRESS "ControlChange/1/15" // 12 from host to device
#define HUI_PORT_ADDRESS "ControlChange/1/47" // 44 from host to device



//ZONE 00 - channel strip 1
//ZONE 01 - channel strip 2
//ZONE 02 - channel strip 3
//ZONE 03 - channel strip 4
//ZONE 04 - channel strip 5
//ZONE 05 - channel strip 6
//ZONE 06 - channel strip 7
//ZONE 07 - channel strip 8

//ZONE 08 - keyboard shortcuts

#define HUI_CTRL_ZONE 0x08
#define HUI_CTRL_PORT 0x00

#define HUI_SHIFT_ZONE 0x08
#define HUI_SHIFT_PORT 0x01

#define HUI_EDIT_MODE_ZONE 0x08
#define HUI_EDIT_MODE_PORT 0x02

#define HUI_UNDO_ZONE 0x08
#define HUI_UNDO_PORT 0x03

#define HUI_ALT_ZONE 0x08
#define HUI_ALT_PORT 0x04

#define HUI_OPTION_ZONE 0x08
#define HUI_OPTION_PORT 0x05

#define HUI_EDIT_TOOL_ZONE 0x08
#define HUI_EDIT_TOOL_PORT 0x05

#define HUI_SAVE_ZONE 0x08
#define HUI_SAVE_PORT 0x08

//ZONE 09 - windows

#define HUI_MIX_ZONE 0x09
#define HUI_MIX_PORT 0x00

#define HUI_EDIT_ZONE 0x09
#define HUI_EDIT_PORT 0x01

//ZONE 0A - channel selection
//ZONE 0B - assignment 1
//ZONE 0C - assignment 2
//ZONE 0D - cursor movement/mode/scrub/shuttle


//ZONE 0E - transporter main (big switches)

#define HUI_TALKBACK_ZONE 0x0E
#define HUI_TALKBACK_PORT 0x00

#define HUI_REWIND_ZONE 0x0E
#define HUI_REWIND_PORT 0x01

#define HUI_FORWARD_ZONE 0x0E
#define HUI_FORWARD_PORT 0x02

#define HUI_STOP_ZONE 0x0E
#define HUI_STOP_PORT 0x03

#define HUI_PLAY_ZONE 0x0E
#define HUI_PLAY_PORT 0x04

#define HUI_REC_ZONE 0x0E
#define HUI_REC_PORT 0x05


//ZONE 0F - transporter loop/rtz/end

#define HUI_RTZ_ZONE 0x0F
#define HUI_RTZ_PORT 0x00

//ZONE 10 - transporter punch
//ZONE 11 - monitor input
//ZONE 12 - monitor output
//ZONE 13 - num pad 1

#define HUI_NUM_0_ZONE 0x13
#define HUI_NUM_0_PORT 0x00

#define HUI_NUM_1_ZONE 0x13
#define HUI_NUM_1_PORT 0x01

#define HUI_NUM_4_ZONE 0x13
#define HUI_NUM_4_PORT 0x02

#define HUI_NUM_2_ZONE 0x13
#define HUI_NUM_2_PORT 0x03

#define HUI_NUM_5_ZONE 0x13
#define HUI_NUM_5_PORT 0x04

#define HUI_DOT_ZONE 0x13
#define HUI_DOT_PORT 0x05

#define HUI_NUM_3_ZONE 0x13
#define HUI_NUM_3_PORT 0x06

#define HUI_NUM_6_ZONE 0x13
#define HUI_NUM_6_PORT 0x07


//ZONE 14 - num pad 2

#define HUI_ENTER_ZONE 0x14
#define HUI_ENTER_PORT 0x00

#define HUI_PLUS_ZONE 0x14
#define HUI_PLUS_PORT 0x01

//ZONE 15 - num pad 3

#define HUI_NUM_7_ZONE 0x15
#define HUI_NUM_7_PORT 0x00

#define HUI_NUM_8_ZONE 0x15
#define HUI_NUM_8_PORT 0x01

#define HUI_NUM_9_ZONE 0x15
#define HUI_NUM_9_PORT 0x02

#define HUI_MINUS_ZONE 0x15
#define HUI_MINUS_PORT 0x03

//ZONE 16 - timecode leds (no associated buttons)
//ZONE 17 - auto enable
//ZONE 18 - auto mode
//ZONE 19 - status/group

//ZONE 1A - edit

#define HUI_PASTE_ZONE 0x1A
#define HUI_PASTE_PORT 0x00

#define HUI_CUT_ZONE 0x1A
#define HUI_CUT_PORT 0x01

#define HUI_COPY_ZONE 0x1A
#define HUI_COPY_PORT 0x04

#define HUI_SEPARATE_ZONE 0x1A
#define HUI_SEPARATE_PORT 0x05

//ZONE 1B - function keys
//ZONE 1C - parameter edit
//ZONE 1D - click/beep/relay/footswitch (no associated buttons or leds)


#define HUI_CLICK_ZONE 0x1D
#define HUI_CLICK_PORT 0x02

#define HUI_BEEP_ZONE 0x1D
#define HUI_BEEP_PORT 0x03


void Hui_Init_OSC(OSC_t* o);

void Hui_Ping();
void Hui_Port_On(uint8_t zone, uint8_t port);
void Hui_Port_Off(uint8_t zone, uint8_t port);

void Hui_Write_Bundle_Test();




#endif /* SVLIB_INC_HUI_H_ */
