/*
 * lsm303_compass.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: ankit
 */
#include <stdio.h>
#include <stdint.h>
//#include <math.h>
#include "io.hpp" 					// All IO Class definitions
#include "lsm303_compass.hpp"
#include "printf_lib.h"
#include "utilities.h"
float Mx, My, Mz;
float Ax, Ay, Az;
float heading;
float pitch;
float roll;
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
		printf("Initializing LSM303 Magnetometer\n");

		writeReg(CTRL_REG1_A,0x67);
		writeReg(CRA_REG_M,0x9C);	// 7.5 DOR
		writeReg(CRB_REG_M,0x20);	// +/- 1.3 gauss
		writeReg(MR_REG_M,0x00);	// Continuous mode

		//writeReg(CTRL_REG4_A,0x00);

		printf("Initialization done LSM303 Magnetometer\n");

		//m_min = (vector<int16_t>){-32767, -32767, -32767};
		//m_max = (vector<int16_t>){+32767, +32767, +32767};
	}

	return devicePresent;
}

uint8_t lsm303_compass::getStatus()
{
    return (uint8_t)readReg(SR_REG_M);
}
void lsm303_compass::getMagnetometerData()
{
	uint8_t MR_Data[6]={0};

	MR_Data[0] = readReg(OUT_X_H_M);
	delay_us(100);
	MR_Data[1] = readReg(OUT_X_L_M);
	delay_us(100);
	MR_Data[2] = readReg(OUT_Y_H_M);
	delay_us(100);
	MR_Data[3] = readReg(OUT_Y_L_M);
	delay_us(100);
	MR_Data[4] = readReg(OUT_Z_H_M);
	delay_us(100);
	MR_Data[5] = readReg(OUT_Z_L_M);
	delay_us(100);

	Mx = (int16_t)(MR_Data[0] << 8) + MR_Data[1];
	//m.x	= (int16_t)(MR_Data[0] << 8) + MR_Data[1];
	My = (int16_t)(MR_Data[2] << 8) + MR_Data[3];
	//m.y = (int16_t)(MR_Data[2] << 8) + MR_Data[3];
	Mz = (int16_t)(MR_Data[4] << 8) + MR_Data[5];
	//m.z = (int16_t)(MR_Data[4] << 8) + MR_Data[5];

	//u0_dbg_printf("Mx:%f,My:%f,Mz:%f\n",Mx,My,Mz);
}



#if 0
void lsm303_compass::calibrate(void)
{
	LSM_MAG.getMagnetometerData();
	running_min.x = min(running_min.x, m.x);
	running_min.y = min(running_min.y, m.y);
	running_min.z = min(running_min.z, m.z);

	running_max.x = max(running_max.x, m.x);
	running_max.y = max(running_max.y, m.y);
	running_max.z = max(running_max.z, m.z);
}
#endif
void lsm303_compass::getHeading(float *curHeading)
{
#if 1
//	Mag_minx = -572;
//	Mag_miny = -656;
//	Mag_minz = -486;
//	Mag_maxx = 429;
//	Mag_maxy = 395;
//	Mag_maxz = 535;
	//Mag_minx = -542;
	//Mag_miny = -734;
	//Mag_minz = -1282;
	//Mag_maxx = 710;
	//Mag_maxy = 465;
	//Mag_maxz = -1158;

	LSM_MAG.getMagnetometerData();
	LSM_ACCL.getAccelerometerData();

	Mag_minx = -268;
	Mag_miny = -440;
	Mag_minz = -756;
	Mag_maxx = 437;
	Mag_maxy = 254;
	Mag_maxz = -703;

	// use calibration values to shift and scale magnetometer measurements
	Mx = (Mx-Mag_minx)/(Mag_maxx-Mag_minx)*2-1;
	My = (My-Mag_miny)/(Mag_maxy-Mag_miny)*2-1;
	Mz = (Mz-Mag_minz)/(Mag_maxz-Mag_minz)*2-1;


	// Normalize acceleration measurements so they range from 0 to 1
	float accxnorm = Ax/sqrt(Ax*Ax+Ay*Ay+Az*Az);
	float accynorm = Ay/sqrt(Ax*Ax+Ay*Ay+Az*Az);

	pitch = asin(-accxnorm);
	roll = asin(accynorm/cos(pitch));

	//u0_dbg_printf("before :%f\n",heading);

	float magxcomp = Mx*cos(pitch) + Mz*sin(pitch);
	float magycomp = Mx*sin(roll)*sin(pitch) + My*cos(roll)-Mz*sin(roll)*cos(pitch);

#if 0
	if(magxcomp > 0 && magycomp >= 0)
	{
		heading = atan2(magycomp,magxcomp);
	}
	else if(magxcomp < 0)
	{
		heading = 180+atan2(magycomp,magxcomp);
	}
	else if(magxcomp > 0 && magycomp < 0)
	{
		heading = 360+atan2(magycomp,magxcomp);
	}
	else if (magxcomp == 0 && magycomp < 0)
	{
		heading =90;
	}
	else if (magxcomp == 0 && magycomp > 0)
	{
		heading =270;
	}
#endif

	heading = (180*atan2(magycomp, magxcomp))/M_PI;

	if (heading < 0)
		heading +=360;

	*curHeading = heading;
	//u0_dbg_printf("%d,%d,%d\n",Magx,Magy,Magz);
	//u0_dbg_printf("%f\n",heading);
#endif
}


bool lsm303_compass_accl::init()
{

	bool devicePresent = checkDeviceResponse();

	if(devicePresent)
	{
		const unsigned char accelEnable = 0x67;     			/* Normal mode 50Hz  */

		printf("Initializing LSM303 accelerometer sensor\n");
		writeReg(CTRL_REG1_A,accelEnable);
		writeReg(CTRL_REG4_A,0x00);

		printf("Initialization done LSM303 sensor\n");
	}

	return devicePresent;
}

void lsm303_compass_accl::getAccelerometerData()
{
	uint8_t ACC_Data[6]={0};

	ACC_Data[0] = readReg(OUT_X_H_A);
	delay_us(100);
	ACC_Data[1] = readReg(OUT_X_L_A);
	delay_us(100);
	ACC_Data[2] = readReg(OUT_Y_H_A);
	delay_us(100);
	ACC_Data[3] = readReg(OUT_Y_L_A);
	delay_us(100);
	ACC_Data[4] = readReg(OUT_Z_H_A);
	delay_us(100);
	ACC_Data[5] = readReg(OUT_Z_L_A);
	delay_us(100);

	Ax = (int16_t)(ACC_Data[0] << 8) + ACC_Data[1];
	Ay = (int16_t)(ACC_Data[2] << 8) + ACC_Data[3];
	Az = (int16_t)(ACC_Data[4] << 8) + ACC_Data[5];

	//u0_dbg_printf("Ax:%f,Ay:%f,Az:%f\n",Ax,Ay,Az);
}
