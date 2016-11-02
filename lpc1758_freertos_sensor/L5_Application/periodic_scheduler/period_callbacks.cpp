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
#include "string.h"
#include "_can_dbc//generated_can.h"
#include "can.h"
#include <stdio.h>

const uint32_t            RESET__MIA_MS = 30;
const RESET_t              RESET__MIA_MSG = { 2 };

RESET_t reset_cmd_msg = { 0 };

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])

{
    can_msg_t can_msg = { 0 };
    can_msg.msg_id  = mid;
    can_msg.frame_fields.data_len = dlc;
    memcpy(can_msg.data.bytes, bytes, dlc);

    //printf("sending..");
    return CAN_tx(can1, &can_msg, 0);
}


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
    CAN_init(can1,100, 20, 20, 0, 0);
    CAN_bypass_filter_accept_all_msgs();
    CAN_reset_bus(can1);
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

void period_1Hz(uint32_t count)   // transmitter
{
	LE.setAll(0);

	if(CAN_is_bus_off(can1))
	CAN_reset_bus(can1);

     	SENSOR_HEARTBEAT_t sensor_heartbeat={0};
     	sensor_heartbeat.SENSOR_HEARTBEAT_data=1;

	   if (dbc_encode_and_send_SENSOR_HEARTBEAT(&sensor_heartbeat))
	   {
         LD.setNumber(11);
         //printf("Heartbeat");
	    }

	   //     	SENSOR_POWER_SYNC_t sensor_power_sync={0};
	   //     	sensor_power_sync.SENSOR_POWER_SYNC_data=1;

//	   if (dbc_encode_and_send_SENSOR_POWER_SYNC(&sensor_power_sync))
//	  	   {
//	           LD.setNumber(22);
	   //printf("Sync");
//	  	    }

//	LE.toggle(1);
}

void period_10Hz(uint32_t count)  // receiver
{
	can_msg_t can_msg;

	 while (CAN_rx(can1, &can_msg, 0))
	{
	 dbc_msg_hdr_t can_msg_hdr;
	 can_msg_hdr.dlc = can_msg.frame_fields.data_len;
	 can_msg_hdr.mid = can_msg.msg_id;

	dbc_decode_RESET(&reset_cmd_msg, can_msg.data.bytes, &can_msg_hdr);
	 //  printf("message id: %d\n",can_msg.msg_id);
	 }

	  	SENSOR_SONIC_t sensor_data;
	  	sensor_data.SENSORS_SONIC_back=11;
	  	sensor_data.SENSORS_SONIC_front_center=22;
	    sensor_data.SENSORS_SONIC_front_left=33;
	  	sensor_data.SENSORS_SONIC_front_right=44;

	  	if (dbc_encode_and_send_SENSOR_SONIC(&sensor_data))
	  	     {
	  	     LD.setNumber(33);
	  	     //printf("Data");
	  	      }

	      if(dbc_handle_mia_RESET(&reset_cmd_msg, 10))
	     {
	       LE.setAll(1);
	       //printf("MIA occured");
	       LD.setNumber(55);
	      }
	     else
	      {
	      //LD.setNumber(reset_cmd_msg.RESET_data);
	    	// LD.setNumber(75);
	      }

//	      if(dbc_handle_mia_STOP_CAR(&reset_cmd_msg, 10))
//	    	     {
//	    	       LE.setAll(1);
//	    	       //printf("MIA occured");
//	    	       LD.setNumber(55);
//	    	      }
//	    	     else
//	    	      {
//	    	      //LD.setNumber(reset_cmd_msg.RESET_data);
//	    	    	// LD.setNumber(75);
//	    	      }


}

void period_100Hz(uint32_t count)
{
//    LE.toggle(3);
}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
//    LE.toggle(4);
}
