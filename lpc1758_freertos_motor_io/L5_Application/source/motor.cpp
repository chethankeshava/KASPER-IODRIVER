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
#include "printf_lib.h"
#include "stdio.h"
#include "adc0.h"


can_msg_t can_msg2;

MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
SENSOR_SONIC_t s_cmd_msg = { 0 };
COMPASS_DATA_t c_cmd_msg = {0};
GPS_LOCATION_t g_cmd_msg={0};


const uint32_t            MOTORIO_DIRECTION__MIA_MS = 300;
const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 0 };


#define H_LEFT                           9.0   //9.0 old one is 8.7
#define S_LEFT                           8.7
#define H_RIGHT                          5.7   //5.7
#define S_RIGHT                          6.5
#define STRAIGHT_STEER                   7.8   //7.5

#define DC_STOP                          7.1
#define DC_SLOW                    		 6.4
#define DC_NORMAL                   	 6.2
#define DC_SUPER_FAST             		 5.5

#define DESIRED_COUNT_SLOW				 8
#define DESIRED_COUNT_NORMAL			 11

float dc_pwm 	=	6.6;
extern int m_rpmCounter;
int tilt_y =0;
int speed_temp;
/**
 * todo: avoid using globals. You can use semaphores or make this interrupt based.
 */
bool stop_flag = false;
int s_left=0;
int s_center=0;
int s_right=0;
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

//As per waveform and real testing(percent range - 6.0(right) - 7.5(center) - 9.3(left))
//As per waveform only(percent range - 5.5(forward) - 8.5(stop) - 10.5(backward))


void drive_car(void)
{
	/**
	 * todo: do not use a while loop. use periodic tasks to receive and parse messages.
	 */
	LD.setNumber(88);

	while (CAN_rx(can1, &can_msg2, 0))
	{
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg2.frame_fields.data_len;
		can_msg_hdr.mid = can_msg2.msg_id;
		//	printf("%d",can_msg2.msg_id);

#if 1
		//printf("mid--> %d\n ",can_msg2.msg_id);
		if(can_msg2.msg_id== SENSOR_SONIC_HDR.mid)
		{
			dbc_decode_SENSOR_SONIC(&s_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
			s_center=s_cmd_msg.SENSORS_SONIC_front_center;
			s_left=s_cmd_msg.SENSORS_SONIC_front_left;
			s_right=s_cmd_msg.SENSORS_SONIC_front_right;
			//			printf("l-->%d",s_left);
			//			printf("  c-->%d",s_center);
			//			printf("  r-->%d\n",s_right);
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

			//	printf(" %d ",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
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
				//dc_pwm = 6.3;
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
			{
				speed_temp=SLOW;

				//dc_pwm = DC_SLOW;
				drive_with_feedback();
				//	dc_pwm=6.4;
				dc_accelerate(dc_pwm);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
			{
				speed_temp=NORMAL;

				drive_with_feedback();
				//	dc_pwm=6.4;
				dc_accelerate(dc_pwm);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == REVERSE)
			{
				drive_with_feedback();
				dc_accelerate(dc_pwm);
			}
		}
#endif
	}
}
/**
 * todo: clean up these braces. They don't line up correctly and make your code confusing.
 */


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

#if 0
bool rpm_sensor(void)
{
	static int reading = 0;

	// Initialization :
	LPC_PINCON->PINSEL3 |=  (3 << 28); // ADC-4 is on P1.30, select this as ADC0.4

	//xQueueReceive(g_adc_result_queue, &reading, 0);      // Obtain reading from g_adc_result_queue
	reading = adc0_get_reading(4); // Read current value of ADC-4
	//		printf(" %d   %d  \n", reading, white_value);
	//if (reading > white_value && !recvd_white) 		//&& !white_patch_flag)    // Check if white patch is there and if it is there, then make white_patch_flag true and increment white_patch_count
	if (reading > white_value && !recvd_white)
	{
		//white_patch_flag = true;
		white_patch_count++;
		recvd_white = true;
	}
	//else if(reading < white_value && recvd_white)
	else if(reading < 1900 && recvd_white)
	{
		recvd_white = false;
	}
	check_time += 1;
	if(reading > max_light_value)
		max_light_value = reading;

	if(reading < min_light_value)
		min_light_value = reading;

}
#endif

void drive_with_feedback(void)
{
	//printf(" %d ",m_rpmCounter);
	// if tilt  > 60 downhill
	// if tilt < 0 uphill

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
	{

		if(m_rpmCounter>=1 && m_rpmCounter<=2)
		{
			dc_pwm=6.6;
		}
		if(m_rpmCounter>3 && dc_pwm<6.9)
		{
			dc_pwm+=0.2;            //slow
		}
		if(m_rpmCounter > 3 && dc_pwm>6.3)
		{
			dc_pwm-=0.01;			//fast
		}
		printf(" %d ",m_rpmCounter);
		m_rpmCounter=0;

	}

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
	{
		if(m_rpmCounter>=5 && m_rpmCounter<=7)
		{
			dc_pwm+=0.1;
		}
		if(m_rpmCounter>9 && dc_pwm<6.9)
		{
			dc_pwm+=0.1;            //slow
		}
		if(m_rpmCounter < 5 && dc_pwm>6.3)
		{
			dc_pwm-=0.01;			//fast
		}
		printf(" %d ",m_rpmCounter);
		m_rpmCounter=0;
	}
	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == REVERSE)
	{
		dc_pwm=7.0;
	}

}



#if 0
void drive_motor()
{

	//printf(" %d ",m_rpmCounter);
	tilt_y = AS.getY();
	//	printf("y-->%d\n",tilt_y);

	// if tilt  > 60 downhill
	// if tilt < 0 uphill

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
	{
		if(tilt_y > 0 && tilt_y <75)    //straight level
			dc_pwm = 6.45;
		else
		{
			if(white_patch_count < DESIRED_COUNT_SLOW && dc_pwm > 6.2 && tilt_y < 5) // uphill negative-
				dc_pwm-= 0.02;
			if((white_patch_count >= DESIRED_COUNT_SLOW || tilt_y>75) && dc_pwm<=6.7) //downhill
				dc_pwm = 6.7;//dc_pwm+= 0.1;
			if(dc_pwm<=6)
				dc_pwm = 7.0;
		}
		//	printf("s--> %f  %d\n",dc_pwm,tilt_y);

	}

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
	{
		if(tilt_y > 0 && tilt_y < 75)
		{
			dc_pwm = 6.45;
		}

		else
		{
			if(white_patch_count < DESIRED_COUNT_NORMAL && dc_pwm > 6.2 && tilt_y < 5) // uphill negative
				dc_pwm-= 0.02;
			if((white_patch_count >= DESIRED_COUNT_NORMAL || tilt_y > 75) && dc_pwm <= 6.7) //downhill
				dc_pwm = 6.7;//dc_pwm+= 0.1;
			if(dc_pwm <= 6)
				dc_pwm = 7.0;
		}
		//	printf("n-->%f  %d v\n",dc_pwm,tilt_y);
	}
	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == REVERSE)
	{
		//	dc_pwm=7.7;
		//	printf(" rev-->%f ",dc_pwm);
	}

}
#endif
