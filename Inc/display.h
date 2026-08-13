/*
 * display.h
 *
 *  Created on: Feb 13, 2021
 *      Author: Dom
 */

#ifndef SVLIB_INC_DISPLAY_H_
#define SVLIB_INC_DISPLAY_H_

#include "global.h"
#include "buffer.h"

////////////// LCD //////////////

typedef enum {
	LCD_TYPE_8_2,
	LCD_TYPE_16_2,
	LCD_TYPE_16_4,
} LCD_Types;

typedef enum {
	LCD_MODE_STANDARD,
	LCD_MODE_I2C,
	LCD_MODE_I2C_DMA,
} LCD_Modes;

typedef struct {
	IOPin rs;
	IOPin rw;
	IOPin en;
	IOPin data_1;
	IOPin data_2;
	IOPin data_3;
	IOPin data_4;
} LCD_Standard_t;

typedef struct {
	I2C_HandleTypeDef* hi2c;
	uint16_t address;
} LCD_I2C_t;

typedef struct {

	union {
		LCD_Standard_t LCD_Standard;
		LCD_I2C_t LCD_I2C;
	};

	LCD_Types type;
	LCD_Modes mode;

	Buffer_t displayBuffer;
	Buffer_t TxData;

	uint8_t cursorPosition;
	uint8_t backgroudLit;
	uint8_t needsUpdate;


	uint8_t x;
	uint8_t y;

	uint8_t rows;
	uint8_t cols;
	uint8_t realRows;
	uint8_t realCols;

} LCD_t;


void LCD_I2C_Init(LCD_t* lcd, I2C_HandleTypeDef* hi2c, uint16_t address, LCD_Types type);
void LCD_Service(LCD_t* lcd);

void lcd_send_cmd (LCD_t* lcd, uint8_t cmd);
void lcd_send_data (LCD_t* lcd, uint8_t data);

void lcd_home(LCD_t* lcd);
void lcd_set_cursor_position(LCD_t* lcd, uint8_t col, uint8_t rows);
void lcd_store_custom_character(LCD_t* lcd, uint8_t location, char *pattern);

void lcd_clear(LCD_t* lcd);
void lcd_clear_line(LCD_t* lcd, int row);

void lcd_write(LCD_t* lcd, uint8_t row, uint8_t col, char* srt);
void lcd_write_line(LCD_t* lcd, uint8_t row, char* str);
void lcd_write_string (LCD_t* lcd, char *str);
void lcd_write_char (LCD_t *lcd, char c);
void lcd_write_buffer (LCD_t *lcd, uint8_t *data, uint32_t size);

void lcd_horizontal_bar(LCD_t* lcd, uint8_t row, uint16_t value, uint16_t max_value);
void lcd_button_state(LCD_t* lcd, uint8_t id, _Bool state);

void lcd_bkg_on(LCD_t *lcd);
void lcd_bkg_off(LCD_t *lcd);

void lcd_refresh(LCD_t*lcd);

////////////// MENU //////////////

typedef struct menu_node_struct menu_node_t;

struct menu_node_struct{

	char* name;
	menu_node_t* next;
	menu_node_t* prev;
	menu_node_t* child;
	menu_node_t* parent;
	void (*menu_function)(void);

};

typedef struct {

	LCD_t* lcd;
	menu_node_t *currentPointer;
	menu_node_t *firstMenuNode;
	uint8_t menu_index;
	uint8_t lcd_row_pos;

} menu_t;

void menu_init(LCD_t*lcd, menu_t* menu, menu_node_t* firstMenuNode);
void menu_next(menu_t* menu);
void menu_prev(menu_t* menu);
void menu_enter(menu_t* menu);
void menu_back(menu_t* menu);
uint8_t menu_get_level(menu_node_t *menuNode);
uint8_t menu_get_index(menu_t* menu, menu_node_t *menuNode);
void menu_refresh(menu_t* menu);

#endif /* SVLIB_INC_DISPLAY_H_ */
