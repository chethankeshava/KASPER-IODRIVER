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
#include "gps.hpp"
#include "compass.hpp"
#include "printf_lib.h"
#include "can.h"
#include "../_can_dbc/generated_can.h"

#define 	GPS_CAN_RX_QUEUE_SIZE			16
#define 	GPS_CAN_TX_QUEUE_SIZE			16


bool receivedAck = false;
/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);

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
	can_msg_t can_msg={0};

	if(!(compassi2c.init()))
	{
		u0_dbg_printf("Device not present\n");
		return false;
	}

	if(CAN_init(GPS_CAN_BUS, 100, GPS_CAN_RX_QUEUE_SIZE, GPS_CAN_TX_QUEUE_SIZE, NULL, NULL))
	{
		u0_dbg_printf("Initialize CAN module\n");
	}
	else
	{
		u0_dbg_printf("unable to initialize CAN module\n");
	}

	CAN_reset_bus(GPS_CAN_BUS);
	CAN_bypass_filter_accept_all_msgs();

    /**
     * todo: NO while loops in RTOS systems.
     */
	while(!receivedAck)
	{
		if(CAN_is_bus_off(GPS_CAN_BUS))
		{
			CAN_reset_bus(GPS_CAN_BUS);
		}

		geoSendHeartBeat();

		if (CAN_rx(can1, &can_msg, 0))
		{
			if(can_msg.msg_id == POWER_SYNC_ACK_HDR.mid)
			{
				u0_dbg_printf("Received ACK from master\n");
				receivedAck = true;

			}


		}
	}


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
    LE.toggle(1);
    /**
     * todo: will running this at 1Hz be fast enough? If you are parsing GPS data at 10Hz
     *          maybe you should consider running your heading calculations at the same rate.
     */
    compassi2c.getHeading();

	if(CAN_is_bus_off(GPS_CAN_BUS))
	{
		CAN_reset_bus(GPS_CAN_BUS);
	}
	u0_dbg_printf("Sending heart beat\n");
    geoSendHeartBeat();
}

void period_10Hz(uint32_t count)
{
    LE.toggle(2);
    geoSendHeartBeat();
    geoSendGpsData();
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
