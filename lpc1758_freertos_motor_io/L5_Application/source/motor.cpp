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
SENSOR_SONIC_t sensor_cmd_msg = { 0 };

can_msg_t can_msg2;
//const uint32_t            RESET__MIA_MS = 3;
//
//const RESET_t      RESET__MIA_MSG = { 4 };

//RESET_t reset_cmd_msg = { 0 };

//
//MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
//const uint32_t            MOTORIO_DIRECTION__MIA_MS = 3;
//
//const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 4 };


#define HARD_LEFT                           9.0
#define S_LEFT                              8.0
#define HARD_RIGHT                          5.7
#define S_RIGHT                             6.5
#define STRAIGHT                            7.5
#define SPEED_VAR_FACTOR                    0.05
#define DC_STOP                             7.0
#define DC_SUPER_SLOW                       6.5
#define DC_SUPER_FAST                     	5.5


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

//MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
//can_msg_t can_msg;
//const uint32_t            MOTORIO_DIRECTION__MIA_MS = 3;
//
//const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 4 };



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
			//		case 410:
			//			LE.off(1);
			//			LD.setNumber(99);
			//			dbc_decode_MOTORIO_DIRECTION(&mDirection_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			//			//			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed==1)
			//			//			{
			//			//				dc_accelerate();
			//			//
			//			//			}
			//			if(sensor_cmd_msg.SENSORS_SONIC_front_left>=20)
			//			{
			//				dc_accelerate();
			//
			//			}
			//			if(sensor_cmd_msg.SENSORS_SONIC_front_left<20)
			//			{
			//				dc_stop();
			//
			//			}
			//			//			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed==0)
			//			//			{
			//			//				dc_stop();
			//			//			}
			//			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==2)
			//			{
			//				if(c>=0 && c<=100 )
			//				{
			//					//		printf("c1-->%d\n",c);
			//					servo_left();
			//					delay_ms(10);
			//					//	servo_right();
			//					//	servo_straight();
			//				}
			//
			//			}
			//			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==3)
			//			{
			//				if(c>=101 && c<=200 )
			//				{
			//					//	printf("c2-->%d\n",c);
			//					servo_right();
			//					delay_ms(10);
			//
			//					//	servo_right();
			//					//	servo_straight();
			//
			//				}
			//				if(c>220)
			//				{
			//					c=0;
			//				}
			//			}
			//
			//			break;
		case 400:
			dbc_decode_SENSOR_SONIC(&sensor_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			printf("l--> %d\n",sensor_cmd_msg.SENSORS_SONIC_front_left);
			printf("r--> %d\n ",sensor_cmd_msg.SENSORS_SONIC_front_right);
			printf("c--> %d\n ",sensor_cmd_msg.SENSORS_SONIC_front_center);

			if(sensor_cmd_msg.SENSORS_SONIC_front_left>=20 && sensor_cmd_msg.SENSORS_SONIC_front_right>=20)
			{
				servo_straight();

			}
			if(sensor_cmd_msg.SENSORS_SONIC_front_left<20 && sensor_cmd_msg.SENSORS_SONIC_front_right>=20)
			{

				servo_right();
			}
			if(sensor_cmd_msg.SENSORS_SONIC_front_right<20 && sensor_cmd_msg.SENSORS_SONIC_front_left>=20)
			{
				servo_left();
			}
			if(sensor_cmd_msg.SENSORS_SONIC_front_center>=20)
			{
				dc_accelerate();
			}
			if(sensor_cmd_msg.SENSORS_SONIC_front_center<20)
			{
				dc_stop();
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
	//printf("Inite\n");


	MotorControl.setServo(STRAIGHT);
	//  delay_ms(100);

}

void dc_accelerate(void)
{
//	printf(" acc ");

	MotorControl.setDC(DC_SUPER_SLOW);

}
void dc_stop(void)
{
//	printf(" dcstop ");
	MotorControl.setDC(DC_STOP);

	//delay_ms(15);
}

void dc_fast(void)
{
	MotorControl.setDC(6.1);
	//MotorControl.setDC(DC_STOP);

}


