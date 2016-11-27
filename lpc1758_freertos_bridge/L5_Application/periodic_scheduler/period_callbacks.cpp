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

// todo: fix the spacing for these includes.

#define BluetoothPeriodicSchedular 1

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

void canBusErrorCallBackRx(uint32_t ibits);
// todo: this declaration is not necessary since it is in bluetooth.hpp

/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
#if BluetoothPeriodicSchedular
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
#if BluetoothPeriodicSchedular
	BRIDGE_HEARTBEAT_t HEARTBEAT_Data = { 0 };
	BRIDGE_POWER_SYNC_t BRIDGE_POWER_SYNC_Data = {0};

/**
 * todo: not necessary to do this in 1Hz task since you have bus_off_cb function defined.
 * You can do this there instead.
 */
	if (CAN_is_bus_off(can1))
	{
		CAN_reset_bus(can1);
		LE.toggle(1);
	}
	dbc_encode_and_send_BRIDGE_HEARTBEAT(&HEARTBEAT_Data);
	dbc_encode_and_send_BRIDGE_POWER_SYNC(&BRIDGE_POWER_SYNC_Data);
#endif
}

void period_10Hz(uint32_t count)
{
#if BluetoothPeriodicSchedular
	Can_Receive_ID_Task();
	Check_Start_STOP_Condition();
#endif
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
