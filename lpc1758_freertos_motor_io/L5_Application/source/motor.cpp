/*
 * motor.cpp
 *
 *  Created on: Oct 31, 2016
 *      Author: Chethan
 */
#include "motor.hpp"
#include "../_can_dbc/generated_can.h"
#include "can.h"
#include "string.h"
#include "printf_lib.h"
#include "stdio.h"
#include "adc0.h"


enum MOTOR_TURN
{
	SLIGHT_LEFT = -2,
	HARD_LEFT,
	STRAIGHT,
	SLIGHT_RIGHT,
	HARD_RIGHT,
};

enum MOTOR_SPEED
{
	STOP = 0,
	SLOW,
	NORMAL,
	FAST,
	REVERSE
};

enum MOTOR_DIRECTION
{
	FORWARD = 0,
	BACK
};

can_msg_t can_msg2;

MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
SENSOR_SONIC_t s_cmd_msg = { 0 };
COMPASS_DATA_t c_cmd_msg = {0};
GPS_LOCATION_t g_cmd_msg={0};


const uint32_t            MOTORIO_DIRECTION__MIA_MS = 300;
const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 0 };

bool reverse_flag = 0;


#define H_LEFT                           9.2    // old one is 8.7
#define S_LEFT                           8.7
#define H_RIGHT                          6.3   //5.7 then 5.9
#define S_RIGHT                          6.5
#define STRAIGHT_STEER                   7.8   //7.5

#define DC_STOP                          7.1
#define DC_SLOW                    		 6.4
#define DC_NORMAL                   	 6.2

float dc_pwm 	=	6.4;
extern int m_rpmCounter;
extern int prev_rpmCounter;

int tilt_y =0;
int speed_temp;

bool stop_flag = false;
int s_left=0;
int s_center=0;
int s_right=0;
int s_back=0;
float distance = 0;
float bearing=0;
float heading=0;
float latitide=0;
float longitude=0;






MotorController::MotorController(): driveMotor(PWM::pwm2,54), steerMotor(PWM::pwm1,54)
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

//As per testing for the Steer Motor(percent range - 6.0(right) - 7.5(center) - 9.3(left))
//As per testing for the DC motor (percent range - 5.5(forward) - 8.5(stop) - 10.5(backward))


void drive_car(int count)
{
	LD.setNumber(88);

	MotorControl.setServo(S_LEFT);
	dc_accelerate(6.4);


	while (CAN_rx(can1, &can_msg2, 0))
	{
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg2.frame_fields.data_len;
		can_msg_hdr.mid = can_msg2.msg_id;
		//	printf("%d",can_msg2.msg_id);

#if 1
		//	printf("mid--> %d\n ",can_msg2.msg_id);
		if(can_msg2.msg_id == SENSOR_SONIC_HDR.mid)
		{
			dbc_decode_SENSOR_SONIC(&s_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			s_center=s_cmd_msg.SENSORS_SONIC_front_center;
			s_left=s_cmd_msg.SENSORS_SONIC_front_left;
			s_right=s_cmd_msg.SENSORS_SONIC_front_right;
			s_back=s_cmd_msg.SENSORS_SONIC_back;
			//	printf("l-->%d",s_left);
			//	printf("  c-->%d",s_center);
			//	printf("  r-->%d\n",s_right);
		}
		if(can_msg2.msg_id == COMPASS_DATA_HDR.mid)
		{
			dbc_decode_COMPASS_DATA(&c_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			distance = c_cmd_msg.COMPASS_DATA_distance;
			bearing=c_cmd_msg.COMPASS_DATA_bearing;
			heading=c_cmd_msg.COMPASS_DATA_heading;
		}
		if(can_msg2.msg_id == GPS_LOCATION_HDR.mid)
		{
			dbc_decode_GPS_LOCATION(&g_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			latitide = g_cmd_msg.GPS_LOCATION_latitude;
			longitude=g_cmd_msg.GPS_LOCATION_longitude;
		}

		if(can_msg2.msg_id == MOTORIO_DIRECTION_HDR.mid)
		{
			dbc_decode_MOTORIO_DIRECTION(&mDirection_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);

			//printf(" %d ",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
			//printf(" %d ",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);

			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_LEFT)
			{
				MotorControl.setServo(S_LEFT);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_LEFT)
			{
				MotorControl.setServo(H_LEFT);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==STRAIGHT)
			{
				MotorControl.setServo(STRAIGHT_STEER);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_RIGHT)
			{
				MotorControl.setServo(S_RIGHT);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_RIGHT)
			{
				MotorControl.setServo(H_RIGHT);
			}

			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == STOP)
			{
				speed_temp=STOP;
				dc_stop();
				dc_pwm = 6.4;
			}
			if(count%10 == 0)
			{
				if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
				{
					speed_temp=SLOW;
					dc_pwm = DC_SLOW;
					drive_with_feedback();
					if(reverse_flag)
					{
						dc_pwm=6.4;
						reverse_flag = 0;
					}
				//	dc_pwm=6.4;
					dc_accelerate(dc_pwm);
				}
				if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
				{
					speed_temp=NORMAL;
					drive_with_feedback();
					if(reverse_flag)
					{
						dc_pwm=6.4;
						reverse_flag = 0;
					}
					//dc_pwm=6.4;
					dc_accelerate(dc_pwm);
				}
			}
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == REVERSE)
			{
				dc_pwm=7.8;
				dc_accelerate(dc_pwm);
				reverse_flag = 1;
			}
		}
#endif
	}
}



void handle_motor_mia(void)
{
	if(dbc_handle_mia_MOTORIO_DIRECTION(&mDirection_cmd_msg, 1))
	{
		LE.on(1);
		mDirection_cmd_msg.MOTORIO_DIRECTION_direction=MOTORIO_DIRECTION__MIA_MSG.MOTORIO_DIRECTION_direction;
		LD.setNumber(41);
	}
}

void dc_accelerate(float pwmValue)
{
	MotorControl.setDC(pwmValue);
	stop_flag = true;
}

void dc_stop(void)
{
	if(stop_flag)
	{
		MotorControl.setDC(DC_STOP);
		stop_flag = false;
	}
}

void dcmotor_init(void)
{
	uint8_t boot=0;
	while(boot < 15)//made it 15 from 20
	{
		MotorControl.setDC(DC_STOP);
		delay_ms(50);
		boot++;
	}
	delay_ms(20);
}


void drive_with_feedback(void)
{
	//printf(" %d ",m_rpmCounter);


	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
	{
		if(prev_rpmCounter >=8 && dc_pwm < 7.0 )
		{
			dc_pwm+=0.1;
		}
		else if(prev_rpmCounter >=6 && dc_pwm < 7.0)
		{
			dc_pwm+=0.1;            //slow
		}

		if(prev_rpmCounter >= 0 && prev_rpmCounter < 3 && dc_pwm>= 5.7)
		{
			dc_pwm-=0.02;			//fast
		}
		else if(prev_rpmCounter >= 3 && prev_rpmCounter < 5 && dc_pwm>= 5.8)
		{
			dc_pwm-=0.01;			//fast
		}
	}
	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
	{
		if(prev_rpmCounter >=8 && dc_pwm < 7.0 )
		{
			dc_pwm+=0.1;
		}
		else if(prev_rpmCounter >=6 && dc_pwm < 7.0)
		{
			dc_pwm+=0.1;            //slow
		}

		if(prev_rpmCounter >= 0 && prev_rpmCounter < 3 && dc_pwm>= 5.7)
		{
			dc_pwm-=0.02;			//fast
		}
		else if(prev_rpmCounter >= 3 && prev_rpmCounter < 5 && dc_pwm>= 5.8)
		{
			dc_pwm-=0.01;			//fast
		}
	}

}

