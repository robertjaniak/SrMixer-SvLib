/*
 * keystroke.h
 *
 *  Created on: 1 lis 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_KEYSTROKE_H_
#define SVLIB_INC_KEYSTROKE_H_


#define SHORTCUT_ADDRESS "Shortcut"
#define KEY_ADDRESS "Key"

#define KEY_CTRL "Ctrl"
#define KEY_ALT "Alt"
#define KEY_SHIFT "Shift"
#define KEY_CMD "Cmd"
#define KEY_ENTER "Enter"

#define SHT_NEW "new"
#define SHT_OPEN "open"
#define SHT_CLOSE "close"
#define SHT_SAVE "save"
#define SHT_MUTE_REGION "muteRegion"
#define SHT_IMPORT_AUDIO "importAudio"
#define SHT_IMPORT_DATA "importData"
#define SHT_FADE "fade"
#define SHT_REDO "redo"
#define SHT_LOC_1 "loc_1"
#define SHT_LOC_2 "loc_2"
#define SHT_LOC_3 "loc_3"
#define SHT_LOC_4 "loc_4"
#define SHT_LOC_5 "loc_5"
#define SHT_LOC_6 "loc_6"
#define SHT_LOC_7 "loc_7"
#define SHT_LOC_8 "loc_8"
#define SHT_ZOOM_VERT_IN "zoomVertIn"
#define SHT_ZOOM_VERT_OUT "zoomVertOut"
#define SHT_ZOOM_HOR_IN "zoomHorIn"
#define SHT_ZOOM_HOR_OUT "zoomHorOut"
#define SHT_BACKSPACE "backspace"


void keyPress(char* key);
void keyRelease(char* key);
void schortcutSend(char* command);

#endif /* SVLIB_INC_KEYSTROKE_H_ */
