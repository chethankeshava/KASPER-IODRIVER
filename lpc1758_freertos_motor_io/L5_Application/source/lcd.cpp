/*
 * lcd.cpp
 *
 *  Created on: Nov 26, 2016
 *      Author: prash
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "lcd.hpp"
#include "LPC17xx.h"
#include "utilities.h"
#include "uart2.hpp"


char buffer[]={0x01,0x07,0x00,0x00,0x24,0x22};			// Set Angularmeter Value 36
char buffer1[]={0x02,0x00,0x09,0x33,0x37,0x2E,0x32,0x31,0x33,0x32,0x31,0x33,0x21};	//Sent latitude 37.213213

Uart2& u2 = Uart2::getInstance();

int lcdInit()
{
	u2.init(9600); /* Init baud rate */
	return 0;
}

int lcdWriteObj (int object, int index, unsigned int data)
{
	unsigned int checksum, msb, lsb ;

	lsb = (data >> 0) & 0xFF ;
	msb = (data >> 8) & 0xFF ;

	u2.putChar (WRITE_OBJ) ;
	checksum  = WRITE_OBJ ;
	u2.putChar (object) ;
	checksum ^= object ;
	u2.putChar (index) ;
	checksum ^= index ;
	u2.putChar (msb) ;
	checksum ^= msb ;
	u2.putChar (lsb) ;
	checksum ^= lsb ;
	u2.putChar (checksum) ;

	return 0 ;
}

int lcdWriteStr (int index, char *string)
{
	char *p ;
	unsigned int checksum ;

	int len = strlen (string) ;

	if (len > 255)
	return -1 ;

	u2.putChar(WRITE_STR) ;
	checksum  = WRITE_STR ;
	u2.putChar(index) ;
	checksum ^= index ;
	u2.putChar((unsigned char)len) ;
	checksum ^= len ;

	for (p = string ; *p ; ++p)
	{
		u2.putChar(*p) ;
		checksum ^= *p ;
	}

	u2.putChar(checksum) ;

	delay_ms(5);

	return 0 ;
}




