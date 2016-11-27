/*
 * File: master_controller.cpp
 *
 * Implements the functionality for Master Controller.
 *
 *  Created on: 27-Oct-2016
 */

#include <master_controller.h>
#include <can.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include <string.h>

SENSOR_HEARTBEAT_t can_msg_sensor_hb= {0};
MOTORIO_HEARTBEAT_t can_msg_motorio_hb= {0};
BRIDGE_HEARTBEAT_t can_msg_bridge_hb= {0};
GEO_HEARTBEAT_t can_msg_geo_hb= {0};

SENSOR_SONIC_t can_msg_sensor_data = {0};
POWER_SYNC_ACK_t power_sync_ack = {0};
STATE_CAR car_state = INITIAL_STATE;
SENSOR_SONIC_t sensor_data = {0};
checkpoints_data_t checkpoints_data ={0};
COMPASS_DATA_t can_msg_compass ={0};

const uint32_t                             MOTORIO_HEARTBEAT__MIA_MS= 3000;
const MOTORIO_HEARTBEAT_t                  MOTORIO_HEARTBEAT__MIA_MSG={0};
const uint32_t                             SENSOR_HEARTBEAT__MIA_MS= 3000;
const SENSOR_HEARTBEAT_t                   SENSOR_HEARTBEAT__MIA_MSG={0};
const uint32_t                             BRIDGE_HEARTBEAT__MIA_MS= 3000;
const BRIDGE_HEARTBEAT_t                   BRIDGE_HEARTBEAT__MIA_MSG={0};
const uint32_t                             GEO_HEARTBEAT__MIA_MS = 3000;
const GEO_HEARTBEAT_t                      GEO_HEARTBEAT__MIA_MSG={0};
const uint32_t                             SENSOR_SONIC__MIA_MS = 300;
const SENSOR_SONIC_t                       SENSOR_SONIC__MIA_MSG={0};


/* dbc_app_send_can_msg:
 * Functionality: Routine used by the CAN message transmit wrapper function
 * @params: void
 * Return type: status_t (bool)
 */
bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id                = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(can1, &can_msg, 0);
}


/* init_can_master:
 * Functionality: Initialize CAN for master controller
 * @params: void
 *  Return type: status_t (bool)
 */
status_t init_can_master(void)
{
	status_t status = true;
	/*Use can1 for master controller */
	status = CAN_init(can1, 100, 10, 10, NULL, NULL);
	CAN_reset_bus(can1);
	CAN_bypass_filter_accept_all_msgs();

	return status;
}


/* send_power_sync_ack:
 * Functionality: Send power sync acknowledgement to all controllers.
 *                Once acknowledgement is issued, periodic scheduler
 *                tasks are initiated.
 * @params: void
 * Return type: status_t (bool)
 */
status_t send_power_sync_ack(void)
{
	status_t status = false;
	status = dbc_encode_and_send_POWER_SYNC_ACK(&power_sync_ack);
#ifdef DEBUG_PRINTF
	printf("\nPower sync ack sent");
#endif

	return status;
}


/*
 * read_controller_heartbeat:
 * Functionality: Receive the heartbeats of all controllers. Called every 1 second.
 *                The initial set of successful heartbeats from all controllers are
 *                also considered for power sync mechanism.
 * @params: void
 * Return type: status_t (bool)
 */

/**
 * todo: While a heartbeat is a good idea you may not need to have every ECU send a
 * separate distinct heartbeat message. Since they already send data at a certain
 * frequency you can use those to determine MIA. Only the master controller truly needs
 * a dedicated heartbeat message since it does not regularly communicate with the other ECUs.
 */
status_t read_controller_heartbeat(void)
{
	status_t status = false, miastatus = false;
	status_t status_sensor = false, status_motorio = false, status_bridge = false, status_geo = false;
	can_msg_t can_msg;

	/**
	 * todo: NEVER use while loops in RTOS systems. You have no idea how long you will be in this
	 * 			loop. What if you get flooded with messages? This may work with only 5 ECUs on the bus
	 * 			and with a limited number of message IDs that you are listening for, but not in a bigger
	 * 			system. If you need to process received messages more quickly then move this to a faster
	 * 			rate task.
	 */
	while (CAN_rx(can1, &can_msg, 0))
	{
		// Form the message header from the metadata of the arriving message
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;
#ifdef DEBUG_PRINTF
		printf("\nReceived HeartBeat");
#endif
		if(can_msg_hdr.mid == SENSOR_HEARTBEAT_HDR.mid )
		{
			status_sensor = true;
			printf("Received SENSOR HeartBeat\n");
#ifdef DEBUG_PRINTF
			printf("\nReceived SENSOR HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == MOTORIO_HEARTBEAT_HDR.mid )
		{
			status_motorio = true;
#ifdef DEBUG_PRINTF
			printf("\nReceived MOTORIO HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == BRIDGE_HEARTBEAT_HDR.mid )
		{
			status_bridge = true;
#ifdef DEBUG_PRINTF
			printf("\nReceived BRIDGE HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == GEO_HEARTBEAT_HDR.mid )
		{
			status_geo = true;
#ifdef DEBUG_PRINTF
			printf("\nReceived GEO HeartBeat");
#endif
		}
	}

	/* Handle MIA 's for controller heartbeat */
	miastatus = dbc_handle_mia_SENSOR_HEARTBEAT(&can_msg_sensor_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nSensor HeartBeat MIA");
#endif
		LOG_INFO("\nSensor heartBeat MIA Occurred");
	}
	miastatus = dbc_handle_mia_MOTORIO_HEARTBEAT(&can_msg_motorio_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nMotorio HeartBeat MIA");
#endif
		LOG_INFO("\nMotorio heartBeat MIA Occurred\n");
	}
	miastatus = dbc_handle_mia_BRIDGE_HEARTBEAT(&can_msg_bridge_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nBridge HeartBeat MIA");
#endif
		LOG_INFO("\nBridge heartBeat MIA Occurred");
	}
	miastatus = dbc_handle_mia_GEO_HEARTBEAT(&can_msg_geo_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nGeo HeartBeat MIA");
#endif
		LOG_INFO("\nGeo heartBeat MIA Occurred");
	}

	status = status_sensor & status_motorio & status_bridge & status_geo;
	return status;
}


/*
 * receive_sensor_data:
 * Functionality: Receive the CAN data sent by the sensors
 * @params: void
 * Return type: SENSOR_SONIC_t
 */
SENSOR_SONIC_t receive_sensor_data(void)
{
	can_msg_t can_msg;
	status_t status = true, miastatus = false;

	if(CAN_rx(can1, &can_msg, 0))
	{
		// Form the message header from the metadata of the arriving message
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;

		if(can_msg_hdr.mid == SENSOR_SONIC_HDR.mid )
		{
			LD.setNumber(0);
			status = dbc_decode_SENSOR_SONIC(&can_msg_sensor_data, can_msg.data.bytes, &can_msg_hdr);
			if(!status)
				printf("\nCould not read sensor values");
#ifdef DEBUG_PRINTF
			printf("Received SENSOR DATA\n");
			printf("Sensor Data:\n Front Left: %u \n ", can_msg_sensor_data.SENSORS_SONIC_front_left);
			printf("Front Center: %u \n", can_msg_sensor_data.SENSORS_SONIC_front_center);
			printf("Front Right: %u \n", can_msg_sensor_data.SENSORS_SONIC_front_right);
			printf("Back: %u\n", can_msg_sensor_data.SENSORS_SONIC_back);
#endif
		}
	}
	/**
	 * todo: try to avoid magic numbers. You use 100ms here because you know that this task is
	 * 			going to run in the 10Hz task, but what happens if it gets moved in the future.
	 * 			If someone new is maintaining the code they may miss this. Use something like
	 * 			"10HZ_MS"
	 */
	miastatus = dbc_handle_mia_SENSOR_SONIC(&can_msg_sensor_data, 100);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("Sensor Data MIA\n");
#endif
		LD.setNumber(99);
		LOG_INFO("Sensor Data MIA Occurred\n");
	}

	return can_msg_sensor_data;
}


/*
 * obstacle_avoidance_and_drive:
 * Functionality: Algorithm to control the motor based on the sensor values.
 *                Obstacles are indicated by the sensor values.
 * @params: void
 * Return type: status_t (bool)
 */
status_t avoid_obstacle_and_drive(void)
{
	status_t status = true;
	//SENSOR_SONIC_t sensor_data = {0};
	MOTORIO_DIRECTION_t motor_cmd = {0};

	//sensor_data = receive_sensor_data();
	uint16_t sensor_left = sensor_data.SENSORS_SONIC_front_left;
	uint16_t sensor_center = sensor_data.SENSORS_SONIC_front_center;
	uint16_t sensor_right = sensor_data.SENSORS_SONIC_front_right;

	/**
	 * todo: don't forget you will need to be able to back up to navigate around large obstacles
	 */
	if((sensor_left <= 60) && (sensor_center <= 60) && (sensor_right <= 60))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if(((sensor_left > 60) && (sensor_left <= 120)) && (sensor_right > 120))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_RIGHT;
	}
	else if(((sensor_right > 60) && (sensor_right <= 120)) && (sensor_left > 120))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_LEFT;
	}
	else if(((sensor_left > 60) && (sensor_left <= 120)) &&
			((sensor_center > 60) && (sensor_center <= 120)) &&
			((sensor_right > 60) && (sensor_right <= 120)))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
#ifdef DEBUG_PRINTF
	printf("\nSensor value: Left = %u | Center = %u | Right = %u", sensor_left, sensor_center, sensor_right);
	printf("\nMotor value: Speed = %u | Direction = %u | Turn = %u",motor_cmd.MOTORIO_DIRECTION_speed, motor_cmd.MOTORIO_DIRECTION_direction, motor_cmd.MOTORIO_DIRECTION_turn);
#endif

	status = dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);

	return status;
}


/*
 * cmd_from_app:
 * Functionality: Receive Start and Stop Commands from the Application
 * @params: void
 * Return type: status_t (bool)
 */
status_t cmd_from_app(void)
{
	status_t status = false;
	can_msg_t can_msg;
	/**
	 * todo: NO while loops.
	 */
	while (CAN_rx(can1, &can_msg, 0))
	{
		if(can_msg.msg_id == START_CMD_APP_HDR.mid)
		{
			/* Start the  motor */
			MOTORIO_DIRECTION_t motor_cmd;
			motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			status = dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
		}
		else if(can_msg.msg_id == STOP_CMD_APP_HDR.mid)
		{
			/* Stop the car */
			MOTORIO_DIRECTION_t motor_cmd;
			motor_cmd.MOTORIO_DIRECTION_speed = STOP;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			status = dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
		}
	}

	return status;
}

void drive_car()
{
	if(car_state == START_CAR)
	{
		/* Sensor data is populated in 100Hz ,get the sensor data and avoid obstacle*/
		avoid_obstacle_and_drive();
	}
	else if(car_state == SEND_CHECKPOINTS)
	{
		/* Send the first Location Update to Geo Controller */
		NEXT_CHECKPOINT_DATA_t geo_data = {0};
		geo_data.NEXT_CHECKPOINT_DATA_latitude = checkpoints_data.latitide[checkpoints_data.geo_update_pos];
		geo_data.NEXT_CHECKPOINT_DATA_longitude = checkpoints_data.longitude[checkpoints_data.geo_update_pos];
		checkpoints_data.geo_update_pos++;
		dbc_encode_and_send_NEXT_CHECKPOINT_DATA(&geo_data);
		car_state = NAVIGATING;

	}
	else if(car_state == NAVIGATING)
	{
		/* Check the current location distance from next checkpoint*/

		/* If reached the next checkpoint send the next checkpoint
		 * car_state = SEND_CHECKPOINTS */

		/* Check the reading of bearing from compass and send motor commands
    	     can_msg_compass */


	}
	else if(car_state == STOP_CAR)
	{
		/* Stop the car */
		MOTORIO_DIRECTION_t motor_cmd;
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
		dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
	}
}
/* Receive Data from CAN Bus*/
void receive_data_from_can(void)
{
	can_msg_t can_msg;
	dbc_msg_hdr_t can_msg_hdr;

	while(CAN_rx(can1, &can_msg, 0))
	{
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;
		/* Command from App to start*/
		if(can_msg.msg_id ==  START_CMD_APP_HDR.mid)
		{
			car_state = START_CAR;
#ifdef DEBUG_PRINTF
			printf("RX START CMD\n");
#endif
		}
		else if(can_msg.msg_id == SENSOR_SONIC_HDR.mid)
		{
			dbc_decode_SENSOR_SONIC(&sensor_data, can_msg.data.bytes, &can_msg_hdr);
#ifdef DEBUG_PRINTF
			printf("RX SENSOR DATA\n");
#endif
		}
		else if(can_msg.msg_id == BRIDGE_TOTAL_CHECKPOINT_HDR.mid)
		{
			BRIDGE_TOTAL_CHECKPOINT_t can_msg_bridge ={0};
			dbc_decode_BRIDGE_TOTAL_CHECKPOINT(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.tot_points = can_msg_bridge.BRIDGE_TOTAL_CHECKPOINT_NUMBER;
#ifdef DEBUG_PRINTF
			printf("RX Bridge Checkpoints\n");
#endif
		}
		else if(can_msg.msg_id == BLUETOOTH_DATA_HDR.mid)
		{
			/* Collect All the checkpoint Data */
			BLUETOOTH_DATA_t can_msg_bridge = {0};
			dbc_decode_BLUETOOTH_DATA(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.latitide[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LAT;
			checkpoints_data.longitude[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LON;
			checkpoints_data.position++;

			if(checkpoints_data.position == checkpoints_data.tot_points)
			{
				car_state = SEND_CHECKPOINTS;
#ifdef DEBUG_PRINTF
printf("All Checkpoint Data Received\n");
#endif
			}

#ifdef DEBUG_PRINTF
			printf("RX Bridge Checkpoints\n");
#endif
		}
		else if(can_msg.msg_id == COMPASS_DATA_HDR.mid)
		{
			dbc_decode_COMPASS_DATA(&can_msg_compass,can_msg.data.bytes, &can_msg_hdr);
#ifdef DEBUG_PRINTF
			printf("RX COMPASS DATA\n");
#endif
		}
		else if(can_msg.msg_id == GPS_LOCATION_HDR.mid)
		{
			GPS_LOCATION_t can_msg_loc = {0};
			dbc_decode_GPS_LOCATION(&can_msg_loc,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.cur_loc_lat = can_msg_loc.GPS_LOCATION_latitude;
			checkpoints_data.cur_loc_long = can_msg_loc.GPS_LOCATION_longitude;
#ifdef DEBUG_PRINTF
			printf("GET CURRENT LOCATION DATA\n");
#endif
		}
		else if(can_msg.msg_id == STOP_CMD_APP_HDR.mid)
		{
			car_state = STOP_CAR;
#ifdef DEBUG_PRINTF
			printf("RX STOP CAR CMD\n");
#endif
		}

	}
}


/*
 * send_bridge_ack:
 * Functionality: Send acknowledgement to the bridge to indicate that master
 * 				  is ready to receive data
 * @params void
 * Return type: status_t (bool)
 */
status_t send_bridge_ack(void)
{
	status_t status = true;

	RECEIVE_START_ACK_t ack_data = {0};
	dbc_encode_and_send_RECEIVE_START_ACK(&ack_data);
#ifdef DEBUG_PRINTF
	printf("Send ACK to Bridge to receive data\n");
#endif

	return status;
}


/*
 * is_bus_off:
 * Functionality: Check if bus if in the OFF state
 * @params void
 * Return type: void
 */
void is_bus_off(void)
{
	if(CAN_is_bus_off(can1))
	{
#ifdef DEBUG_PRINTF
		printf("\nBus is Off");
#endif
		CAN_reset_bus(can1);
	}
}
