/*
 * File: master_controller.cpp
 *
 * Implements the functionality for Master Controller.
 *
 *	Author: Aajna Karki & Spoorthi Mysore Shivakumar
 
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

SENSOR_SONIC_t can_msg_sensor_data = {0};
POWER_SYNC_ACK_t power_sync_ack = {0};
STATE_CAR car_state = START_CAR;

static status_t status_sensor = true, status_motorio = true, status_bridge = true, status_geo = true;

const uint32_t                             SENSOR_SONIC__MIA_MS = 300;
const SENSOR_SONIC_t                       SENSOR_SONIC__MIA_MSG={0};

SENSOR_SONIC_t sensor_data = {0};
checkpoints_data_t checkpoints_data ={0};
COMPASS_DATA_t can_msg_compass ={0};

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
	MOTORIO_DIRECTION_t motor_cmd = {0};
	static MOTORIO_DIRECTION_t prev_motor_cmd = {0};

	uint16_t sensor_left = sensor_data.SENSORS_SONIC_front_left;
	uint16_t sensor_center = sensor_data.SENSORS_SONIC_front_center;
	uint16_t sensor_right = sensor_data.SENSORS_SONIC_front_right;


	uint8_t ls_range = (sensor_left <= 45) ? NEAR : sensor_left <= 100 ? MEDIUM : FAR;
	uint8_t rs_range = (sensor_right <= 45) ? NEAR : sensor_right <= 100 ? MEDIUM : FAR;
	uint8_t centre_range = (sensor_center <= 45) ? NEAR : sensor_center <= 100 ? MEDIUM : FAR;

	// TODO: As Calvin has rightly commented, we need to handle backing up
	if(centre_range == NEAR)
	{
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((ls_range == NEAR) && (centre_range == NEAR) && (rs_range == NEAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((ls_range == MEDIUM) && (centre_range != NEAR) && (rs_range == FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = SLIGHT_RIGHT;
	}
	else if((ls_range == FAR) && (centre_range != NEAR) && (rs_range == MEDIUM))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = SLIGHT_LEFT;
	}
	else if((ls_range == NEAR) && (rs_range != NEAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_RIGHT;
	}
	else if((ls_range != NEAR) && (rs_range == NEAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_LEFT;
	}
	else if((ls_range == MEDIUM) || (centre_range == MEDIUM) || (rs_range == MEDIUM))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}

#ifdef DEBUG_PRINTF
	printf("\nSensor value: Left = %u | Center = %u | Right = %u", sensor_left, sensor_center, sensor_right);
	printf("\nMotor value: Speed = %d | Direction = %d | Turn = %d",motor_cmd.MOTORIO_DIRECTION_speed, motor_cmd.MOTORIO_DIRECTION_direction, motor_cmd.MOTORIO_DIRECTION_turn);
#endif

	status = dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
	prev_motor_cmd = motor_cmd;

	return status;
}

/*
 * drive_car:
 * Functionality: Based on the state of the car, the behaviour of the car 
 *                is controlled.
 * @params: void
 * Return type: void
 */
void drive_car()
{
	/*
	 * Being handled in 100Hz as soon as sensor data is read
	 */
	if(status_sensor && status_motorio && status_bridge && status_geo)
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
			geo_data.NEXT_CHECKPOINT_DATA_latitude = checkpoints_data.latitude[checkpoints_data.geo_update_pos];
			geo_data.NEXT_CHECKPOINT_DATA_longitude = checkpoints_data.longitude[checkpoints_data.geo_update_pos];
			checkpoints_data.geo_update_pos++;
			dbc_encode_and_send_NEXT_CHECKPOINT_DATA(&geo_data);
			car_state = NAVIGATING;

		}

		else if(car_state == NAVIGATING)
		{
			// TODO: Navigation algorithm in progress
			/* Check the current location distance from next checkpoint*/
			/* Check the reading of bearing from compass and send motor commands
    	  can_msg_compass */
		}

		else if(car_state == STOP_CAR)
		{
			/* Stop the car */
			MOTORIO_DIRECTION_t motor_cmd = {0};
			motor_cmd.MOTORIO_DIRECTION_speed = STOP;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
		}
	}
}


/*
 * receive_data_from_can:
 * Functionality: Monitors the availability of any CAN message on the CAN bus
 *                and parses it appropriately
 * @params: void
 * Return type: void
 */
void receive_data_from_can(void)
{

	can_msg_t can_msg;
	dbc_msg_hdr_t can_msg_hdr;

	while(CAN_rx(can1, &can_msg, 0))
	{
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;

		if(can_msg.msg_id == MOTOR_POWER_SYNC_HDR.mid)
		{
			status_motorio = true;
		}

		if(can_msg.msg_id == SENSOR_POWER_SYNC_HDR.mid)
		{
			status_sensor = true;
		}

		if(can_msg.msg_id == BRIDGE_POWER_SYNC_HDR.mid)
		{
			status_bridge = true;
		}

		if(can_msg.msg_id == GEO_POWER_SYNC_HDR.mid)
		{
			status_geo = true;
		}

		if(status_sensor && status_motorio && status_bridge && status_geo)
		{
			dbc_encode_and_send_POWER_SYNC_ACK(&power_sync_ack);
		}

		/* Command from App to start*/
		if(can_msg.msg_id ==  START_CMD_APP_HDR.mid)
		{
			car_state = START_CAR;
			MOTORIO_DIRECTION_t motor_cmd;
			motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
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
			BRIDGE_TOTAL_CHECKPOINT_t can_msg_bridge = {0};
			dbc_decode_BRIDGE_TOTAL_CHECKPOINT(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.total_points = can_msg_bridge.BRIDGE_TOTAL_CHECKPOINT_NUMBER;
#ifdef DEBUG_PRINTF
			printf("RX Bridge Checkpoints\n");
#endif
		}

		else if(can_msg.msg_id == BLUETOOTH_DATA_HDR.mid)
		{
			/* Collect All the checkpoint Data */
			BLUETOOTH_DATA_t can_msg_bridge = {0};
			dbc_decode_BLUETOOTH_DATA(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.latitude[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LAT;
			checkpoints_data.longitude[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LON;
			checkpoints_data.position++;

			if(checkpoints_data.position == checkpoints_data.total_points)
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
			MOTORIO_DIRECTION_t motor_cmd;
			motor_cmd.MOTORIO_DIRECTION_speed = STOP;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);

#ifdef DEBUG_PRINTF
			printf("RX STOP CAR CMD\n");
#endif
		}

	}
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

