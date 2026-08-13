/*
 * display.c
 *
 *  Created on: Feb 13, 2021
 *      Author: Dom
 */

#include "display.h"
#include "i2c.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "string.h"

#define BKG 0b0001000
#define EN  0b00000100
#define RW  0b00000000
#define RS  0b00000001

#define HI2C_DEF hi2c1
#define LCD_ADDRESS 0x4E //PCF8574 address

#define INIT_4_BIT_MODE 0x02
#define INIT_8_BIT_MODE 0x03

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME   0x02

#define FUNCTION_SET 0x20
	#define TWO_LINES 0x08

#define DISPLAY_CONTROL  0x08
	#define DISPLAY_OFF  0x00
	#define DISPLAY_ON   0x04
	#define CURSOR_ON    0x02
	#define CURSOR_BLINK 0x01

#define ENTRY_MODE 0x04
	#define INCREMENT_CURSOR 0x2
	#define SHIFT_DISPLAY 	 0x1

#define UNDERLINE_OFF_BLINK_OFF 0x0C
#define UNDERLINE_OFF_BLINK_ON  0x0D
#define UNDERLINE_ON_BLINK_OFF  0x0E
#define UNDERLINE_ON_BLINK_ON   0x0F

#define SET_DDRAM_ADDRESS 0x80

#define FIRST_CHAR_LINE_1 0x80
#define FIRST_CHAR_LINE_2 0xC0

#define SET_CGRAM_ADDRES 0x40

#define DISPLAY_BUFFER_SIZE 80
#define DISPLAY_TX_DATA_SIZE (DISPLAY_BUFFER_SIZE * 4)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayTxData[DISPLAY_TX_DATA_SIZE];

void set_backlit_state(_Bool state);

static void lcd_init (LCD_t* lcd)
{
	// 4-bit initialization
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd (lcd, 0x03);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd (lcd, 0x03);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd (lcd, 0x03);
	HAL_Delay(10);
	lcd_send_cmd (lcd, INIT_4_BIT_MODE);
	HAL_Delay(10);

	// display initialization
	lcd_send_cmd (lcd, FUNCTION_SET | TWO_LINES);
	HAL_Delay(1);
	lcd_send_cmd (lcd, DISPLAY_CONTROL | DISPLAY_OFF);
	HAL_Delay(1);
	lcd_send_cmd (lcd, CLEAR_DISPLAY);
	HAL_Delay(2);
	lcd_send_cmd (lcd, ENTRY_MODE | INCREMENT_CURSOR);
	HAL_Delay(1);
	lcd_send_cmd (lcd, DISPLAY_CONTROL | DISPLAY_ON);
}


// prepare TxBuffer for 4 pin LCD. split data at 4 byte and store its to separate byte
static void writeByteToTxBuffer(LCD_t* lcd, uint8_t data){

	uint8_t data_u, data_l;
	uint8_t data_t[4] = {0};

	data_u = (data & 0xF0);
	data_l = ((data << 4) & 0xF0);

	if (lcd->backgroudLit) {
		data_u |= BKG;
		data_l |= BKG;
	}

	data_t[0] = data_u|RS|EN;
	data_t[1] = data_u|RS;
	data_t[2] = data_l|RS|EN;
	data_t[3] = data_l|RS;

	for(uint8_t i = 0; i < 4; i++){
		Buffer_write_u8(&lcd->TxData, data_t[i]);
	}

	lcd->needsUpdate = 1;
}

void refresh_TxBuffer(LCD_t *lcd){

	Buffer_setLocation(&lcd->TxData, 0);
	for(uint16_t i = 0; i < DISPLAY_BUFFER_SIZE; i++){
		writeByteToTxBuffer(lcd, displayBuffer[i]);
	}
}

void lcd_buf_clear(LCD_t *lcd){
	Buffer_fill(&lcd->displayBuffer, ' ');
	refresh_TxBuffer(lcd);
}

void lcd_buf_copy(LCD_t *lcd, uint8_t *data, uint32_t size) {

	memcpy(displayBuffer, data, lcd->realCols);
	memcpy(displayBuffer + 40, data + lcd->realCols, lcd->realCols);

	refresh_TxBuffer(lcd);
}

void LCD_I2C_Init(LCD_t* lcd, I2C_HandleTypeDef* _hi2c, uint16_t _address, LCD_Types _type){

	lcd->LCD_I2C.hi2c = _hi2c;
	lcd->LCD_I2C.address = _address << 1;
	lcd->type = _type;
	lcd->mode = LCD_MODE_I2C_DMA;

	lcd->cursorPosition = 0;
	lcd->backgroudLit = ON;
	lcd->needsUpdate = 0;

	Buffer_init(&lcd->displayBuffer, displayBuffer, DISPLAY_BUFFER_SIZE, BUFFER_TYPE_STANDARD);
	Buffer_init(&lcd->TxData, displayTxData, DISPLAY_TX_DATA_SIZE, BUFFER_TYPE_STANDARD);

	//lcd_buf_clear(lcd);

	switch (lcd->type) {

		case LCD_TYPE_8_2:
			lcd->cols = 40;
			lcd->rows = 2;
			lcd->realCols = 8;
			lcd->realRows = 2;
			break;

		case LCD_TYPE_16_2:
			lcd->cols = 40;
			lcd->rows = 2;
			lcd->realCols = 16;
			lcd->realRows = 2;

			break;
		default:
			break;
	}

	lcd_init(lcd);
}

void static sendToLcdByGPIO (LCD_t* lcd, char _data, int _rs){

	HAL_GPIO_WritePin(lcd->LCD_Standard.rs.port, lcd->LCD_Standard.rs.pin, _rs);  // rs = 1 for data, rs=0 for command

	/* write the data to the respective PIN */
	HAL_GPIO_WritePin(lcd->LCD_Standard.data_4.port, lcd->LCD_Standard.data_4.pin, ((_data>>7)&0x01));  //D7
	HAL_GPIO_WritePin(lcd->LCD_Standard.data_3.port, lcd->LCD_Standard.data_3.pin, ((_data>>6)&0x01));  //D6
	HAL_GPIO_WritePin(lcd->LCD_Standard.data_2.port, lcd->LCD_Standard.data_2.pin, ((_data>>5)&0x01));  //D5
	HAL_GPIO_WritePin(lcd->LCD_Standard.data_1.port, lcd->LCD_Standard.data_1.pin, ((_data>>4)&0x01));  //D4

	/* Toggle EN PIN to send the data
	 * if the HCLK > 100 MHz, use the  20 us delay
	 * if the LCD still doesn't work, increase the delay to 50, 80 or 100..
	 */
	HAL_GPIO_WritePin(lcd->LCD_Standard.en.port, lcd->LCD_Standard.en.pin, 1);
	HAL_GPIO_WritePin(lcd->LCD_Standard.en.port, lcd->LCD_Standard.en.pin, 0);

}

void static sendToLcdByI2C(LCD_t* lcd, uint8_t* data, uint16_t size){

	HAL_I2C_Master_Transmit(lcd->LCD_I2C.hi2c, lcd->LCD_I2C.address, data, size, 1);
}


void lcd_send_cmd (LCD_t* lcd, uint8_t cmd)
{
	uint8_t data_u, data_l;
	uint8_t data_t[4] = {0};

	data_u = (cmd & 0xF0);
	data_l = ((cmd << 4) & 0xF0);

	switch (lcd->mode){
		case LCD_MODE_STANDARD:
			sendToLcdByGPIO(lcd, data_u, 0); //send upper nibble, RS=0 for sending command
			sendToLcdByGPIO(lcd, data_l, 0); //send lower nibble, RS=0 for sending command
			break;

		case LCD_MODE_I2C:
		case LCD_MODE_I2C_DMA:
			if (lcd->backgroudLit) {
				data_u |= BKG;
				data_l |= BKG;
			}
			data_t[0] = data_u|EN;
			data_t[1] = data_u;
			data_t[2] = data_l|EN;
			data_t[3] = data_l;
			sendToLcdByI2C(lcd, (uint8_t *)data_t, 4);
			break;
	}
}

void lcd_send_data (LCD_t* lcd, uint8_t data)
{
	uint8_t data_u, data_l;
	uint8_t data_t[4] = {0};

	data_u = (data&0xF0);
	data_l = ((data<<4)&0xF0);

	switch (lcd->mode){
		case LCD_MODE_STANDARD:
			sendToLcdByGPIO(lcd, data_u, 1); //send upper nibble, RS=1 for sending data
			sendToLcdByGPIO(lcd, data_l, 1); //send lower nibble, RS=1 for sending data
			break;

		case LCD_MODE_I2C:
		case LCD_MODE_I2C_DMA:
			if (lcd->backgroudLit) {
				data_u |= BKG;
				data_l |= BKG;
			}
			data_t[0] = data_u|RS|EN;
			data_t[1] = data_u|RS;
			data_t[2] = data_l|RS|EN;
			data_t[3] = data_l|RS;
			sendToLcdByI2C(lcd, (uint8_t *)data_t, 4);
			break;
	}
}

void lcd_home(LCD_t* lcd){
	lcd_send_cmd(lcd, RETURN_HOME);
	//HAL_Delay(2); // do not use in UART Callback or change SysTick priority
}

void lcd_clear(LCD_t* lcd){

	switch (lcd->mode){
		case LCD_MODE_STANDARD:
			lcd_send_cmd(lcd, CLEAR_DISPLAY);
			break;

		case LCD_MODE_I2C:
		case LCD_MODE_I2C_DMA:
			lcd_buf_clear(lcd);
			break;
	}

	//HAL_Delay(2); // do not use in UART Callback or change SysTick priority
}

void lcd_buf_clear_line(LCD_t *lcd, uint8_t line){

	uint8_t index;
	index = (line - 1) * lcd->cols;

	for (uint8_t i = 0; i < lcd->cols; i++){
		Buffer_setLocation(&lcd->displayBuffer,  index + i);
		Buffer_write_u8(&lcd->displayBuffer, ' ');

		Buffer_setLocation(&lcd->TxData, (index + i) * 4);
		writeByteToTxBuffer(lcd, ' ');
	}
}

void lcd_clear_line(LCD_t* lcd, int row){

	int i = lcd->realCols;

	switch (lcd->mode) {

		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:

			lcd_set_cursor_position(lcd, row, 1);
			while (i--) lcd_send_data(lcd, 0x20);
			break;

		case LCD_MODE_I2C_DMA:
			lcd_buf_clear_line(lcd, row);
			break;

		default:
			break;
	}


}

void lcd_buf_write_char(LCD_t *lcd, char c){

	uint8_t index;

	switch (lcd->type) {

		case LCD_TYPE_8_2:
		case LCD_TYPE_16_2:

			index = (lcd->cols * lcd->y ) + lcd->x;

			Buffer_setLocation(&lcd->displayBuffer, index);
			Buffer_write_u8(&lcd->displayBuffer, c);

			Buffer_setLocation(&lcd->TxData, index*4);
			writeByteToTxBuffer(lcd, c);

			lcd->x++;

			if (lcd->x >= lcd->cols){
				lcd->x = 0;
				lcd->y++;
			}

			if (lcd->y >= lcd->rows){
				lcd->y = 0;
			}

			break;
		default:
			break;
	}
}

void lcd_buf_write_str(LCD_t *lcd, char *str){
	while(*str) lcd_buf_write_char(lcd, *str++);
}

void lcd_buf_locate(LCD_t *lcd, uint8_t x, uint8_t y){

	lcd->x = x - 1;
	lcd->y = y - 1;
}

void lcd_buf_write(LCD_t* lcd, uint8_t x, uint8_t y, char* str){
	lcd_buf_locate(lcd, x, y);
	lcd_buf_write_str(lcd, str);
}

void lcd_write(LCD_t* lcd, uint8_t row, uint8_t col, char* str){

	switch (lcd->mode) {

		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:
			lcd_set_cursor_position(lcd, row, col);
			lcd_write_string(lcd, str);
			break;

		case LCD_MODE_I2C_DMA:
			lcd_buf_write(lcd, col, row, str);
			break;

		default:
			break;
	}
}

void lcd_write_line(LCD_t* lcd, uint8_t row, char* str){

	switch (lcd->mode) {

		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:
			lcd_clear_line(lcd,row);
			lcd_set_cursor_position(lcd,row, 1);
			lcd_write_string(lcd, str);
			break;

		case LCD_MODE_I2C_DMA:
			lcd_buf_clear_line(lcd, row);
			lcd_buf_locate(lcd, 1, row);
			lcd_buf_write_str(lcd, str);
			break;

		default:
			break;
	}
}

void lcd_write_string (LCD_t* lcd, char* str){

		switch (lcd->mode) {
			case LCD_MODE_STANDARD:
			case LCD_MODE_I2C:
				while (*str) lcd_send_data (lcd, *str++);
				break;

			case LCD_MODE_I2C_DMA:
				lcd_buf_write_str(lcd, str);
				break;
			default:
				break;
		}
}

void lcd_write_buffer (LCD_t *lcd, uint8_t *data, uint32_t size){

	switch (lcd->mode) {
		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:
			break;

		case LCD_MODE_I2C_DMA:
			lcd_buf_copy(lcd, data, size);
			break;
		default:
			break;
	}

}

void lcd_set_cursor_position(LCD_t* lcd, uint8_t row, uint8_t col){

	switch (row){
		case 1:
			col |= FIRST_CHAR_LINE_1;
			break;
		case 2:
			col |= FIRST_CHAR_LINE_2;
			break;
	}
	lcd->cursorPosition = col - 1;
	lcd_send_cmd (lcd, lcd->cursorPosition);

}

void lcd_store_custom_character(LCD_t* lcd, uint8_t location, char *pattern){
	lcd_send_cmd (lcd, SET_CGRAM_ADDRES + (location*8)); //Send the Address of CGRAM
	while (*pattern) lcd_send_data (lcd, *pattern++);
}

void lcd_horizontal_bar(LCD_t* lcd, uint8_t row, uint16_t value, uint16_t max_value){

	//#define MAX_VALUE 65535 // 4095 or 255 or 127

	const uint8_t bar_lenght = 13;
	uint16_t delta = round((max_value + 1) / bar_lenght);
	uint8_t bar = 0;
	uint8_t blank = bar_lenght;

	char digits[3];
	uint8_t digitValue = ((value * 100) / max_value); //scale value to percent (0-100)
	itoa(digitValue, digits, 10);
	lcd_set_cursor_position(lcd, row, 1);
	lcd_write_string(lcd, "   ");

	/* 3 digit value before bar */
	if (digitValue < 10)
		lcd_set_cursor_position(lcd, row, 3);
	else if ((digitValue >= 10) && (digitValue < 100 ))
		lcd_set_cursor_position(lcd, row, 2);
	else if (digitValue >= 100)
		lcd_set_cursor_position(lcd, row, 1);
		
	lcd_write_string(lcd, digits); // write digit value

	/* bar length calculation */
	if ((value >= 1) && (value < delta))
		bar = 1;
	else if ((value >= delta) && (value < max_value))
		bar = round(value / delta);
	else if (value >= max_value)
		bar = bar_lenght;
		
	blank = bar_lenght - bar;

	/* loop for draw bar */ 
	while (bar--) lcd_send_data (lcd, 0xFF); // sign for bar
	while (blank--) lcd_send_data (lcd, 0x20); // sign for blank
}



void lcd_button_state(LCD_t* lcd, uint8_t id, _Bool state){
	if (state)
		lcd_write(lcd, 1, id + 1, "X");
	else
		lcd_write(lcd, 1, id + 1, " ");
}


void set_backlit_state(_Bool state){ // set backlit state to DMA buffer

	for(uint16_t i = 0; i < DISPLAY_TX_DATA_SIZE; i++){

		displayTxData[i] = state ? setBit(displayTxData[i], 4) : clearBit(displayTxData[i], 4);

	}
}

void lcd_bkg_on(LCD_t *lcd){

	uint8_t data_t[1] = {0b00001000};

	switch (lcd->mode){
		case LCD_MODE_STANDARD:
			HAL_I2C_Master_Transmit(lcd->LCD_I2C.hi2c, lcd->LCD_I2C.address, (uint8_t *)data_t, 1, 20);
			lcd_refresh(lcd);
			break;

		case LCD_MODE_I2C:
		case LCD_MODE_I2C_DMA:
			lcd->backgroudLit = ON;
			set_backlit_state(true);
			lcd->needsUpdate = 1;
			break;
	}
}

void lcd_bkg_off(LCD_t *lcd){

	uint8_t data_t[1] = {0b00000000};

	switch (lcd->mode){
		case LCD_MODE_STANDARD:
			HAL_I2C_Master_Transmit(lcd->LCD_I2C.hi2c, lcd->LCD_I2C.address, (uint8_t *)data_t, 1, 20);
			lcd_refresh(lcd);
			break;

		case LCD_MODE_I2C:
		case LCD_MODE_I2C_DMA:
			lcd->backgroudLit = OFF;
			set_backlit_state(false);
			lcd->needsUpdate = 1;
			break;
	}
}


void lcd_write_char (LCD_t *lcd, char c){

	switch (lcd->mode) {
		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:
			break;

		case LCD_MODE_I2C_DMA:
			lcd_buf_write_char(lcd, c);
			break;
		default:
			break;
	}
}


void lcd_refresh(LCD_t *lcd){

	switch (lcd->mode) {
		case LCD_MODE_STANDARD:
		case LCD_MODE_I2C:
			for (uint8_t i = 0; i < DISPLAY_BUFFER_SIZE; i++){
				Buffer_setLocation(&lcd->displayBuffer, i + 1);
				lcd_send_data(lcd, (char)Buffer_read_u8(&lcd->displayBuffer));
				}
			break;

		case LCD_MODE_I2C_DMA:
			HAL_I2C_Master_Transmit_DMA(lcd->LCD_I2C.hi2c, lcd->LCD_I2C.address, displayTxData, DISPLAY_TX_DATA_SIZE);
			break;
		default:
			break;
	}

	lcd->needsUpdate = 0;

}



//////////// MENU ////////////

void menu_next(menu_t* menu) {

	if (menu->currentPointer->next != NULL)
	{
		menu->currentPointer = menu->currentPointer->next;
		menu->menu_index++;
		if (++menu->lcd_row_pos > menu->lcd->rows - 1){
			menu->lcd_row_pos = menu->lcd->rows - 1;
		}
	}
	else
	{
		menu->menu_index = 0;
		menu->lcd_row_pos = 0;

		if (menu->currentPointer->parent != NULL){
			menu->currentPointer = (menu->currentPointer->parent)->child;
		}
		else menu->currentPointer = menu->firstMenuNode;
	}

	menu_refresh(menu);

}

void menu_prev(menu_t* menu) {

	if (menu->currentPointer->prev != NULL){
		menu->currentPointer = menu->currentPointer->prev;
	}


	if (menu->menu_index)
	{
		menu->menu_index--;
		if (menu->lcd_row_pos > 0) menu->lcd_row_pos--;
	}
	else
	{
		menu->menu_index = menu_get_index(menu, menu->currentPointer);

		if (menu->menu_index >= menu->lcd->rows - 1) {
			menu->lcd_row_pos = menu->lcd->rows - 1;
		}
		else {
			menu->lcd_row_pos = menu->menu_index;
		}
	}

	menu_refresh(menu);
}

uint8_t menu_get_index(menu_t* menu, menu_node_t *menuNode) {

	menu_node_t *temp;
	uint8_t i = 0;

	if (menuNode->parent) {
		temp = (menuNode->parent)->child;
	}
	else {
		temp = menu->firstMenuNode;
	}

	while (temp != menuNode) {
		temp = temp->next;
		i++;
	}

	return i;
}

void menu_enter(menu_t* menu) {

	if (menu->currentPointer->menu_function) {
		menu->currentPointer->menu_function();
	}

		if (menu->currentPointer->child)
		{

			switch (menu_get_level(menu->currentPointer)) {
				case 0:
					//lcd_row_pos_level_1 = lcd_row_pos;
					break;

				case 1:
					//lcd_row_pos_level_2 = lcd_row_pos;
					break;
			}

			// switch...case can be replaced by:
			// lcd_row_pos_level[ menu_get_level(currentPointer) ] = lcd_row_pos;

			menu->menu_index = 0;
			menu->lcd_row_pos = 0;

			menu->currentPointer = menu->currentPointer->child;

			menu_refresh(menu);
		}
}

void menu_back(menu_t* menu) {

	if (menu->currentPointer->parent) {

		switch (menu_get_level(menu->currentPointer)) {
			case 1:
				//lcd_row_pos = lcd_row_pos_level_1;
				break;

			case 2:
				//lcd_row_pos = lcd_row_pos_level_2;
				break;
			}

		menu->currentPointer = menu->currentPointer->parent;
		menu->menu_index = menu_get_index(menu, menu->currentPointer);

		menu_refresh(menu);

	}
}

uint8_t menu_get_level(menu_node_t *menuNode) {

	menu_node_t *temp = menuNode;
	uint8_t i = 0;

	if (!menuNode->parent) {
		return 0;
	}

	while (temp->parent != NULL) {
		temp = temp->parent;
		i++;
	}

	return i;
}


static void menu_buffer_refresh(menu_t* menu){

	menu_node_t *temp;
	uint8_t i;

	if (menu->currentPointer->parent) {
		temp = (menu->currentPointer->parent)->child;
	}
	else {
		temp = menu->firstMenuNode;
	}

	for (uint8_t i = 0; i != menu->menu_index - menu->lcd_row_pos; i++) {
		temp = temp->next;
	}

	lcd_buf_clear(menu->lcd);

	for (i = 0; i < menu->lcd->rows; i++) {

		lcd_buf_locate(menu->lcd, 1, i+1);
		if (temp == menu->currentPointer){
			lcd_buf_write_char(menu->lcd, 62);
		}
		else {
			lcd_buf_write_char(menu->lcd, ' ');
		}

		lcd_buf_locate(menu->lcd, 3, i+1);
		lcd_buf_write_str(menu->lcd, temp->name);

		temp = temp->next;
		if (!temp) {
			break;
		}

	}
}

void menu_refresh(menu_t* menu) {
	menu_buffer_refresh(menu);
	lcd_refresh(menu->lcd);
}

void menu_init(LCD_t*lcd, menu_t* menu, menu_node_t* firstMenuNode){

	menu->lcd = lcd;
	menu->currentPointer = firstMenuNode;
	menu->firstMenuNode = firstMenuNode;
	menu->lcd_row_pos = 0;
	menu->menu_index = 0;

	menu_refresh(menu);
}

void LCD_Service(LCD_t* lcd){

	if(lcd->needsUpdate){
		lcd_refresh(lcd);
	}

}
