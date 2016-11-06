/*
 * bluetooth.cpp
 *
 *  Created on: Oct 20, 2016
 *      Author: Bharat Khanna
 */

#include"bluetooth.hpp"
#include<uart2.hpp>
#include<string.h>
#include "printf_lib.h"
#include "../../_can_dbc/generated_can.h"
#include"stdio.h"
#include "io.hpp"
SemaphoreHandle_t Bluetooth_Lat_Lon_Semaphore = NULL;

char Bluetooth_Buffer[96];
char stored_Bluetooth_data[96];
Bluetooth_Received *Bluetooth_Rec;
SemaphoreHandle_t BluetoothSemaphore = NULL;



bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id                = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(can1, &can_msg, 0);
}

void canBusErrorCallBackRx(uint32_t ibits)
{
	const uint32_t errbit = (ibits >> 16) & 0x1F;
	const char * rxtx = ((ibits >> 21) & 1) ? "RX" : "TX";
	const uint32_t errc = (ibits >> 22) & 3;

	u0_dbg_put("\n\n ***** CAN BUS ENTERED ERROR STATE!\n");
	u0_dbg_printf("ERRC = %#x, ERRBIT = %#x while %s\n", errc, errbit, rxtx);
}

bool uart_putchar(char character)
{
	Uart2& Bluetooth_uart_2 = Uart2::getInstance();
	Bluetooth_uart_2.init(115200);
	Bluetooth_uart_2.putChar(character);
	while(! (LPC_UART2->LSR & (1 << 6)));
	return true;
}

Bluetooth_Enable::Bluetooth_Enable(uint8_t priority) :
       scheduler_task("Bluetooth_Enable", 2000, priority),Bluetooth_uart_2(Uart2::getInstance())
{
	Bluetooth_uart_2.init(Baud_Rate_Bluetooth_Uart2,Rx_Q_Size,Tx_Q_Size);
}
bool Bluetooth_Enable::init(void)
{
	   return true;
}


bool Bluetooth_Enable::run(void *p)
{
	Bluetooth_uart_2.gets(Bluetooth_Buffer, sizeof(Bluetooth_Buffer), portMAX_DELAY);
	strcpy(stored_Bluetooth_data,Bluetooth_Buffer);
	u0_dbg_printf("I have crossed the gets function %s\n",stored_Bluetooth_data);
	return true;
}


void Check_Start_STOP_Condition()
{
	START_CMD_APP_t START_CONDITION	=	{0};
	STOP_CMD_APP_t STOP_CONDITION	=	{0};
	if(stored_Bluetooth_data[0] == '1')
	{
		START_CONDITION.START_CMD_APP_data = 1;
		dbc_encode_and_send_START_CMD_APP(&START_CONDITION);
		printf("stored_Bluetooth_data[0]= '1' received\n");
		stored_Bluetooth_data[0] = {0};
	}
	if(stored_Bluetooth_data[1] == '1')
	{
		STOP_CONDITION.STOP_CMD_APP_data = 1;
		dbc_encode_and_send_STOP_CMD_APP(&STOP_CONDITION);
		printf("stored_Bluetooth_data[1]= '1' received\n");
		stored_Bluetooth_data[1] = {0};
	}

}

void Can_Receive_ID_Task()
{
	can_msg_t can_msg_Info;
	RESET_t REST_Info = { 0 };
	GPS_LOCATION_t GPS_LOCATION_RECEIVE = {0};

	if (CAN_rx(can1, &can_msg_Info, 0))
	{
		LE.off(1);
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg_Info.frame_fields.data_len;
		can_msg_hdr.mid = can_msg_Info.msg_id;
		//u0_dbg_printf("id :%d\n",can_msg_hdr.mid);
		if(can_msg_Info.msg_id == STOP_CAR_HDR.mid)
			printf("Case_STOP_CAR\n");
		if(can_msg_Info.msg_id == RESET_HDR.mid)
			dbc_decode_RESET(&REST_Info, can_msg_Info.data.bytes, &can_msg_hdr);
		if(can_msg_Info.msg_id == CURRENT_LOCATION_ACK_HDR.mid)
			printf("Case_CURRENT_LOCATION_ACK\n");
		if(can_msg_Info.msg_id == RECEIVE_START_ACK_HDR.mid)
			printf("Case_RECEIVE_START_ACKt\n");
		if(can_msg_Info.msg_id == BRIDGE_POWER_SYNC_HDR.mid)
			printf("BRIDGE_POWER_SYNC_data\n");
		if(can_msg_Info.msg_id == GPS_LOCATION_HDR.mid)
		{
			dbc_decode_GPS_LOCATION(&GPS_LOCATION_RECEIVE, can_msg_Info.data.bytes, &can_msg_hdr);
			printf("GPS_LOCATION_RECEIVE\n");
			//u0_dbg_printf("Latitude: %f ,Longitude: %f \n",GPS_LOCATION_DATA.GPS_LOCATION_latitude,GPS_LOCATION_DATA.GPS_LOCATION_longitude);
		}
		if(dbc_handle_mia_RESET(&REST_Info, 1))
		{
			REST_Info.RESET_data=RESET__MIA_MSG.RESET_data;
			LD.setNumber(REST_Info.RESET_data);
			LE.on(1);
		}
	}
}
