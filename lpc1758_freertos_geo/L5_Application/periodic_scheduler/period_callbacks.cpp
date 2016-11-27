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
#include "eint.h"
#include "gpio.hpp"
#include <stdio.h>
#include "gps.hpp"
#include <math.h>
#define 	GPS_CAN_RX_QUEUE_SIZE			16
#define 	GPS_CAN_TX_QUEUE_SIZE			16

uint64_t T1_Left,T1_Right,T1_Center;
uint64_t T2_Right,T2_Left,T2_Center;
int Distance_left,Distance_center,Distance_right =0;

GPIO inputPWPin(P2_1);    			//input-PW Pin--Center
GPIO outputRxPin(P2_0);  			//output-RX Pin--Center
GPIO inputPWPinLeft(P2_3);    		//input-PW Pin--Left
GPIO outputRxPinLeft(P2_2);  		//output-RX Pin--Left
GPIO inputPWPinRight(P2_5);    		//input-PW Pin--right
GPIO outputRxPinRight(P2_4);  		//output-RX Pin--right
//Uart2 &gpsUart;
gpsTask gpstask;
//lsm303_compass lsm303;
bool receivedAck = false;
/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);
char rxBuff[256];
char gpsStr[256];
uint8_t angleXMsb = 0;
uint8_t angleXLsb = 0;
uint8_t angleYMsb = 0;
uint8_t angleYLsb = 0;
uint8_t angleZMsb = 0;
uint8_t angleZLsb = 0;
int X = 0;
int Y = 0;
int Z = 0;
float heading=0;
float Pitch;
float Roll;

float Accx;
float Accy;
float Accz;
int16_t Magx;
int16_t Magy;
int16_t Magz;
float Mag_minx;
float Mag_miny;
float Mag_minz;
float Mag_maxx;
float Mag_maxy;
float Mag_maxz;
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

	//gpstask.init();
	LSM.init();

#if 1
#if 0
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
#endif
#if 0
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
				LD.setNumber(10);
			}

		}

		delay_ms(1);
	}
#endif
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
    LE.toggle(1);
    //compassi2c.getHeading();
#if 0
	if(CAN_is_bus_off(GPS_CAN_BUS))
	{
		CAN_reset_bus(GPS_CAN_BUS);
	}

	u0_dbg_printf("Sending heart beat\n");
    geoSendHeartBeat();
#endif


    angleXMsb = LSM.getXAngleMsb();
	angleXLsb = LSM.getXAngleLsb();
	angleZMsb = LSM.getZAngleMsb();
	angleZLsb = LSM.getZAngleLsb();
	angleYMsb = LSM.getYAngleMsb();
	angleYLsb = LSM.getYAngleLsb();


	Magx = (int16_t)((angleXMsb << 8) | angleXLsb);
	Magy = (int16_t)((angleYMsb << 8) | angleYLsb);
	Magz = (int16_t)((angleZMsb << 8) | angleZLsb);

#if 0
	Mag_minx = -572;
	  Mag_miny = -656;
	  Mag_minz = -486;
	  Mag_maxx = 429;
	  Mag_maxy = 395;
	  Mag_maxz = 535;

	  // use calibration values to shift and scale magnetometer measurements
	  Magx = (Magx-Mag_minx)/(Mag_maxx-Mag_minx)*2-1;
	  Magy = (Magy-Mag_miny)/(Mag_maxy-Mag_miny)*2-1;
	  Magz = (Magz-Mag_minz)/(Mag_maxz-Mag_minz)*2-1;

	  // Normalize acceleration measurements so they range from 0 to 1
	  float accxnorm = Accx/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);
	  float accynorm = Accy/sqrt(Accx*Accx+Accy*Accy+Accz*Accz);
	Pitch = asin(-accxnorm);
	Roll = asin(accynorm/cos(Pitch));


	if(Magx > 0 && Magy >= 0)
	{
		heading = atan2(Magy,Magx);
	}
	else if(Magx < 0)
	{
		heading = 180+atan2(Magy,Magx);
	}
	else if(Magx > 0 && Magy < 0)
	{
		heading = 360+atan2(Magy,Magx);
	}
#endif
	heading = (atan2(Magy,Magx)*180)/M_PI;

	u0_dbg_printf("before :%f\n",heading);
	if (heading < 0)
		heading +=360;

	u0_dbg_printf("%d,%d,%d\n",Magx,Magy,Magz);
	u0_dbg_printf("%f\n",heading);

}

void period_10Hz(uint32_t count)
{
    LE.toggle(2);

    //geoSendHeartBeat();
    //geoSendGpsData();
    gpstask.readGpsData();
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
