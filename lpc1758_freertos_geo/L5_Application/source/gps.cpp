/*
 * gps.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: ankit
 */
#include <string.h>
#include "tasks.hpp"
#include "uart2.hpp"
#include "printf_lib.h"
#include "gps.hpp"
#include "can.h"
#include "../_can_dbc/generated_can.h"
#include "utilities.h"

#define 	GPS_DATA_LEN 					256
#define 	PMTK_SET_NMEA_OUTPUT_RMCONLY 	"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"
#define 	PMTK_SET_NMEA_OUTPUT_100 		"$PMTK220,100*2F\r\n"
#define 	GPS_BAUD_RATE 					9600			///	100Kbps baud rate
#define 	GPS_CAN_RX_QUEUE_SIZE			16
#define 	GPS_CAN_TX_QUEUE_SIZE			16


char gpsData[GPS_DATA_LEN];
char gBuffer[256];							/// Global buffer for received data
int latitude =0;
int longitude =0;
float latitude_min =0.0;
float longitude_min =0.0;


/**************************************************************************************************
*
**************************************************************************************************/
void canBusOffCallback( uint32_t icr_data )
{

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
bool parseGpsData(char *buffer)
{
	char latBuf[8]={0};
	char *tok = NULL;
	tok = strtok((char *)buffer, ",");
	if (! tok)
		return false;

	if(!strcmp(tok,"$GPRMC"))
	{
		tok = strtok(NULL, ",");
		if (!tok)
			return false;


		u0_dbg_printf("%s\n",tok);

		tok = strtok(NULL, ",");
		if (!tok)
			return false;

		u0_dbg_printf("%s\n",tok);

		if(!strcmp(tok,"V"))
		{
			u0_dbg_printf("void(Invalid) data\n");
			latitude = 0;
			longitude = 0;
			//return true;
		}

		tok = strtok(NULL, ",");
		if (!tok)
			return false;
		latBuf[0]=tok[0];
		latBuf[1]=tok[1];
		latitude = atoi(latBuf);
		u0_dbg_printf("%d\n",latitude);
		latitude_min = atof(&tok[2]);
		u0_dbg_printf("%.4f\n",latitude_min);

		tok = strtok(NULL, ",");
		if (!tok)
			return false;
		u0_dbg_printf("%s\n",tok);

		tok = strtok(NULL, ",");
		if (!tok)
			return false;
		latBuf[0]=tok[0];
		latBuf[1]=tok[1];
		latBuf[2]=tok[2];
		longitude = atoi(latBuf);

		longitude_min = atof(&tok[3]);
		u0_dbg_printf("%.4f\n",longitude_min);


		tok = strtok(NULL, ",");
		if (!tok)
			return false;
		if(!strcmp(tok,"W"))
		{
			longitude *= -1;
		}

		u0_dbg_printf("%d\n",longitude);
	}
	return true;
}

/**************************************************************************************************
*
**************************************************************************************************/
void gpsInit()
{
	if(CAN_init(GPS_CAN_BUS, 100, GPS_CAN_RX_QUEUE_SIZE, GPS_CAN_TX_QUEUE_SIZE, canBusOffCallback, NULL))
	{
		u0_dbg_printf("Initialize CAN module\n");
	}
	else
	{
		u0_dbg_printf("unable to initialize CAN module\n");
	}
	CAN_reset_bus(GPS_CAN_BUS);
}

/**************************************************************************************************
*
**************************************************************************************************/
void gpsSetRMCOnlyOutput()
{
	char gprmcOnly[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
	char frequency[] =  "$PMTK220,100*2F\r\n";

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
gpsTask::gpsTask(uint8_t priority) :
	            scheduler_task("gpsTask", 2000, priority),gpsUart(Uart2::getInstance())
{
	/* Nothing to init */
	u0_dbg_printf("Initializing uart\n");
	gpsUart.init(GPS_BAUD_RATE,rx_q,tx_q);
	char gprmcOnly[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
	char frequency[] = "$PMTK220,100*2F\r\n";
	int i = 0;

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

}

/**************************************************************************************************
*
**************************************************************************************************/
bool gpsTask::init(void)
{

	gpsInit();
	//gpsSetRMCOnlyOutput();

	//NVIC_EnableIRQ(UART2_IRQn);
	return true;
}

/**************************************************************************************************
*
**************************************************************************************************/
bool gpsTask::run(void *p)
{
	char rxBuff[256];
	char gpsStr[256];
	gpsUart.gets(rxBuff, sizeof(rxBuff), portMAX_DELAY);
	u0_dbg_printf("Received data is %s\n",rxBuff);
	strncpy(gpsStr,rxBuff,sizeof(rxBuff));
	parseGpsData(gpsStr);
	return true;
}


/**************************************************************************************************
*
**************************************************************************************************/
bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(GPS_CAN_BUS, &can_msg, 0);
}

/**************************************************************************************************
*
**************************************************************************************************/
void geoSendGpsData()
{
	GPS_LOCATION_t gps_data = { 0 };
	//gps_data.GPS_LOCATION_latitude = 37.123456;
	//gps_data.GPS_LOCATION_longitude = 121.123456;
	gps_data.GPS_LOCATION_latitude = latitude;
	gps_data.GPS_LOCATION_longitude = longitude;

	// This function will encode the CAN data bytes, and send the CAN msg using dbc_app_send_can_msg()
	dbc_encode_and_send_GPS_LOCATION(&gps_data);
}

void geoSendHeartBeat()
{
	GEO_HEARTBEAT_t heartBeat={0};

	heartBeat.GEO_HEARTBEAT_data = 0xAA;
	dbc_encode_and_send_GEO_HEARTBEAT(&heartBeat);
}
