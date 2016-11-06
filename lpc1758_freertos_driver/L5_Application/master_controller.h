#ifndef MASTER_CONTROLLER_H_
#define MASTER_CONTROLLER_H_


#include <../_can_dbc/generated_can.h>
#include <stdint.h>

#define MASTER
#define DEBUG_PRINTF

typedef bool status_t;

/*
 * MOTOR_SPEED
 * Type: enum
 * Fields: Stop=0, Slow=1, Normal=2, Fast=3
 *
 */
enum MOTOR_SPEED{
	STOP = 0,
	SLOW,
	NORMAL,
	FAST
};

/*
 * MOTOR_TURN
 * Type: enum
 * Fields: Straight=0, Slight Right=1, Hard Right=2, Slight Left=3, Hard Left=4
 *
 */
enum MOTOR_TURN{
	STRAIGHT = 0,
	SLIGHT_RIGHT,
	HARD_RIGHT,
	SLIGHT_LEFT,
	HARD_LEFT,
};

/*
 * MOTOR_DIRECTION
 * Type: enum
 * Fields: Straight=0, Slight Right=1, Hard Right=2, Slight Left=3, Hard Left=4
 *
 */
enum MOTOR_DIRECTION{
	FORWARD = 0,
	REVERSE
};

// TODO: Associate sensor readings with appropriate range enums
/*
 * OBSTACLE_RANGE
 * Type: enum
 * Fields: Near=0, Medium=1, Far=2
 *
 */
enum OBSTACLE_RANGE{
	NEAR = 0,
	MEDIUM,
	FAR
};


/* FUNCTION DECLARATIONS */

/* Check if can bus is in bus-off state */
void is_bus_off(void);

/* Initialize can for master controller */
status_t init_can_master(void);

/* Check for periodic heartbeat from each controller */
// TODO: Check if reset function needs to be included.
status_t read_controller_heartbeat(void);

/* Avoid obstacles based on sensor readings and drive the motor */
status_t avoid_obstacle_and_drive(void);

/* Receive sensor data for obstacle avoidance */
SENSOR_SONIC_t receive_sensor_data(void);

/* Receive Start/Stop command from Application */
status_t cmd_from_app(void);

/* Send power sync acknowledgement to all controllers to start the periodic transactions */
status_t send_power_sync_ack(void);

/* TODO: Determine range based on sensor readings */
//OBSTACLE_RANGE determine_obstacle_range(&can_msg_sensor_data);

#endif
