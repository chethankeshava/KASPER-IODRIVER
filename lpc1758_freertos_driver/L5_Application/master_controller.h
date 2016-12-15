#ifndef MASTER_CONTROLLER_H_
#define MASTER_CONTROLLER_H_


#include <../_can_dbc/generated_can.h>
#include <stdint.h>

#define SENSOR_MIA_TIMEOUT 10

#define MASTER
//#define DEBUG_PRINTF

typedef bool status_t;

#define DIST_TO_CHECKPOINT 10 // in meters
#define DIST_TO_STOP 2

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
	FAST,
	REVERSE
};

/*
 * MOTOR_TURN
 * Type: enum
 * Fields: Straight=0, Slight Right=1, Hard Right=2, Slight Left=3, Hard Left=4
 *
 */
enum MOTOR_TURN{
	SLIGHT_LEFT = -2,
	HARD_LEFT,
	STRAIGHT,
	SLIGHT_RIGHT,
	HARD_RIGHT,
};

/*
 * MOTOR_DIRECTION
 * Type: enum
 * Fields: Straight=0, Slight Right=1, Hard Right=2, Slight Left=3, Hard Left=4
 *
 */
enum MOTOR_DIRECTION{
	FORWARD = 0,
	BACK
};

// TODO: Associate sensor readings with appropriate range enums
/*
 * OBSTACLE_RANGE
 * Type: enum
 * Fields: Near=0, Medium=1, Far=2
 *
 */
enum OBSTACLE_RANGE{
	VERY_NEAR = 0,
	NEAR,
	MEDIUM,
	FAR,
};

enum STATE_CAR{
	START_CAR,
	SEND_CHECKPOINTS,
	NAVIGATING,
	STOP_CAR,
	INITIAL_STATE,
};


typedef struct{
    float latitude[20];
    float longitude[20];
	uint8_t total_points;
	uint8_t geo_update_pos;
	uint8_t position;
	float cur_loc_lat;
	float cur_loc_long;
}checkpoints_data_t;


/* FUNCTION DECLARATIONS */

/* Check if can bus is in bus-off state */
void is_bus_off(void);

/* Initialize can for master controller */
status_t init_can_master(void);

/* Avoid obstacles based on sensor readings and drive the motor */
status_t avoid_obstacle_and_drive(void);

/* Receive sensor data for obstacle avoidance */
SENSOR_SONIC_t receive_sensor_data(void);

/* Receive Start/Stop command from Application */
status_t cmd_from_app(void);

/* Send power sync acknowledgment to all controllers to start the periodic transactions */
status_t send_power_sync_ack(void);

void test_motor();

/* TODO: Determine range based on sensor readings */
//OBSTACLE_RANGE determine_obstacle_range(&can_msg_sensor_data);
void drive_car(void);

void receive_data_from_can(void);

void navigate_car_based_on_bearing(float car_direction);

#endif
