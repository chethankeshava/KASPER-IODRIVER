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



extern int white_patch_count;
float latitide = 37.322993;
float longitude = -121.883200;
int white_value,max_light_value,min_light_value;
int temp_count;
/**
 * todo: avoid global variables.
 */
int pwm_input = 6.4;
int rpm;
extern int tilt_y;
extern int dc_pwm;
/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);
char lcdBuffer[128]={};
/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are needed in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_DISPATCHER_TASK_STACK_SIZE_BYTES = (512 * 3);


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

	//RX PART
	CAN_bypass_filter_accept_all_msgs();
	CAN_reset_bus(can1);


	white_value = 2300;
	max_light_value = 2000;
	min_light_value = 7000;
	temp_count = 0;

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

	lcdWriteObj(LCD_OBJ_LED_DIGITS,LCD_LEDDIGIT_INDEX,white_patch_count);
#if 1
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_FRONT_LEFT_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_left);
	lcdWriteStr(LCD_FRONT_LEFT_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_center);
	lcdWriteStr(LCD_FRONT_CENTER_SENSOR_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%d",s_right);
	lcdWriteStr(LCD_FRONT_RIGHT_SENSOR_INDEX,lcdBuffer);
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_RPM_STRING_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",latitide);
	lcdWriteStr(LCD_LATITUDE_INDEX,lcdBuffer);
	sprintf(lcdBuffer,"%f",longitude);
	lcdWriteStr(LCD_LONGITUDE_INDEX,lcdBuffer);
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_RPM_STRING_INDEX,lcdBuffer);
	//sprintf(lcdBuffer,"%d",white_patch_count);
	//lcdWriteStr(LCD_RPM_STRING_INDEX,lcdBuffer);
	//rpm_sensor();
#endif
	if(CAN_is_bus_off(can1))
	{
		puts(" Bus OFF ");
		CAN_reset_bus(can1);
	}

	//printf(" %d \n",white_patch_count);

	white_patch_count=0;
	//int   tilt_x = AS.getX();

	//int   tilt_z = AS.getZ();
	//printf("x-->%d  y-->%d z-->%d\n", tilt_x,tilt_y,tilt_z);



}

void period_10Hz(uint32_t count)
{
	drive_car();
	drive_motor();

	//printf(" %d ",count);
	//sprintf(lcdBuffer,"%d",rpm);
	//lcdWriteStr(LCD_RPM_STRING_INDEX,lcdBuffer);

	if(SW.getSwitch(1))
	{
		dc_stop();
	}
	else if(SW.getSwitch(2))
	{
		//	puts("hey");
		dc_accelerate(6.4);
	}
	else if(SW.getSwitch(3))
	{
		dc_accelerate(6.2);
	}
	else if(SW.getSwitch(4))
	{
		dc_accelerate(6.0);
	}
	//rx_rpm();



}

void period_100Hz(uint32_t count)
{
	LD.setNumber(white_patch_count);
	if(rpm_sensor())
//		rpm++;

	if(count%500==0)
	{
		//	printf("RPM = %d\n",rpm);
//		rpm = 0;
	}
	if(count%100==0)
	{
		white_value = (max_light_value + min_light_value)/2;
		max_light_value = 0;
		min_light_value = 2500;
	}
}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
	//LE.toggle(4);
}
