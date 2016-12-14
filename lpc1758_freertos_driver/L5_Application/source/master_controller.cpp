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

//#define DEBUG_SENSOR 1
#define DEBUG_GEO 1
#define DEBUG_GEO_NAV_DIR 1
#define DEBUG_BRIDGE 1
#define DEBUG_START_STOP 1
#define DEBUG_CRITICAL 1
#define CAR_STATE 1

SENSOR_SONIC_t can_msg_sensor_data = {0};
POWER_SYNC_ACK_t power_sync_ack = {0};
STATE_CAR car_state = INITIAL_STATE;

static bool obstacle_avoidance_state = false;

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
	uint16_t sensor_back = sensor_data.SENSORS_SONIC_back;


	uint8_t ls_range = (sensor_left <= 5) ? VERY_NEAR : sensor_left <= 60 ? NEAR : sensor_left <= 120 ? MEDIUM : FAR;
	uint8_t centre_range = (sensor_center <= 5) ? VERY_NEAR : sensor_center <= 60 ? NEAR : sensor_center <= 140 ? MEDIUM : FAR;
	uint8_t rs_range = (sensor_right <= 5) ? VERY_NEAR : sensor_right <= 60 ? NEAR : sensor_right <= 120 ? MEDIUM : FAR;
	uint8_t back_range = (sensor_back <= 5) ? VERY_NEAR : sensor_back <= 60 ? NEAR : sensor_back <= 120 ? MEDIUM : FAR;


	// TODO: As Calvin has rightly commented, we need to handle backing up
#if 0
	if((centre_range == NEAR) && (back_range != FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed s= STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((ls_range == NEAR) && (rs_range == NEAR) && (back_range != FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((centre_range == NEAR) && (back_range == FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = REVERSE;
		motor_cmd.MOTORIO_DIRECTION_direction = BACK;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((ls_range == NEAR) && (rs_range == NEAR) && (back_range == FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = REVERSE;
		motor_cmd.MOTORIO_DIRECTION_direction = BACK;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
#endif
#if 1
	if((centre_range == NEAR))
	{
		status = false;
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
	else if((ls_range == NEAR) && (rs_range == NEAR))
	{
		status = false;
		motor_cmd.MOTORIO_DIRECTION_speed = STOP;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}
#endif
	else if((ls_range == MEDIUM) && (centre_range != NEAR) && (rs_range == FAR))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = SLIGHT_RIGHT;
	}
	else if((ls_range == FAR) && (centre_range != NEAR) && (rs_range == MEDIUM))
	{
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
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
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}

	else
	{
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
	}



#ifdef DEBUG_SENSOR
	printf("\nSensor value: L: %u | C: %u | R: %u | B: %u", sensor_left, sensor_center, sensor_right, sensor_back);
	printf("\nMotor value: Speed = %d | Direction = %d | Turn = %d",motor_cmd.MOTORIO_DIRECTION_speed, motor_cmd.MOTORIO_DIRECTION_direction, motor_cmd.MOTORIO_DIRECTION_turn);
#endif

	dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
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
	bool obstacle_status = false;
	if(obstacle_avoidance_state)
	{
		/* Sensor data is populated in 100Hz ,get the sensor data and avoid obstacle*/
		obstacle_status = avoid_obstacle_and_drive();
	}

	if(car_state == NAVIGATING && obstacle_status)
	{
		// TODO: Navigation algorithm in progress
		// Check for final destination
		/* Check the current location distance from next checkpoint*/
#ifdef CAR_STATE
		printf("\nState of car :: NAVIGATING");
#endif
		if((can_msg_compass.COMPASS_DATA_distance <= DIST_TO_CHECKPOINT) && (checkpoints_data.geo_update_pos < checkpoints_data.total_points))
		{
#ifdef CAR_STATE
			printf("\nState of car :: REACHED NEXT CHECKPOINT\n");
#endif
			car_state = SEND_CHECKPOINTS;
		}
		else if((can_msg_compass.COMPASS_DATA_distance <= DIST_TO_STOP) && (checkpoints_data.geo_update_pos == checkpoints_data.total_points))
		{
			printf("\nState of car :: STOP CAR\n");
			MOTORIO_DIRECTION_t motor_cmd = {0};
			car_state = STOP_CAR;
			obstacle_avoidance_state = false;
			motor_cmd.MOTORIO_DIRECTION_speed = STOP;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
		}

		/* Check the difference of current bearing from heading of checkpoint */
		float car_direction;
		car_direction = can_msg_compass.COMPASS_DATA_bearing - can_msg_compass.COMPASS_DATA_heading;
		navigate_car_based_on_bearing(car_direction);
	}
	else if(car_state == SEND_CHECKPOINTS)
	{
#ifdef CAR_STATE
		printf("\nState of car :: SEND_CHECKPOINTS");
#endif
		/* Send the first Location Update to Geo Controller */
		NEXT_CHECKPOINT_DATA_t geo_data = {0};
		geo_data.NEXT_CHECKPOINT_DATA_latitude = checkpoints_data.latitude[checkpoints_data.geo_update_pos];
		geo_data.NEXT_CHECKPOINT_DATA_longitude = checkpoints_data.longitude[checkpoints_data.geo_update_pos];
#ifdef DEBUG_GEO
		printf("\nGEO NEXT CHECKPOINT: %f %f\n",geo_data.NEXT_CHECKPOINT_DATA_latitude,geo_data.NEXT_CHECKPOINT_DATA_longitude);
#endif
		dbc_encode_and_send_NEXT_CHECKPOINT_DATA(&geo_data);
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



void navigate_car_based_on_bearing(float car_direction)
{
	MOTORIO_DIRECTION_t motor_cmd = {0};

	if(car_direction < 0)
	{
		car_direction += 360;
	}

	if((car_direction > 45) & (car_direction < 90))
	{
#ifdef DEBUG_GEO_NAV_DIR
		printf("\n Navigation: Slight Right");
#endif
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = SLIGHT_LEFT;
	}
	else if((car_direction > 90) & (car_direction < 180))
	{
#ifdef DEBUG_GEO_NAV_DIR
		printf("\n Navigation: Hard Right");
#endif
		motor_cmd.MOTORIO_DIRECTION_speed = NORMAL;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_LEFT;
	}
	else if((car_direction > 180) & (car_direction < 270))
	{
#ifdef DEBUG_GEO_NAV_DIR
		printf("\n Navigation: Hard Left");
#endif
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = HARD_RIGHT;
	}
	else if((car_direction > 270) & (car_direction < 360))
	{
#ifdef DEBUG_GEO_NAV_DIR
		printf("\n Navigation: Slight Left");
#endif
		motor_cmd.MOTORIO_DIRECTION_speed = SLOW;
		motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
		motor_cmd.MOTORIO_DIRECTION_turn = SLIGHT_RIGHT;
	}

	dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);
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
	status_t miastatus = false;

	while(CAN_rx(can1, &can_msg, 0))
	{
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;

		if(can_msg.msg_id == SENSOR_SONIC_HDR.mid)
		{
			LD.setNumber(0);
			dbc_decode_SENSOR_SONIC(&sensor_data, can_msg.data.bytes, &can_msg_hdr);
		}

		else if(can_msg.msg_id == BRIDGE_TOTAL_CHECKPOINT_HDR.mid)
		{
			BRIDGE_TOTAL_CHECKPOINT_t can_msg_bridge = {0};
			dbc_decode_BRIDGE_TOTAL_CHECKPOINT(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.total_points = can_msg_bridge.BRIDGE_TOTAL_CHECKPOINT_NUMBER;
#ifdef DEBUG_BRIDGE
			printf("Bridge Checkpoints: Total points = %d\n",checkpoints_data.total_points);
#endif
			checkpoints_data.position = 0;
			checkpoints_data.geo_update_pos = 0;

		}

		else if(can_msg.msg_id == BLUETOOTH_DATA_HDR.mid)
		{
			/* Collect All the checkpoint Data */
			BLUETOOTH_DATA_t can_msg_bridge = {0};
			dbc_decode_BLUETOOTH_DATA(&can_msg_bridge,can_msg.data.bytes, &can_msg_hdr);
			checkpoints_data.latitude[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LAT;
			checkpoints_data.longitude[checkpoints_data.position] = can_msg_bridge.BLUETOOTH_DATA_LON;

#ifdef DEBUG_BRIDGE
			printf("\nBridge Data: Lat: %f | Lon: %f\n",can_msg_bridge.BLUETOOTH_DATA_LAT, can_msg_bridge.BLUETOOTH_DATA_LON);
#endif
			checkpoints_data.position++;
			if(checkpoints_data.position == checkpoints_data.total_points)
			{
				car_state = SEND_CHECKPOINTS;
#ifdef DEBUG_BRIDGE
				printf("All Checkpoint Data Received\n");
#endif
			}
		}

		else if(can_msg.msg_id == COMPASS_DATA_HDR.mid)
		{
			dbc_decode_COMPASS_DATA(&can_msg_compass,can_msg.data.bytes, &can_msg_hdr);
			if(car_state == SEND_CHECKPOINTS)
			{
				++checkpoints_data.geo_update_pos;
				car_state = NAVIGATING;
			}
#ifdef DEBUG_GEO
			printf("RX COMPASS DATA :: Distance: %f | Heading: %f | Bearing: %f\n",can_msg_compass.COMPASS_DATA_distance, can_msg_compass.COMPASS_DATA_heading, can_msg_compass.COMPASS_DATA_bearing);
#endif
		}

		/* Command from App to start*/
		else if(can_msg.msg_id ==  START_CMD_APP_HDR.mid)
		{
#ifdef DEBUG_START_STOP
			printf("\nRX START CMD\n");
#endif
			obstacle_avoidance_state = true;

		}


		else if(can_msg.msg_id == STOP_CMD_APP_HDR.mid)
		{
#ifdef DEBUG_START_STOP
			printf("\nRX STOP CAR CMD\n");
#endif
			car_state = STOP_CAR;
			obstacle_avoidance_state = false;
			MOTORIO_DIRECTION_t motor_cmd;
			motor_cmd.MOTORIO_DIRECTION_speed = STOP;
			motor_cmd.MOTORIO_DIRECTION_direction = FORWARD;
			motor_cmd.MOTORIO_DIRECTION_turn = STRAIGHT;
			dbc_encode_and_send_MOTORIO_DIRECTION(&motor_cmd);

		}

	}
	miastatus = dbc_handle_mia_SENSOR_SONIC(&can_msg_sensor_data, SENSOR_MIA_TIMEOUT);
	if(miastatus == true)
	{
#ifdef DEBUG_SENSOR
		printf("Sensor Data MIA\n");
#endif
		LD.setNumber(99);
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
#ifdef DEBUG_CRITICAL
		printf("\nBus is Off");
#endif
		CAN_reset_bus(can1);
	}
}

