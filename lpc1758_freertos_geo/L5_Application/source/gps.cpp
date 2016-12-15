/*
 * gps.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: ankit
 */
#include <string.h>
#include <math.h>
#include "tasks.hpp"
#include "uart2.hpp"
#include "printf_lib.h"
#include "gps.hpp"
#include "geo.hpp"
#include "can.h"
#include "../_can_dbc/generated_can.h"
#include "utilities.h"
#include "io.hpp"
#define 	GPS_DATA_LEN 					256
#define 	PMTK_SET_NMEA_OUTPUT_RMCONLY 	"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"
#define 	PMTK_SET_NMEA_OUTPUT_100 		"$PMTK220,100*2F\r\n"
#define 	GPS_BAUD_RATE 					9600			///	100Kbps baud rate


//extern Uart2 &gpsUart;

/**
 * todo: try to avoid global variables.
 */


char gpsData[GPS_DATA_LEN];
char gBuffer[256];							/// Global buffer for received data
char rxBuff[256]={};
char gpsStr[256]={};
int32_t latitude_fixed, longitude_fixed;
float latitudeDegrees, longitudeDegrees;
float geoidheight, altitude;
float g_distance=300.00;
extern bool newChkPointRecvd;
bool recvdTrueGPSData =false;
/**********************************************************************************************************************
 *
 *********************************************************************************************************************/
inline float radiansToDegrees(float radians)
{
    return radians * (180.0 / M_PI);
}

/**********************************************************************************************************************
 *
 *********************************************************************************************************************/
inline float degreesToRadians(float degree)
{
    return degree * (M_PI / 180.0);
}

/**************************************************************************************************
*
**************************************************************************************************/
void gpsPutch(char data)
{
	LPC_UART2->THR = (char)(data);
	while(! (LPC_UART2->LSR & (1 << 6)));
}

/**************************************************************************************************
*
**************************************************************************************************/
bool geoTask::parseGpsData(char *buffer)
{
	char latBuf[8]={0};
	char *tok = NULL;
	float latitude =0;
	float longitude =0;
	float latitude_min =0.0;
	float longitude_min =0.0;
	//u0_dbg_printf("%s\n",buffer);

	tok = strtok((char *)buffer, ",");
	if (! tok)
		return false;

	if(!strcmp(tok,"$GPRMC"))
	{
		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		if(strcmp(tok,"A"))
		{
			//u0_dbg_printf("void(Invalid) data\n");
			return true;
		}

		recvdTrueGPSData = true;

		// Parsing Latitude
		tok = strtok(NULL, ",");
		if (!tok)
			return false;
		latBuf[0]=tok[0];
		latBuf[1]=tok[1];
		latitude = atof(latBuf);
		latitude_min = atof(&tok[2]);
		latitude_min /= 60;
		latitude = latitude + latitude_min;
		curLatitude = latitude;
		//u0_dbg_printf("%f\n",curLatitude);

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		// Parsing Longitude
		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		latBuf[0]=tok[0];
		latBuf[1]=tok[1];
		latBuf[2]=tok[2];
		longitude = atof(latBuf);
		longitude_min = atof(&tok[3]);
		longitude_min /= 60;
		longitude = longitude + longitude_min;

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		//if(!strcmp(tok,"W"))
		{
			longitude *= -1;
		}

		curLongitude = longitude;
		u0_dbg_printf("%f,%f\n",curLatitude,curLongitude);
	}
	return true;
}


/**************************************************************************************************
*
**************************************************************************************************/
void gpsSetRMCOnlyOutput()
{
	char gprmcOnly[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";		// Only GPRMC
	char frequency[] =  "$PMTK220,100*2F\r\n";										// 10Hz

	int i = 0;

	for( i = 0; gprmcOnly[i] != '\0'; i++)
	{
		//gpsPutch(gprmcOnly[i]);
		LPC_UART2->THR = gprmcOnly[i];
		delay_ms(1);
	}
	i =0;
	for( i = 0; frequency[i] != '\0'; i++)
	{
		//gpsPutch(frequency[i]);
		LPC_UART2->THR = frequency[i];
		delay_ms(1);
	}
}


/**************************************************************************************************
*
**************************************************************************************************/
bool geoTask::init(void)
{
	gpsUart.init(GPS_BAUD_RATE,rx_q,tx_q);
	char gprmcOnly[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
	char frequency[] = "$PMTK220,100*2F\r\n";
	int i = 0;

	for( i = 0; gprmcOnly[i] != '\0'; i++)
	{
		gpsPutch(gprmcOnly[i]);
		delay_ms(1);
	}
#if 0
	i =0;

	for( i = 0; frequency[i] != '\0'; i++)
	{
		gpsPutch(frequency[i]);
		delay_ms(1);
	}
#endif
	return true;
}

/**************************************************************************************************
*
**************************************************************************************************/
bool geoTask::readGpsData()
{

	int i=0;
	char c=0;
	gpsUart.flush();
	while(gpsUart.getChar(&c, 0))
	{
		if ('\r' != c && '\n' != c)
		{
			rxBuff[i++] = c;
		}
		if('\n' == c )
		{
			break;
		}
	}

	rxBuff[i++] = '\0';

	strncpy(gpsStr,rxBuff,sizeof(rxBuff));
	parseGpsData(gpsStr);
	//u0_dbg_printf("%s\n",rxBuff);
	//gpsUart.flush();
	return true;
}
void geoTask::setChkPointData(float chkLatitude,float chkLongitude)
{
	chkPointLatitude = chkLatitude;
	chkPointLongitude = chkLongitude;
}
geoTask::geoTask() : gpsUart(Uart2::getInstance())
{
	/* Nothing to init */
}

/**************************************************************************************************
*
**************************************************************************************************/
void geoTask::sendGpsData()
{
	if(recvdTrueGPSData == true)
	{
		GPS_LOCATION_t geoGpsData = { 0 };
		//readGpsData();
		//curLatitude = 37.335187;
		//curLongitude = -121.881071;
		geoGpsData.GPS_LOCATION_latitude = curLatitude;
		geoGpsData.GPS_LOCATION_longitude = curLongitude;

		u0_dbg_printf("lat : %f lon: %f\n",curLatitude,curLongitude);

		// This function will encode the CAN data bytes, and send the CAN msg using dbc_app_send_can_msg()
		dbc_encode_and_send_GPS_LOCATION(&geoGpsData);
	}
}

/**********************************************************************************************************************
 * geoCalculateBearing : calculates bearing angle from given GPS points
 *********************************************************************************************************************/
void geoTask::calculateBearing()
{
	float lat1 = degreesToRadians(curLatitude);
	float lon1 = degreesToRadians(curLongitude);
	float lat2 = degreesToRadians(chkPointLatitude);
	float lon2 = degreesToRadians(chkPointLongitude);

	float lon_diff = lon2 - lon1;

	float y = sin(lon_diff) * cos(lat2);
	float x = cos(lat1) * sin(lat2) - sin(lat1)* cos(lat2) * cos(lon_diff);
	float brng = atan2(y,x);

	brng = radiansToDegrees(brng) ;

	if( brng < 0)
	{
		brng = 360 + brng;
	}

	bearing = brng;
}

/**********************************************************************************************************************
 * geoCalculateDistance : calculates distance between two GPS locations in meters
 *********************************************************************************************************************/
void geoTask::calculateDistance()
{
    float lat1 = degreesToRadians(curLatitude);
    float lon1 = degreesToRadians(curLongitude);
    float lat2 = degreesToRadians(chkPointLatitude);
    float lon2 = degreesToRadians(chkPointLongitude);

    float lat_diff = lat2 - lat1;
    float lon_diff = lon2 - lon1;

    float b = ((sin(lat_diff/2))*(sin(lat_diff/2))) + (cos(lat1) * cos(lat2) * (sin(lon_diff/2))*sin(lon_diff/2));
    float c = 2 * atan2(sqrt(b), sqrt(1-b));

    float d = EARTH_RADIUS_KM * c * 1000;
    //d=d/1000;									// Convert to kilometers
    distance = d;
#if 0
    // For testing
    if(g_distance < 0)
    {
	   g_distance = 300;
    }
    g_distance -= 30;

    distance = g_distance;
#endif
}

/**************************************************************************************************
*
**************************************************************************************************/
void geoTask::sendCompassData()
{
	COMPASS_DATA_t geoCompassData = { 0 };

	calculateDistance();

	if(newChkPointRecvd)
	{
		calculateBearing();
		newChkPointRecvd = false;
	}

	//compassi2c.getHeading(&heading);
	LSM.getHeading(&heading);
	geoCompassData.COMPASS_DATA_bearing = bearing;
	geoCompassData.COMPASS_DATA_heading = heading;
	geoCompassData.COMPASS_DATA_speed	= speed;
	geoCompassData.COMPASS_DATA_distance = distance;

	u0_dbg_printf("head: %f,bearing:%f,Dis:%.2f\n",heading,bearing,distance);
	// This function will encode the CAN data bytes, and send the CAN msg using dbc_app_send_can_msg()
	dbc_encode_and_send_COMPASS_DATA(&geoCompassData);

}
