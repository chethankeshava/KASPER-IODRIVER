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
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "io.hpp"
#include "periodic_callback.h"
#include "gps.hpp"
#include "compass.hpp"
#include "printf_lib.h"
#include "can.h"
#include "utilities.h"
#include "../_can_dbc/generated_can.h"
#include "eint.h"
#include "gpio.hpp"
#include "geo.hpp"
#include "gps.hpp"


#define 	GPS_CAN_RX_QUEUE_SIZE			16
#define 	GPS_CAN_TX_QUEUE_SIZE			16


geoTask geotask;
bool receivedAck = false;
float tempheading =0.0;
/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);
bool newChkPointRecvd = false;

/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are needed in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_DISPATCHER_TASK_STACK_SIZE_BYTES = (512 * 3);



/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
	can_msg_t msg;

//	LSM.init();
//	LSM_ACCL.init();

#if 0
	// Initialize compass module
	if(!(compassi2c.init()))
	{
		u0_dbg_printf("Device not present\n");
		//return false;
	}
#endif
	// Initialize GEO module
	if(!geotask.init())
	{
		u0_dbg_printf("GEO not initialized\n");
		return false;
	}

	// Initialize CAN bus
	if(CAN_init(GEO_CAN_BUS, 100, GPS_CAN_RX_QUEUE_SIZE, GPS_CAN_TX_QUEUE_SIZE, NULL, NULL))
	{
		u0_dbg_printf("Initialize CAN module\n");
	}
	else
	{
		u0_dbg_printf("unable to initialize CAN module\n");
	}

	// Put CAN bus in normal mode
	CAN_reset_bus(GEO_CAN_BUS);

	// Receive all messages
	CAN_bypass_filter_accept_all_msgs();

	// Power sync ACK
#if 0
	while(!receivedAck)
	{
		if(CAN_is_bus_off(GEO_CAN_BUS))
		{
			CAN_reset_bus(GEO_CAN_BUS);
		}

		geoSendHeartBeat();

		if (CAN_rx(can1, &msg, 0))
		{
			if(msg.msg_id == POWER_SYNC_ACK_HDR.mid)
			{
				u0_dbg_printf("Received ACK from master\n");
				receivedAck = true;
				LD.setNumber(10);
			}

		}

		delay_ms(1);
	}
#endif

	return true; // Must return true upon success
}

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

	if(CAN_is_bus_off(GEO_CAN_BUS))
	{
		CAN_reset_bus(GEO_CAN_BUS);
	}

	//geotask.readGpsData();
	geotask.sendGpsData();
	geotask.sendCompassData();
	//compassi2c.getHeading(&tempheading);

}

void period_10Hz(uint32_t count)
{
	NEXT_CHECKPOINT_DATA_t checkPointData;
	can_msg_t msg;

	geotask.readGpsData();
	//geotask.sendCompassData();
	//u0_dbg_printf("#");
	//geotask.readGpsData();
	//geotask.sendCompassData();

	while(CAN_rx(GEO_CAN_BUS, &msg, 10))	// 100ms timeout for receive
	{
		if(msg.msg_id == NEXT_CHECKPOINT_DATA_HDR.mid)
		{
			dbc_msg_hdr_t can_msg_hdr;
			can_msg_hdr.dlc = msg.frame_fields.data_len;
			can_msg_hdr.mid = msg.msg_id;

			u0_dbg_printf("Received Next way point data from master\n");

			dbc_decode_NEXT_CHECKPOINT_DATA(&checkPointData, msg.data.bytes, &can_msg_hdr );
			geotask.setChkPointData(checkPointData.NEXT_CHECKPOINT_DATA_latitude,checkPointData.NEXT_CHECKPOINT_DATA_longitude);
			u0_dbg_printf("lat:%f",checkPointData.NEXT_CHECKPOINT_DATA_latitude);
			u0_dbg_printf("lon:%f",checkPointData.NEXT_CHECKPOINT_DATA_longitude);
			newChkPointRecvd = true;
		}
	}

}

void period_100Hz(uint32_t count)
{
    LE.toggle(3);

}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
    LE.toggle(4);
}
