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
#include "can.h"
#include "../_can_dbc/generated_can.h"
#include "string.h"
#include "stdio.h"
#include "motor.hpp"
#include "lpc_pwm.hpp"
#include "lcd.hpp"
#include "gpio.hpp"
#include "eint.h"
#include "file_logger.h"

extern int speed_temp;
int m_rpmCounter=0;
int prev_rpmCounter=0;
extern int s_back;

extern float latitide;
extern float longitude;
/**
 * todo: avoid global variables.
 */
int pwm_input = 6.4;
int rpm;
/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);
char lcdBuffer[128]={};
/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are ne44eded in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_DISPATCHER_TASK_STACK_SIZE_BYTES = (512 * 3);

void counter(void)
{
	//puts("hi");
	//printf("#");
	m_rpmCounter++;
}

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id= mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	// printf("sending\n");
	return CAN_tx(can1, &can_msg, 0);

}
/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
	rpm = 0;
	dcmotor_init();
	CAN_init(can1, 100, 5, 5, 0, 0);
	CAN_reset_bus(can1);

	GPIO inputPin(P2_5);
	inputPin.setAsInput();
	inputPin.enablePullDown();
	eint3_enable_port2(5, eint_rising_edge,counter);

	//RX PART
	CAN_bypass_filter_accept_all_msgs();
	CAN_reset_bus(can1);

//	logger_init(PRIORITY_HIGH); // initilize the logger
	// logger_send_flush_request();
	//printf("Number of buffers--->%u\n",logger_get_num_buffers_watermark());
	// printf("get_highest_file_write_time_ms--->%u\n",logger_get_highest_file_write_time_ms());
	// printf("get_blocked_call_count-->%u\n",logger_get_blocked_call_count());
	// printf("logged_call_count-->%u\n",logger_get_logged_call_count(log_info));


	lcdInit();
	lcdWriteObj(LCD_OBJ_FORM,LCD_FORM0_INDEX,0);
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

	lcdWriteObj(LCD_OBJ_METER,LCD_SPEED_METER_INDEX,m_rpmCounter);
#if 1
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_FRONT_LEFT_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_left);
	lcdWriteStr(LCD_FRONT_LEFT_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_center);
	lcdWriteStr(LCD_FRONT_CENTER_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_right);
	lcdWriteStr(LCD_FRONT_RIGHT_SENSOR_INDEX,lcdBuffer);
	//sprintf(lcdBuffer,"%d",s_);
	//lcdWriteStr(LCD_BACK_SENSOR_INDEX,lcdBuffer);
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_RPM_STRING_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",latitide);
	lcdWriteStr(LCD_LATITUDE_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",longitude);
	lcdWriteStr(LCD_LONGITUDE_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",bearing);
	lcdWriteStr(LCD_BEARING_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",heading);
	lcdWriteStr(LCD_HEADING_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",distance);
	lcdWriteStr(LCD_DISTANCE_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",m_rpmCounter);
	lcdWriteStr(LCD_SPEED_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_back);
	lcdWriteStr(LCD_BACK_SENSOR_INDEX,lcdBuffer);

#endif

	if(CAN_is_bus_off(can1))
	{
		puts(" Bus OFF ");
		LD.setNumber(0);
		CAN_reset_bus(can1);
	}


//	LOG_INFO(" %d ",m_rpmCounter);

//		printf(" %d ",m_rpmCounter);
//		prev_rpmCounter=m_rpmCounter;
//
//	m_rpmCounter=0;






}

void period_10Hz(uint32_t count)
{

	drive_car(count);
	//drive_car();
	//	drive_with_feedback();

	if(SW.getSwitch(1))
	{
		dc_stop();
	}
	else if(SW.getSwitch(2))
	{
		dc_accelerate(6.4);
	}
	else if(SW.getSwitch(3))
	{
		dc_accelerate(6.2);
	}
	else if(SW.getSwitch(4))
	{
		dc_accelerate(7.6);
	}
}

void period_100Hz(uint32_t count)
{
	if(count%100 == 0)
	{
		prev_rpmCounter=m_rpmCounter;
		m_rpmCounter=0;
		printf(" %d ",prev_rpmCounter);
	}

}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
	//LE.`toggle(4);
}
