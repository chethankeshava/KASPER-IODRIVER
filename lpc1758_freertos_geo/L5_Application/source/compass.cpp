/*
 * compass.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: ankit
 */


#include "compass.hpp"
#include "math.h"
#include "printf_lib.h"


bool compass::init()
{
	bool devicePresent = checkDeviceResponse();

	if(devicePresent)
	{
		if ((readReg(HMC5883L_REG_IDENT_A) != 0x48) || (readReg(HMC5883L_REG_IDENT_B) != 0x34) || (readReg(HMC5883L_REG_IDENT_C) != 0x33))
		{
			return false;
		}

		mgPerDigit = 0.92f;
		setRange(HMC5883L_RANGE_1_3GA);
		setMeasurementMode(HMC5883L_CONTINOUS);
		setDataRate(HMC5883L_DATARATE_30HZ);
		setSamples(HMC5883L_SAMPLES_8);
		setOffset(0, 0);
		v={0};
	}

	return devicePresent;
}

//Vector compass::readRaw(void)
void compass::readRaw(void)
{
    v.XAxis = get16BitRegister(HMC5883L_REG_OUT_X_M) - xOffset;
    v.YAxis = get16BitRegister(HMC5883L_REG_OUT_Y_M) - yOffset;
    v.ZAxis = get16BitRegister(HMC5883L_REG_OUT_Z_M);

    //return v;
}

//Vector compass::readNormalize(void)
void compass::readNormalize(void)
{

    v.XAxis = ((float)((int16_t)get16BitRegister(HMC5883L_REG_OUT_X_M)) - xOffset) * mgPerDigit;
    v.YAxis = ((float)((int16_t)get16BitRegister(HMC5883L_REG_OUT_Y_M)) - yOffset) * mgPerDigit;
    v.ZAxis = (float)((int16_t)get16BitRegister(HMC5883L_REG_OUT_Z_M)) * mgPerDigit;

   // return v;
}

void compass::setMeasurementMode(hmc5883l_mode_t mode)
{
    uint8_t value;

    value = readReg(HMC5883L_REG_MODE);
    value &= 0b11111100;
    value |= mode;

    writeReg(HMC5883L_REG_MODE, mode);
}

void compass::setOffset(int xo, int yo)
{
    xOffset = xo;
    yOffset = yo;
}

void compass::setSamples(hmc5883l_samples_t samples)
{
    uint8_t value;

    value = readReg(HMC5883L_REG_CONFIG_A);
    value &= 0b10011111;
    value |= (samples << 5);

    writeReg(HMC5883L_REG_CONFIG_A, value);
}
void compass::setRange(hmc5883l_range_t range)
{
    switch(range)
    {
	case HMC5883L_RANGE_0_88GA:
	    mgPerDigit = 0.073f;
	    break;

	case HMC5883L_RANGE_1_3GA:
	    mgPerDigit = 0.92f;
	    break;

	case HMC5883L_RANGE_1_9GA:
	    mgPerDigit = 1.22f;
	    break;

	case HMC5883L_RANGE_2_5GA:
	    mgPerDigit = 1.52f;
	    break;

	case HMC5883L_RANGE_4GA:
	    mgPerDigit = 2.27f;
	    break;

	case HMC5883L_RANGE_4_7GA:
	    mgPerDigit = 2.56f;
	    break;

	case HMC5883L_RANGE_5_6GA:
	    mgPerDigit = 3.03f;
	    break;

	case HMC5883L_RANGE_8_1GA:
	    mgPerDigit = 4.35f;
	    break;

	default:
	    break;
    }

    writeReg(HMC5883L_REG_CONFIG_B, (range << 5));
}

void compass::setDataRate(hmc5883l_dataRate_t dataRate)
{
    uint8_t value;

    value = readReg(HMC5883L_REG_CONFIG_A);
    value &= 0b11100011;
    value |= (dataRate << 2);

    writeReg(HMC5883L_REG_CONFIG_A, value);

}

void compass::getHeading(float *curHeading)
{
	readNormalize();
	float headingDegrees=0.0;
	// Calculate heading
	float heading = atan2(v.YAxis, v.XAxis);

	// Set declination angle on your location and fix heading
	// You can find your declination on: http://magnetic-declination.com/
	// (+) Positive or (-) for negative
	// For Bytom / Poland declination angle is 4'26E (positive)
	// Formula: (deg + (min / 60.0)) / (180 / M_PI);
	float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
	heading += declinationAngle;

	// Correct for heading < 0deg and heading > 360deg
	if (heading < 0)
	{
		heading += 2 * M_PI;
	}

	if (heading > 2 * M_PI)
	{
		heading -= 2 * M_PI;
	}

	// Convert to degrees
	*curHeading = (heading * 180/M_PI);

	u0_dbg_printf("Heading value is %.2f,%.2f\n",heading,curHeading);
	v={0};

}


