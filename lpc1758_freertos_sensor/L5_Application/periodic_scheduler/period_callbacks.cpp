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
#include "soft_timer.hpp"
#include "lpc_timers.h"
#include "utilities.h"
#include <math.h>
#include "gpio.hpp"
#include "eint.h"

GPIO inputPWPin(P2_1);    //input-PW Pin--Center
GPIO outputRxPin(P2_0);  //output-RX Pin--Center
GPIO inputPWPinLeft(P2_3);    //input-PW Pin--Left
GPIO outputRxPinLeft(P2_2);  //output-RX Pin--Left
GPIO inputPWPinRight(P2_5);    //input-PW Pin--right
GPIO outputRxPinRight(P2_4);  //output-RX Pin--right

SENSOR_SONIC_t sensor_msg= { 0 };

	uint64_t T1;
	uint64_t T2_Right,T2_Left,T2_Center;
	int Distance_left,Distance_center,Distance_right =0;

	bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
	{
	    can_msg_t can_msg = { 0 };
	    can_msg.msg_id  = mid;
	    can_msg.frame_fields.data_len = dlc;
	    memcpy(can_msg.data.bytes, bytes, dlc);
	   printf("sending......\n");
	    return CAN_tx(can1, &can_msg, 0);
	}

const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);

/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are needed in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_DISPATCHER_TASK_STACK_SIZE_BYTES = (512 * 3);

void eintCallbackright_Rise()
{
	T1=sys_get_uptime_us();
//	printf("T1: %d\n",T1);
  //Note down the system start time for right sensor ( start_right_time)
}

void eintCallbackright_Fall_Center()
{
	T2_Center=sys_get_uptime_us();
  //Note down the system start time for right sensor ( start_right_time)
//	printf("T2: %d\n",T2);
	Distance_center=(T2_Center-T1)/147;

	sensor_msg.SENSORS_SONIC_front_center=Distance_center;

//	printf("Distance Center: %d    ",Distance_center);
	//printf("Distance Center: %d ",sensor_msg.SENSORS_SONIC_front_center);
}

void eintCallbackright_Fall_Left()
{
	T2_Left=sys_get_uptime_us();
  //Note down the system start time for right sensor ( start_right_time)
//	printf("T2: %d\n",T2);
	Distance_left=(T2_Left-T1)/147;

	sensor_msg.SENSORS_SONIC_front_left=Distance_left;

	//printf("Distance Left: %d   ",Distance_left);
	//printf("Distance Left: %d ",sensor_msg.SENSORS_SONIC_front_left);
}

void eintCallbackright_Fall_Right()
{
	T2_Right=sys_get_uptime_us();
  //Note down the system start time for right sensor ( start_right_time)
//	printf("T2: %d\n",T2);
	Distance_right=(T2_Right-T1)/147;

	sensor_msg.SENSORS_SONIC_front_right=Distance_right;

	//printf("Distance Right: %d   \n",Distance_right);
	//printf("Distance Right: %d\n",sensor_msg.SENSORS_SONIC_front_right);
}

/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
	//printf("init\n");
	CAN_init(can1,100, 20, 20, 0, 0);
		CAN_reset_bus(can1);

	inputPWPin.setAsInput();
	inputPWPin.enablePullDown();
	outputRxPin.setAsOutput();
	outputRxPin.setHigh();

	inputPWPinLeft.setAsInput();
	inputPWPinLeft.enablePullDown();
	outputRxPinLeft.setAsOutput();
	outputRxPinLeft.setHigh();

	inputPWPinRight.setAsInput();
	inputPWPinRight.enablePullDown();
	outputRxPinRight.setAsOutput();
	outputRxPinRight.setHigh();

	eint3_enable_port2(3, eint_rising_edge, eintCallbackright_Rise);  //Interrupt function callback left
	eint3_enable_port2(3, eint_falling_edge, eintCallbackright_Fall_Left); //Interrupt function callback left
	eint3_enable_port2(1, eint_rising_edge, eintCallbackright_Rise);  //Interrupt function callback center
	eint3_enable_port2(1, eint_falling_edge, eintCallbackright_Fall_Center); //Interrupt function callback center
	eint3_enable_port2(5, eint_rising_edge, eintCallbackright_Rise);  //Interrupt function callback right
	eint3_enable_port2(5, eint_falling_edge, eintCallbackright_Fall_Right); //Interrupt function callback right

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
   // LE.toggle(1);
	 if(CAN_is_bus_off(can1))
		  {
			  CAN_reset_bus(can1);
			  printf("BUS is off!!!!!!");
		  }
}

void period_10Hz(uint32_t count)
{
	// LE.toggle(2);
	if(dbc_encode_and_send_SENSOR_SONIC(&sensor_msg))
    {
   LD.setNumber(88);
   printf("values:");
	  printf("center:%d ",sensor_msg.SENSORS_SONIC_front_center);
	   printf("left:%d ",sensor_msg.SENSORS_SONIC_front_left);
	   printf("right:%d\n",sensor_msg.SENSORS_SONIC_front_right);
    }
}

void period_100Hz(uint32_t count)
{
    //LE.toggle(3);
}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
   // LE.toggle(4);
}
