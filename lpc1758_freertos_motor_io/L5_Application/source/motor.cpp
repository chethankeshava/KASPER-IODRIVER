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

const uint32_t            MOTORIO_DIRECTION__MIA_MS = 300;
const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 0 };


SENSOR_SONIC_t s_cmd_msg = { 0 };
int tilt_y =0;

#define H_LEFT                           8.7   //9.0
#define S_LEFT                           8.0
#define H_RIGHT                          6.0   //5.7
#define S_RIGHT                          6.5
#define STRAIGHT_STEER                   7.5

#define DC_STOP                          7.1
#define DC_SLOW                    		 6.4
#define DC_NORMAL                   	 6.2
#define DC_SUPER_FAST             		 5.5

#define DESIRED_COUNT_SLOW				 8
#define DESIRED_COUNT_NORMAL			 11


static bool recvd_white = false;
static bool white_patch_flag = false;
static int check_time = 0;

int light_threshold = 1900;

//char speed_flag;



#define RPM_DEAD_SECOND_COUNT             4
#define SPEED_VAR_FACTOR                 0.05

int white_patch_count;
//extern QueueHandle_t g_adc_result_queue;
float stop_dc   =   7.0;
float slow_dc   =   6.35;
float normal_dc =   6.28;
float fast_dc   =   6.20;
float dc_pwm 	=	6.35;

//int white_value,max_light_value,min_light_value,temp_count;



/**
 * todo: avoid using globals. You can use semaphores or make this interrupt based.
 */
bool stop_flag = false;




int s_left=0;
int s_center=0;
int s_right=0;

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

	while (CAN_rx(can1, &can_msg2, 0))
	{
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg2.frame_fields.data_len;
		can_msg_hdr.mid = can_msg2.msg_id;
		//	printf("%d",can_msg2.msg_id);

#if 1
		//		printf("mid--> %d\n ",can_msg2.msg_id);

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
		//
		if(can_msg2.msg_id == MOTORIO_DIRECTION_HDR.mid)
		{
			LD.setNumber(00);
			dbc_decode_MOTORIO_DIRECTION(&mDirection_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);

			//		printf("t--> %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
			//printf("s--> %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);

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
				dc_stop();
				dc_pwm = 6.3;
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
			{
				//dc_pwm = DC_SLOW;
				drive_motor();
				dc_accelerate(dc_pwm);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
			{
				drive_motor();
				dc_accelerate(dc_pwm);
			}
			//			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == FAST)
			//			{
			//				//dc_stop();

			//				dc_accelerate(DC_SUPER_FAST);
			//			actual_speed = fast_dc;
			//							desired_count = dc_fast_count;
			//			}


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


bool rpm_sensor(void)
{
	static int reading = 0;

	// Initialization :
	LPC_PINCON->PINSEL3 |=  (3 << 28); // ADC-4 is on P1.30, select this as ADC0.4

	//xQueueReceive(g_adc_result_queue, &reading, 0);      // Obtain reading from g_adc_result_queue
	reading = adc0_get_reading(4); // Read current value of ADC-4
	//	printf(" %d   %d  \n", reading, white_value);
	//if (reading > white_value && !recvd_white) 		//&& !white_patch_flag)    // Check if white patch is there and if it is there, then make white_patch_flag true and increment white_patch_count
	if (reading > 1900 && !recvd_white)
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
#if 0
	//int   tilt_x = AS.getX();
	//int   tilt_y = AS.getY();
	//int   tilt_z = AS.getZ();
	//	printf("x-->%d  y-->%d z-->%d\n", tilt_x,tilt_y,tilt_z);


	if((rpm < 15 && slow_dc > 6.25 && normal_dc > 6.2))
	{
		slow_dc-= 0.01;
		normal_dc-= 0.01;
	}
	else if(rpm < 15)
	{
		//stop_dc   =   7.0;
		slow_dc   =   6.35;
		normal_dc =   6.28;
	}
	if((rpm > 90 && rpm < 200 && slow_dc<6.4 && normal_dc<6.4))
	{
		slow_dc+= 0.03;
		normal_dc+= 0.03;
	}

	if(rpm > 250 && slow_dc > 6.2 && normal_dc > 6.1 )
	{
		slow_dc-= 0.03;
		normal_dc-= 0.03;
	}

	if(slow_dc>6.2 || normal_dc>6.2)
	{
		//	printf("%f  %f", slow_dc,normal_dc);
	}
	if(reading>white_value)
	{
		return true;
	}
	else
	{
		return false;
	}
	// delay_ms(100);
#endif

}

void drive_motor()
{
	//	if((white_patch_count < 5 && slow_dc > 6.2 && normal_dc > 6.1))
	//	{
	//		slow_dc-= 0.01;
	//		normal_dc-= 0.01;
	//	}
	//	else if(white_patch_count < 5)
	//	{
	//		//stop_dc   =   7.0;
	//		slow_dc   =   6.35;
	//		normal_dc =   6.28;
	//	}
	//		if((white_patch_count > 11 && white_patch_count < 20 && slow_dc<6.4 && normal_dc<6.4) && mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
	//		{
	//			slow_dc+= 0.03;
	//			normal_dc+= 0.03;
	//		}
	tilt_y = AS.getY();
	//	printf("y-->%d\n",tilt_y);

	// if tilt  > 60 downhill
	// if tilt < 0 uphill

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
	{

		//		if(tilt_y >= 0 && tilt_y < 60)		// Straight no slope go on normal speed
		//		{
		//			dc_pwm = DC_SLOW;
		//		}
		//		else if (tilt_y >= 60 && dc_pwm < 6.6)				// downhill, so make it slow
		//		{
		//			dc_pwm += 0.1;
		//		}
		//		else if (tilt_y < 0 && dc_pwm >= 6.0)				// uphill, make it faster
		//		{
		//			dc_pwm -= 0.01;
		//		if(tilt_y >120)
		//			dc_pwm=6.8;
		//	else
		//	{
		if(tilt_y > 0 && tilt_y <75)    //straight level
			dc_pwm = 6.45;
		else
		{
			if(white_patch_count < DESIRED_COUNT_SLOW && dc_pwm > 6.2 && tilt_y < 5) // uphill negative-
				dc_pwm-= 0.02;
			//				if(white_patch_count < DESIRED_COUNT_SLOW && dc_pwm > 6.4 && tilt_y > 5)
			//					dc_pwm-= 0.01;
			if((white_patch_count >= DESIRED_COUNT_SLOW || tilt_y>75) && dc_pwm<=6.7) //downhill
				dc_pwm = 6.7;//dc_pwm+= 0.1;
			if(dc_pwm<=6)
				dc_pwm = 7.0;
		}
		//	}
		//		}
		printf("s--> %f  %d\n",dc_pwm,tilt_y);

	}

	if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
	{

		//		if(tilt_y >= 0 && tilt_y < 60)		// Straight no slope go on normal speed
		//		{
		//			dc_pwm = DC_NORMAL;
		//		}
		//		else if (tilt_y >= 60 && dc_pwm < 6.6)				// downhill, so make it slow
		//		{
		//			dc_pwm += 0.1;
		//		}
		//		else if (tilt_y < 0 && dc_pwm >= 6.0)				// uphill, make it faster
		//		{
		//			dc_pwm -= 0.01;
		//		}
		//
		//		if(tilt_y >120)
		//			dc_pwm=6.8;
		//		else
		//		{
		if(tilt_y > 0 && tilt_y< 75)
		{
			dc_pwm = 6.45;
			//if(white_patch_count >= DESIRED_COUNT_NORMAL)
			//dc_pwm = 6.6;
		}

		else
		{
			if(white_patch_count < DESIRED_COUNT_NORMAL && dc_pwm > 6.2 && tilt_y < 5) // uphill negative
				dc_pwm-= 0.02;
			//	if(white_patch_count < DESIRED_COUNT_NORMAL && dc_pwm > 6.4 && tilt_y > 5)
			//	dc_pwm-= 0.01;
			//				if(white_patch_count < DESIRED_COUNT_NORMAL && dc_pwm < 6.4 && tilt_y > 5)
			//					dc_pwm+= 0.1;
			if((white_patch_count >= DESIRED_COUNT_NORMAL || tilt_y>75) && dc_pwm<=6.7) //downhill
				dc_pwm = 6.7;//dc_pwm+= 0.1;
			//else{
			//if(white_patch_count >= DESIRED_COUNT_NORMAL && dc_pwm<=6.7) //downhill
			//	dc_pwm+= 0.1;
			//}
			if(dc_pwm<=6)
				dc_pwm = 7.0;
		}
		//}
		//	if(white_patch_count >= 20)
		//	{
		//		slow_dc   =   6.35;
		//		normal_dc =   6.28;
		//	}
		//	if(white_patch_count >= 20)
		//	{
		//			dc_pwm = 6.25;
		//	}
		////	&& mDirection_cmd_msg.MOTORIO_DIRECTION_speed == STOP
		//	if(white_patch_count < 5)
		//	{
		//			//stop_dc   =   7.0;
		//			dc_pwm   =   6.3;
		//			//normal_dc =   6.28;
		//
		printf("n-->%f  %d \n",dc_pwm,tilt_y);

	}

}
