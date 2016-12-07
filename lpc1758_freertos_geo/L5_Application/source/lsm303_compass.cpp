/*
 * lsm303_compass.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: ankit
 */

#include <stdint.h>

#include "io.hpp" // All IO Class definitions
#include "lsm303_compass.hpp"
#include <stdio.h>

uint8_t angleXMsb = 0;
uint8_t angleXLsb = 0;
uint8_t angleYMsb = 0;
uint8_t angleYLsb = 0;
uint8_t angleZMsb = 0;
uint8_t angleZLsb = 0;
uint16_t X = 0;
uint16_t Y = 0;
uint16_t Z = 0;


float Accx;
float Accy;
float Accz;
int16_t Magx;
int16_t Magy;
int16_t Magz;
float Mag_minx;
float Mag_miny;
float Mag_minz;
float Mag_maxx;
float Mag_maxy;
float Mag_maxz;

bool lsm303_compass::init()
{

	bool devicePresent = checkDeviceResponse();

	if(devicePresent)
	{
		//const unsigned char datarate15Hz = (0x04 << 2 | 1<<7); 	// data output rate = 15Hz
		//const unsigned char gainConfig81 = 0x07 << 5;
		//const unsigned char continousConvert = 0;
		//const unsigned char accelEnable = 0x67;     			/* Normal mode 10Hz  */

		printf("Initializing LSM303 sensor\n");
		//writeReg(CTRL_REG1_A,accelEnable);
		//writeReg(CTRL_REG4_A,0x28);
		writeReg(CRA_REG_M,0x0C);	// 7.5 DOR
		writeReg(CRB_REG_M,0x20);	// +/- 1.3 gauss
		writeReg(MR_REG_M,0x00);	// Continuous mode

		printf("Initialization done LSM303 sensor\n");
	}

	return devicePresent;
}

int16_t lsm303_compass::getX()
{
	return (int16_t)get16BitRegister(OUT_X_H_A);
}
int16_t lsm303_compass::getY()
{
    return (int16_t)get16BitRegister(OUT_Y_H_A);
}
int16_t lsm303_compass::getZ()
{
    return (int16_t)get16BitRegister(OUT_Z_H_A);
}

uint8_t lsm303_compass::getXAngleMsb()
{
    return readReg(OUT_X_H_M);
}
uint8_t lsm303_compass::getXAngleLsb()
{
    return readReg(OUT_X_L_M);
}
uint8_t lsm303_compass::getYAngleMsb()
{
    return readReg(OUT_Y_H_M);
}
uint8_t lsm303_compass::getYAngleLsb()
{
    return readReg(OUT_Y_L_M);
}
uint8_t lsm303_compass::getZAngleMsb()
{
    return readReg(OUT_Z_H_M);
}
uint8_t lsm303_compass::getZAngleLsb()
{
    return readReg(OUT_Z_L_M);
}

uint8_t lsm303_compass::getStatus()
{
    return (uint8_t)readReg(SR_REG_M);
}

uint8_t lsm303_compass::getTemperatureMsb()
{
    return (uint8_t)readReg(TEMP_OUT_H_M);
}
uint8_t lsm303_compass::getTemperatureLsb()
{
    return (uint8_t)readReg(TEMP_OUT_L_M);
}

bool lsm303_compass_accl::init()
{

	bool devicePresent = checkDeviceResponse();

	if(devicePresent)
	{
		//const unsigned char datarate15Hz = (0x04 << 2 | 1<<7); 	// data output rate = 15Hz
		//const unsigned char gainConfig81 = 0x07 << 5;
		//const unsigned char continousConvert = 0;
		const unsigned char accelEnable = 0x47;     			/* Normal mode 50Hz  */

		printf("Initializing LSM303 accelerometer sensor\n");
		writeReg(CTRL_REG1_A,accelEnable);
		writeReg(CTRL_REG4_A,0x00);
		//writeReg(CRA_REG_M,0x0C);	// 7.5 DOR
		//writeReg(CRB_REG_M,0x20);	// +/- 1.3 gauss
		//writeReg(MR_REG_M,0x00);	// Continuous mode

		printf("Initialization done LSM303 sensor\n");
	}

	return devicePresent;
}

void lsm303_compass::getHeading()
{
#if 0
	angleXMsb = LSM.getXAngleMsb();
		angleXLsb = LSM.getXAngleLsb();
		angleZMsb = LSM.getZAngleMsb();
		angleZLsb = LSM.getZAngleLsb();
		angleYMsb = LSM.getYAngleMsb();
		angleYLsb = LSM.getYAngleLsb();


		Magx = (int16_t)((angleXMsb << 8) | angleXLsb);
		Magy = (int16_t)((angleYMsb << 8) | angleYLsb);
		Magz = (int16_t)((angleZMsb << 8) | angleZLsb);

		X = LSM.getX();
		Y = LSM.getY();
		Z = LSM.getZ();
		u0_dbg_printf("X:%d,Y:%d,Z:%d\n",X,Y,Z);

		Mag_minx = -572;
			  Mag_miny = -656;
			  Mag_minz = -486;
			  Mag_maxx = 429;
			  Mag_maxy = 395;
			  Mag_maxz = 535;

			  // use calibration values to shift and scale magnetometer measurements
			  Magx = (Magx-Mag_minx)/(Mag_maxx-Mag_minx)*2-1;
			  Magy = (Magy-Mag_miny)/(Mag_maxy-Mag_miny)*2-1;
			  Magz = (Magz-Mag_minz)/(Mag_maxz-Mag_minz)*2-1;

			  // Normalize acceleration measurements so they range from 0 to 1
			  float accxnorm = Accx/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);
			  float accynorm = Accy/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);
			Pitch = asin(-accxnorm);
			Roll = asin(accynorm/cos(Pitch));
			if(Magx > 0 && Magy >= 0)
				{
					heading = atan2(Magy,Magx);
				}
				else if(Magx < 0)
				{
					heading = 180+atan2(Magy,Magx);
				}
				else if(Magx > 0 && Magy < 0)
				{
					heading = 360+atan2(Magy,Magx);
				}

				heading = (atan2(Magy,Magx)*180)/M_PI;

				//u0_dbg_printf("before :%f\n",heading);
				if (heading < 0)
					heading +=360;

				//u0_dbg_printf("%d,%d,%d\n",Magx,Magy,Magz);
				u0_dbg_printf("%f\n",heading);
#endif
}
