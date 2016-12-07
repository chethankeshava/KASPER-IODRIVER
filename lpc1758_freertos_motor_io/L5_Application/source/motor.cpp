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

//const uint32_t            RESET__MIA_MS = 3;
//
//const RESET_t      RESET__MIA_MSG = { 4 };

//RESET_t reset_cmd_msg = { 0 };


MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
const uint32_t            MOTORIO_DIRECTION__MIA_MS = 300;

const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 0 };




#define H_LEFT                           9.0
#define S_LEFT                           8.0
#define H_RIGHT                          5.7
#define S_RIGHT                          6.5
#define STRAIGHT_STEER                   7.5

#define DC_STOP                          7.0
#define DC_SLOW                    		 6.4
#define DC_NORMAL                   	 6.2
#define DC_SUPER_FAST             		 5.5

/**
 * todo: avoid using globals. You can use semaphores or make this interrupt based.
 */
bool stop_flag = false;



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
	//MotorControl.setDC(6.5);

	//	MotorControl.setDC(DC_SUPER_SLOW);
	/**
	 * todo: do not use a while loop. use periodic tasks to receive and parse messages.
	 */
	while (CAN_rx(can1, &can_msg2, 0))
	{
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg2.frame_fields.data_len;
		can_msg_hdr.mid = can_msg2.msg_id;
		//	printf("%d",can_msg2.msg_id);
#if 0
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
			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed!=0)
				printf(" %d ",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);
			//	printf("d--> %d\n ",mDirection_cmd_msg.MOTORIO_DIRECTION_direction);

			//	printf("t--> %d\n ",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
			//	u0_dbg_printf("Speed is %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);

			if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
			{
				dc_accelerate(DC_SUPER_SLOW);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
			{
				dc_accelerate(DC_SUPER_SLOW);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == STOP)
			{
				//dc_stop();
				dc_accelerate(DC_STOP);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == FAST)
			{
				//dc_stop();
				dc_accelerate(DC_SUPER_FAST);
			}

			if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_LEFT)
			{
				MotorControl.setServo(S_LEFT);
				delay_ms(10);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_LEFT)
			{
				MotorControl.setServo(H_LEFT);
				delay_ms(10);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==STRAIGHT)
			{
				MotorControl.setServo(STRAIGHT_STEER);
				delay_ms(10);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_RIGHT)
			{
				MotorControl.setServo(S_RIGHT);
				delay_ms(10);
			}
			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_RIGHT)
			{
				MotorControl.setServo(H_RIGHT);
				delay_ms(10);
				=======
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
#endif

#if 1
			if(can_msg2.msg_id== RESET_HDR.mid)
			{
				//	dbc_decode_RESET(&reset_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
				LE.off(1);
			}
			//		printf("mid--> %d\n ",can_msg2.msg_id);

			if(can_msg2.msg_id == MOTORIO_DIRECTION_HDR.mid)
			{
				LD.setNumber(00);
				dbc_decode_MOTORIO_DIRECTION(&mDirection_cmd_msg, can_msg2.data.bytes, &can_msg_hdr);
				//if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed!=0)
				//	printf("s--> %d \n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);
				//		printf("t--> %d \n",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);

				printf("t--> %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_turn);
				printf("s--> %d\n",mDirection_cmd_msg.MOTORIO_DIRECTION_speed);

				if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == SLOW)
				{
					dc_accelerate(DC_SLOW);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == NORMAL)
				{
					dc_accelerate(DC_NORMAL);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == STOP)
				{
					dc_stop();
				}
				//			else if(mDirection_cmd_msg.MOTORIO_DIRECTION_speed == FAST)
				//			{
				//				//dc_stop();
				//				dc_accelerate(DC_SUPER_FAST);
				//			}

				if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_LEFT)
				{
					MotorControl.setServo(S_LEFT);
					/**
					 * todo: do NOT use delays if you are putting this in the periodic scheduler.
					 */
					delay_ms(10);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_LEFT)
				{
					MotorControl.setServo(H_LEFT);
					delay_ms(10);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==STRAIGHT)
				{
					MotorControl.setServo(STRAIGHT_STEER);
					delay_ms(10);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==SLIGHT_RIGHT)
				{
					MotorControl.setServo(S_RIGHT);
					delay_ms(10);
				}
				else if(mDirection_cmd_msg.MOTORIO_DIRECTION_turn==HARD_RIGHT)
				{
					MotorControl.setServo(H_RIGHT);
					delay_ms(10);
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
		//			if(dbc_handle_mia_RESET(&reset_cmd_msg, 1))
		//
		//				{
		//					LE.on(1);
		//					reset_cmd_msg.RESET_data=RESET__MIA_MSG.RESET_data;
		//
		//					LE.on(1);
		//
		//				}
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

	bool rpm_sensor(void)
	{
		static int reading = 0,last_reading = 1000;

		// Initialization :
		LPC_PINCON->PINSEL3 |=  (3 << 28); // ADC-4 is on P1.30, select this as ADC0.4

		reading = adc0_get_reading(4); // Read current value of ADC-4
		if((reading - last_reading)>150)
		{
			last_reading = reading;
			return true;
		}
		else
		{
			//printf(" %d ", reading);
			last_reading = reading;
			return false;
		}
		// delay_ms(100);

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
