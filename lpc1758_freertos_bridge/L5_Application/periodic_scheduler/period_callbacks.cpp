/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * This contains the period callback functions for the periodic scheduler
 *
 * @warning
 * These callbacks should be used for hard real-time system, and the priority of these
 * tasks are above everything else in the system (above the PRIORITY_CRITICAL).
 * The period functions SHOULD NEVER block and SHOULD NEVER run over their time slot.
 * For example, the 1000Hz take slot runs periodically every 1ms, and whatever you
 * do must be completed within 1ms.  Running over the time slot will reset the system.
 */

#include <stdint.h>
#include "io.hpp"
#include "periodic_callback.h"
#include"bluetooth.hpp"
#include"printf_lib.h"
#include<stdlib.h>
#include<string.h>
#include"printf_lib.h"
#include <stdio.h>
#include "can.h"
#include "../../_can_dbc/generated_can.h"

#define Case_STOP_CAR 						0
#define Case_RESET							1
#define Case_BRIDGE_POWER_SYNC_data 		13
#define Case_RECEIVE_START_ACK				130
#define Case_CURRENT_LOCATION_ACK			170
#define Case_GPS_LOCATION					180



extern char stored_Bluetooth_data[96];
extern char Bluetooth_Buffer[96];

/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);

/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are needed in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_DISPATCHER_TASK_STACK_SIZE_BYTES = (512 * 3);
const GPS_LOCATION_t                  	         	GPS_LOCATION_DATA 	= 	{0};
const BLUETOOTH_DATA_t                           	BLUETOOTH__MIA_MSG 		{0};
const BRIDGE_HEARTBEAT_t                         	BRIDGE_DATA__MIA_MSG 	{0};
const uint32_t            							RESET__MIA_MS 		= 	3;
const RESET_t      									RESET__MIA_MSG		= 	{4};
extern SemaphoreHandle_t Bluetooth_Lat_Lon_Semaphore;

void canBusErrorCallBackRx(uint32_t ibits);

/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
	u0_dbg_printf("This is transmitter/receiver\n");
	if(CAN_init(can1, 100, 256, 256, canBusErrorCallBackRx, NULL))
	{
		u0_dbg_printf("Initialize CAN module\n");
	}
	else
	{
		u0_dbg_printf("unable to initialize CAN module\n");
	}
	CAN_reset_bus(can1);
	CAN_bypass_filter_accept_all_msgs();
	return true; // Must return true upon success
}

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8]);

/// Register any telemetry variables
bool period_reg_tlm(void)
{
    // Make sure "SYS_CFG_ENABLE_TLM" is enabled at sys_config.h to use Telemetry
    return true; // Must return true upon success
}


/**
 * Below are your periodic functions.
 * The argument 'count' is the number of times each periodic task is called.
 */


void period_1Hz(uint32_t count)
{
	BRIDGE_HEARTBEAT_t HEARTBEAT_Data = { 0 };
	BRIDGE_POWER_SYNC_t BRIDGE_POWER_SYNC_Data = {0};
	if (CAN_is_bus_off(can1))
	{
		CAN_reset_bus(can1);
	}
	dbc_encode_and_send_BRIDGE_HEARTBEAT(&HEARTBEAT_Data);
	dbc_encode_and_send_BRIDGE_POWER_SYNC(&BRIDGE_POWER_SYNC_Data);
	 LE.toggle(1);
}

void period_10Hz(uint32_t count)
{
	Bluetooth_Lat_Lon_Semaphore = xSemaphoreCreateBinary();
	BLUETOOTH_DATA_t Bluetooth_Data = { 0 };
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
		if(can_msg_Info.msg_id == Case_STOP_CAR)
			printf("Case_STOP_CAR\n");
		if(can_msg_Info.msg_id == Case_RESET)
			dbc_decode_RESET(&REST_Info, can_msg_Info.data.bytes, &can_msg_hdr);
		if(can_msg_Info.msg_id == Case_CURRENT_LOCATION_ACK)
			printf("Case_CURRENT_LOCATION_ACK\n");
		if(can_msg_Info.msg_id == Case_RECEIVE_START_ACK)
			printf("Case_RECEIVE_START_ACKt\n");
		if(can_msg_Info.msg_id == Case_BRIDGE_POWER_SYNC_data)
			printf("BRIDGE_POWER_SYNC_data\n");
		if(can_msg_Info.msg_id == Case_GPS_LOCATION)
		{
    		dbc_decode_GPS_LOCATION(&GPS_LOCATION_RECEIVE, can_msg_Info.data.bytes, &can_msg_hdr);
    		//u0_dbg_printf("Latitude: %f ,Longitude: %f \n",GPS_LOCATION_DATA.GPS_LOCATION_latitude,GPS_LOCATION_DATA.GPS_LOCATION_longitude);
		}
		if(dbc_handle_mia_RESET(&REST_Info, 1))
		{
			REST_Info.RESET_data=RESET__MIA_MSG.RESET_data;
			LD.setNumber(REST_Info.RESET_data);
			LE.on(1);
		}
	}
	 if (xSemaphoreTake(Bluetooth_Lat_Lon_Semaphore,100))
	 {
		 int i = 0;
		//u0_dbg_printf("\n100 Hz%s\n",stored_Bluetooth_data);
		while(stored_Bluetooth_data[i] != NULL)
		{
			uart_putchar(stored_Bluetooth_data[i]);
			i++;
		}
		Bluetooth_Data.BLUETOOTH_DATA_LAT = 37.123456;
		Bluetooth_Data.BLUETOOTH_DATA_LON = 121.123456;
		dbc_encode_and_send_BLUETOOTH_DATA(&Bluetooth_Data);
	 }
}

void period_100Hz(uint32_t count)
{

}


// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
    LE.toggle(4);
}
