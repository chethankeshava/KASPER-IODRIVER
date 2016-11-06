/*
 * motor.cpp
 *
 *  Created on: Oct 31, 2016
 *      Author: prash
 */
#include "motor.hpp"
#include "../_can_dbc/generated_can.h"
#include "can.h"
#include "string.h"

uint8_t c=0;
//SENSOR_SONIC_t sensor_cmd_msg = { 0 };

can_msg_t can_msg2;
//const uint32_t            RESET__MIA_MS = 3;
//
//const RESET_t      RESET__MIA_MSG = { 4 };

//RESET_t reset_cmd_msg = { 0 };


MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
const uint32_t            MOTORIO_DIRECTION__MIA_MS = 3;

const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 4 };


#define H_LEFT                           9.0
#define S_LEFT                           8.0
#define H_RIGHT                          5.7
#define S_RIGHT                          6.5
#define STRAIGHT_STEER                            7.5

#define DC_STOP                             7.0
#define DC_SUPER_SLOW                       6.5
#define DC_SUPER_FAST                     	5.5

bool stop_flag = 0;


MotorController::MotorController(): driveMotor(PWM::pwm2,50), steerMotor(PWM::pwm1,50)
{

}

void MotorController::setDC(float v)
{
	driveMotor.set(v);
}
void MotorController::setServo(float v)
{
	steerMotor.set(v);
}

//As per waveform and real testing(percent range - 6.0(right) - 7.5(center) - 9.3(left))
//As per waveform only(percent range - 5.5(forward) - 8.5(stop) - 10.5(backward))


void drive_car(void)
{
	//	MotorControl.setDC(DC_SUPER_SLOW);

	while (CAN_rx(can1, &can_msg2, 0))

	{
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg2.frame_fields.data_len;
		can_msg_hdr.mid = can_msg2.msg_id;
		//	printf("%d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
		//printf("%d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);
		//	printf("%d",can_msg2.msg_id);
		c++;
		switch(can_msg2.msg_id)
		{

		case 110:
			LE.off(1);
			//	dbc_decode_RESET(&reset_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			break;
		case 410:
			LE.off(1);
			LD.setNumber(99);
			dbc_decode_MOTORIO_DIRECTION(&mDirection_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			//	printf("s--> %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);
			//	printf("d--> %d\n ",mDirection_cmd_msg.MOTORIO_DIRECTION_direction);
			printf("t--> %d\n ",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);

			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed==NORMAL)
			{
				dc_accelerate();
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed==STOP)
			{
				dc_stop();
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_LEFT)
			{
				MotorControl.setServo(S_LEFT);
				// 	delay_ms(10);
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_LEFT)
			{
				MotorControl.setServo(H_LEFT);
				//	delay_ms(10);
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==STRAIGHT)
			{
				MotorControl.setServo(STRAIGHT_STEER);
				//	delay_ms(10);
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_RIGHT)
			{
				MotorControl.setServo(S_RIGHT);
				//	delay_ms(10);
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_RIGHT)
			{
				MotorControl.setServo(H_RIGHT);
				//	delay_ms(10);
			}

			break;

		default:

			//printf("MID not defined");

			break;

		}

	}
}


void servo_left(void)
{
	MotorControl.setServo(HARD_LEFT);

}


void servo_right(void)
{
	MotorControl.setServo(HARD_RIGHT);
}
void servo_straight(void)
{
	MotorControl.setServo(STRAIGHT);
	//  delay_ms(100);
}

void dc_accelerate(void)
{
	//	printf(" acc ");

	MotorControl.setDC(DC_SUPER_SLOW);
	stop_flag = 1;

}
void dc_stop(void)
{
	//	printf(" dcstop ");
	if(stop_flag)
	{
		MotorControl.setDC(DC_STOP);
		stop_flag = 0;
	}
	//delay_ms(15);
}

void dc_fast(void)
{
	MotorControl.setDC(6.1);
	//MotorControl.setDC(DC_STOP);

}


