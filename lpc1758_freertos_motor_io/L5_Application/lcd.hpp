/*
 * lcd.hpp
 *
 *  Created on: Nov 26, 2016
 *      Author: prash
 */

#ifndef L5_APPLICATION_LCD_HPP_
#define L5_APPLICATION_LCD_HPP_

#include <inttypes.h>
typedef enum{
	FORM0,				// Home
	FORM1,				// Sensor
	FORM2				// Geo
}forms;

typedef enum {
	READ_OBJ = 0x00,
	WRITE_OBJ = 0x01,
	WRITE_STR = 0x02,
	WRITE_STRU = 0x03,
	WRITE_CONTRAST = 0x04,
	REPORT_OBJ = 0x05,
	REPORT_EVENT = 0x07
}lcdCommands;

#define LCD_FORM0_INDEX				0x00
#define LCD_FORM1_INDEX				0x01
#define LCD_FORM2_INDEX				0x02

// Form 1
// Object defintions
#define LCD_SPEED_METER_INDEX		0x00		// must be between 0-30
#define LCD_TANK_INDEX				0x00		// must be between 0-100
#define LCD_LEDDIGIT_INDEX			0x02
// String Definitions
//#define LCD_RPM_STRING_INDEX			0x00
#define LCD_FRONT_LEFT_SENSOR_INDEX		0x00
#define LCD_FRONT_CENTER_SENSOR_INDEX	0x01
#define LCD_FRONT_RIGHT_SENSOR_INDEX	0x02
#define LCD_LATITUDE_INDEX				0x06
#define LCD_LONGITUDE_INDEX				0x07
#define LCD_BEARING_INDEX				0x08
#define LCD_HEADING_INDEX				0x09
#define LCD_DISTANCE_INDEX				0x05
#define LCD_SPEED_INDEX					0x04
#define LCD_BACK_SENSOR_INDEX			0x03

#define LCD_OBJ_DIPSW         0
#define LCD_OBJ_KNOB          1
#define LCD_OBJ_ROCKERSW      2
#define LCD_OBJ_ROTARYSW      3
#define LCD_OBJ_SLIDER        4
#define LCD_OBJ_TRACKBAR      5
#define LCD_OBJ_WINBUTTON     6
#define LCD_OBJ_ANGULAR_METER 7
#define LCD_OBJ_COOL_GAUGE    8
#define LCD_OBJ_CUSTOM_DIGITS 9
#define LCD_OBJ_FORM          10
#define LCD_OBJ_GAUGE         11
#define LCD_OBJ_IMAGE         12
#define LCD_OBJ_KEYBOARD      13
#define LCD_OBJ_LED           14
#define LCD_OBJ_LED_DIGITS    15
#define LCD_OBJ_METER         16
#define LCD_OBJ_STRINGS       17
#define LCD_OBJ_THERMOMETER   18
#define LCD_OBJ_USER_LED      19
#define LCD_OBJ_VIDEO         20
#define LCD_OBJ_STATIC_TEXT   21
#define LCD_OBJ_SOUND         22
#define LCD_OBJ_TIMER         23
#define LCD_OBJ_SPECTRUM      24
#define LCD_OBJ_SCOPE         25
#define LCD_OBJ_TANK          26
#define LCD_OBJ_USERIMAGES    27
#define LCD_OBJ_PINOUTPUT     28
#define LCD_OBJ_PININPUT      29
#define LCD_OBJ_4DBUTTON      30
#define LCD_OBJ_ANIBUTTON     31
#define LCD_OBJ_COLORPICKER   32
#define LCD_OBJ_USERBUTTON    33


int lcdWriteObj (int object, int index, unsigned int data);
int lcdWriteStr (int index, char *string);
int lcdInit();




#endif /* L5_APPLICATION_LCD_HPP_ */
