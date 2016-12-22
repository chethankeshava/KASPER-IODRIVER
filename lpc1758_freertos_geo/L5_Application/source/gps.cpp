/*
 * gps.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: ankit
 */
#include <string.h>
#include <math.h>
#include <stdlib.h>
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
//char gpsStr[256]="$GPRMC,052547.300,A,3720.1633,N,12152.8534,W,0.03,313.37,211216,,,A*71";
int32_t latitude_fixed, longitude_fixed;
float latitudeDegrees, longitudeDegrees;
float geoidheight, altitude;
float g_distance=300.00;
extern bool newChkPointRecvd;
bool recvdTrueGPSData =false;
uint32_t bRecvInt=0;						/// Counter variable
SemaphoreHandle_t gUartSemaphore = NULL;	/// Uart Semaphore
QueueHandle_t gps_queue1;
volatile bool flag_received = false;
char gps_global_string[100];
#if 0
extern "C"
{
    void UART2_IRQHandler(void)
    {
        static  char GPS_buffer1[100];
        static int i_new=0;
        while(!(LPC_UART2-> LSR & 0x01));
        GPS_buffer1[i_new]= LPC_UART2 -> RBR;

        if(GPS_buffer1[i_new]=='\n')
        {
           GPS_buffer1[i_new]='\0';
           flag_received = true;
           if(xQueueSendFromISR(gps_queue1, &GPS_buffer1 , 0))
           {
           }
           else
           {
           }
           i_new=0;
        }
        else
        {
        	i_new++;
        }
    }
 }
#endif
#if 0
extern "C"
{
	void UART2_IRQHandler(void)
	{
		uint32_t intsrc, tmp, tmp1;
		char c = 0;
		uint32_t bToRecv=256, timeOut=0;
		uint8_t recvData = 0;
		/* Determine the interrupt source */
		intsrc = (LPC_UART2->IIR & 0x03CF);
		tmp = intsrc & UART_IIR_INTID_MASK;

		// Receive Line Status
		if (tmp == UART_IIR_INTID_RLS)
		{
			// Check line status
			tmp1 = ((LPC_UART2->LSR) & UART_LSR_BITMASK);
			// Mask out the Receive Ready and Transmit Holding empty status
			tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
					| UART_LSR_BI | UART_LSR_RXFE);
			// If any error exist
			if (tmp1)
			{
					while(1);
			}
		}

		// Receive Data Available or Character time-out
		if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
		{
			while (bToRecv)
			{
				if (!(LPC_UART2->LSR & UART_LSR_RDR))
				{
					//printf("here1\n");
					gBuffer[bRecvInt++] = '\0';
					//u0_dbg_printf("%s\n",gBuffer);
					bRecvInt =0;
					xSemaphoreGiveFromISR(gUartSemaphore,NULL);
					break;
				}
				else
				{
					recvData= LPC_UART2->RBR;
					//u0_dbg_printf("%c\n",recvData);

					if ((recvData == 0xA) | (recvData == 0xD))
					{
						continue;
					}
					//delay_ms(5);
					u0_dbg_printf("%c\n",recvData);
					gBuffer[bRecvInt] = recvData;
					bRecvInt++;
					bToRecv--;
				}
			}

		}

		// Transmit Holding Empty
		if (tmp == UART_IIR_INTID_THRE)
		{
				//UART_IntTransmit();
		}
		bRecvInt =0;
		//gBuffer[bRecvInt] = '\0';

	}
}
#endif
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
	char lonBuf[8]={0};
	char *tok = NULL;
	float latitude =0.0;
	float longitude =0.0;
	float latitude_min =0.0;
	float longitude_min =0.0;
	//u0_dbg_printf("$");
	//u0_dbg_printf("%s\n",buffer);

	tok = strtok((char *)buffer, ",");
	if (! tok)
		return false;

	if(!strcmp(tok,"$GPGGA"))
	{
		tok = strtok(NULL, ",");
		if (!tok)
			return false;
#if 0
		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		if(strcmp(tok,"A"))
		{
			//u0_dbg_printf("void(Invalid) data\n");
			return true;
		}
#endif
		//recvdTrueGPSData = true;

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

		//u0_dbg_printf("%f\n",curLatitude);

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		// Parsing Longitude
		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		lonBuf[0]=tok[0];
		lonBuf[1]=tok[1];
		lonBuf[2]=tok[2];
		longitude = atof(lonBuf);
		//u0_dbg_printf("lat -%s\n",&tok[3]);
		longitude_min = atof(&tok[3]);
		longitude_min = (float)longitude_min /60;
		longitude = longitude + longitude_min;

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		if(!strcmp(tok,"W"))
		{
			longitude *= -1;
		}

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		//u0_dbg_printf("%s\n",tok);
//		if(strcmp(tok,"0"))
//		{
//			return false;
//		}

		recvdTrueGPSData = true;
		if( latitude > 37 && longitude < -121  )
		{
			curLatitude = (float)latitude;
			curLongitude = (float)longitude;
			//recvdTrueGPSData = true;
			u0_dbg_printf("%f,%f\n",curLatitude,curLongitude);
		}
		return true;
	}
}


/**************************************************************************************************
*
**************************************************************************************************/
void gpsSetRMCOnlyOutput()
{
	char gprmcOnly[] = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";		// Only GPRMC
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


#if 0
bool geoTask::init()
{
	uint16_t baud = 0;
	int i = 0;
	char gprmcOnly[] = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
	char frequency[] = "$PMTK220,100*2F\r\n";
	const unsigned int pclk = sys_get_cpu_clock();
	uint32_t baudrate = 9600;
	gUartSemaphore = xSemaphoreCreateBinary();
	gps_queue1 = xQueueCreate(2, 100 * sizeof(char));
	// Turn on UART2 module
	LPC_SC->PCONP |= (1 << 24);

	// Select clock for UART2 PCLK = CCLK
	LPC_SC->PCLKSEL1 &= ~(2<<16);
	LPC_SC->PCLKSEL1 |= (1<<16);


	// Configure UART2 :8-bit character length1 :1 stop bit :Parity Disabled
	LPC_UART2->LCR = 0x03;

	// Enable DLAB (DLAB = 1)
	LPC_UART2->LCR |= (1<<7);

	// For baudrate 115200
	//div = 48000000/16*baudRate;// = 312;
	//divWord = 48000000/(16*115200);
	baud = (pclk / (16 * baudrate));

	LPC_UART2->DLL = (baud & 0xFF);
	LPC_UART2->DLM = (baud >> 8);

	// Disabling DLAB (DLAB =0)
	LPC_UART2->LCR &= ~(1<<7) ;

	// Enable & Reset FIFOs and set 4 char timeout for Rx
	LPC_UART2->FCR = 0x07;
	LPC_UART2->FCR |= (3<<6);
	//UARTx->FCR &= ~(1<<3);

	// Pin select for P2.8 TXD2 and P2.9 RXD2
	LPC_PINCON->PINSEL4 &= ~((1<<16) | (1<<18));
	LPC_PINCON->PINSEL4 |= ((1<<17) | (1<<19));

	for( i = 0; gprmcOnly[i] != '\0'; i++)
	{
		gpsPutch(gprmcOnly[i]);
		delay_ms(1);
	}

	i =0;

	for( i = 0; frequency[i] != '\0'; i++)
	{
		gpsPutch(frequency[i]);
		delay_ms(1);
	}


	// Pin select for P4.28 TXD3 and P4.29 RXD3
	//LPC_PINCON->PINSEL9 |= ((1<<24) | (1<<25) | (1<<26) | (1<<27));
	NVIC_EnableIRQ(UART2_IRQn);
	LPC_UART2->IER = (1 << 0) | (1 << 2); // B0:Rx, B1: Tx
	//sim808_reset(LPC_GPIO0, GPS_RESET_PIN);
	u0_dbg_printf("Starting communication\n");

	//mUARTx = UARTx;
	return true;

}
#endif
#if 1
/**************************************************************************************************
*
**************************************************************************************************/
bool geoTask::init(void)
{
	gpsUart.init(GPS_BAUD_RATE,rx_q,tx_q);
	char gprmcOnly[] = "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
	char frequency[] = "$PMTK220,100*2F\r\n";
	int i = 0;

	for( i = 0; gprmcOnly[i] != '\0'; i++)
	{
		gpsPutch(gprmcOnly[i]);
		delay_ms(1);
	}

	i =0;
#if 1
	for( i = 0; frequency[i] != '\0'; i++)
	{
		gpsPutch(frequency[i]);
		delay_ms(1);
	}
#endif

	LSM_MAG.init();
	LSM_ACCL.init();

	//gUartSemaphore = xSemaphoreCreateBinary();
	return true;
}
#endif
/**************************************************************************************************
*
**************************************************************************************************/
bool geoTask::readGpsData()
{
#if 1
	int i=0;
	char c=0;
	static int count =0;

	//gpsUart.flush();
	count++;

	//u0_dbg_printf("%d\n,count-%d",gpsUart.getRxQueueSize(),count);
	//while(gpsUart.gets(&c, 0))
	gpsUart.gets(rxBuff,sizeof(rxBuff),20);
	//char gpsStr[256]="$GPRMC,052547.300,A,3720.1633,N,12152.8534,W,0.03,313.37,211216,,,A*71";
#if 0
	{
		if ('\r' != c && '\n' != c)
		{
			rxBuff[i++] = c;
		}
		if('\n' == c )
		{
			break;
		}
		//u0_dbg_printf("%c\n",c);
	}
#endif
	//rxBuff[i++] = '\0';
	//u0_dbg_printf("After :%d\n",gpsUart.getRxQueueSize());
	//strncpy(gpsStr,rxBuff,sizeof(rxBuff));
	parseGpsData(rxBuff);
	//u0_dbg_printf("%s\n",rxBuff);
	memset(rxBuff,0,256);
	//bzero(gpsStr,sizeof(gpsStr));
	//u0_dbg_printf("%s\n",rxBuff);
	//gpsUart.flush();
#endif
#if 0
	if(xQueueReceiveFromISR(gps_queue1,&gps_global_string,0))
	{
		u0_dbg_printf("%s\n",gps_global_string);
		//parseGpsData(gps_global_string);
	}
#endif
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
		calculateDistance();
		//curLatitude = 37.335187;
		//curLongitude = -121.881071;
		geoGpsData.GPS_LOCATION_latitude = curLatitude;
		geoGpsData.GPS_LOCATION_longitude = curLongitude;

		u0_dbg_printf("lat : %f lon: %f\n",curLatitude,curLongitude);

		// This function will encode the CAN data bytes, and send the CAN msg using dbc_app_send_can_msg()
		dbc_encode_and_send_GPS_LOCATION(&geoGpsData);
		recvdTrueGPSData = false;
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

	//calculateDistance();

	if(newChkPointRecvd)
	{
		calculateBearing();
		newChkPointRecvd = false;
	}

	//compassi2c.getHeading(&heading);
	LSM_MAG.getHeading(&heading);

	//u0_dbg_printf("Heading is %f\n",head);
	geoCompassData.COMPASS_DATA_bearing = bearing;
	geoCompassData.COMPASS_DATA_heading = heading;
	geoCompassData.COMPASS_DATA_speed	= speed;
	geoCompassData.COMPASS_DATA_distance = distance;

	u0_dbg_printf("head: %f,bearing:%f,Dis:%.2f\n",heading,bearing,distance);
	// This function will encode the CAN data bytes, and send the CAN msg using dbc_app_send_can_msg()
	dbc_encode_and_send_COMPASS_DATA(&geoCompassData);

}
