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
#include <stdio.h>
#include "file_logger.h"
#include "sensor.hpp"
#include <tasks.hpp>

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
		CAN_reset_bus(can1);
		LD.setNumber(44);

		eint3_enable_port2(3, eint_falling_edge, Calculate_Distance_left); //Left Sonar
		eint3_enable_port2(1, eint_falling_edge, Calculate_Distance_center); //Middle Sonar
		eint3_enable_port2(5, eint_falling_edge, Calculate_Distance_right); //Right Sonar
		//eint3_enable_port2(3, eint_falling_edge, Calculate_Distance_rear); //Rear Sonar*/

		LeftRX.setAsOutput(); // set p2.2 as an output pin to enable or disable Left Sonar
		CenterRX.setAsOutput();//set p2.0 as an output pin to enable or disable Center Sonar
		RightRX.setAsOutput();//set p2.4 as an output pin to enable or disable Right Sonar
		//BackRX.setAsOutput();//set p2.6 as an output pin to enable or disable Rear Sonar

		delay_ms(251); //250ms after powerup sensor is ready to receive commands
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
			printf("Bus off!!!");
		}

}

void period_10Hz(uint32_t count)
{
	//LE.toggle(2);
		Sensor();
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
