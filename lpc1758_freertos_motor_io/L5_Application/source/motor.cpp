/*
 * motor.cpp
 *
 *  Created on: Oct 31, 2016
 *      Author: prash
 */
#include "motor.hpp"
#include "../_can_dbc/generated_can.h"
#include "can.h"



#define HARD_LEFT                           9.0
#define S_LEFT                              8.0
#define HARD_RIGHT                          5.7
#define S_RIGHT                             6.5
#define STRAIGHT                            7.5

#define SPEED_VAR_FACTOR                    0.05


#define DC_ON                               1
#define SERVO_ON                            1


#define DC_STOP                             7.0
#define DC_SUPER_SLOW                       6.5

#define DC_THRESH_SLOW                      6.1



//As per waveform and real testing(percent range - 6.0(right) - 7.5(center) - 9.3(left))
//As per waveform only(percent range - 5.5(forward) - 8.5(stop) - 10.5(backward))

MOTORIO_DIRECTION_t mDirection_cmd_msg = { 0 };
can_msg_t can_msg;
const uint32_t            MOTORIO_DIRECTION__MIA_MS = 3;

const MOTORIO_DIRECTION_t      MOTORIO_DIRECTION__MIA_MSG = { 4 };


void dc_check(void){
	//puts("entered");
	//  MotorControl.setDC(6.10);
	delay_ms(2000);
	MotorControl.setDC(DC_STOP);
	delay_ms(2000);
	//MotorControl.setDC(10.9);

	MotorControl.setDC(6.5);
	delay_ms(2000);
	MotorControl.setDC(DC_STOP);
	delay_ms(2000);
	//puts("exit");

}

//void set_motors_pwm(void)
//{
//
//	mDirection_cmd_msg.MOTORIO_DIRECTION_direction== = motor_msg.MOTORIO_DIRECTION_turn_cmd;
//    md.speed = motor_msg.MOTORIO_DIRECTION_speed_cmd;
//
//    if (md.turn == left)
//    { // turn hard left
//        MotorControl.setServo(HARD_LEFT);
//    }
//    else if(md.turn==s_left)
//    { // turn slight left
//        MotorControl.setServo(S_LEFT);
//    }
//    else if(md.turn==straight)
//    { // keep straight
//        MotorControl.setServo(STRAIGHT);
//    }
//    else if(md.turn==s_right)
//    { // turn slight right
//        MotorControl.setServo(S_RIGHT);
//    }
//    else if(md.turn==right)
//    { // turn hard right
//        MotorControl.setServo(HARD_RIGHT);
//    }
//
//    //--------------------------- Speed of DC Motor ------------------------
//    LD.setNumber(white_mark_count); // Show white_mark_count on Segment display for speed feedback
//
//    if (md.speed == stop)
//    { // Stop motor
//        speed_factor = dc_stop;
//        desired_count = 0;
//    }
//    else if (md.speed == slow)
//    { // Slow speed
//        speed_factor = dc_slow;
//        desired_count = dc_slow_count;
//
//    }
//    else if (md.speed == normal)
//    { // Normal Speed
//        speed_factor = dc_normal;
//        desired_count = dc_normal_count;
//
//    }
//    else if (md.speed == turbo)
//    { // Turbo Speed
//        speed_factor = dc_turbo;
//        desired_count = dc_turbo_count;
//    }
//}
void servo_init(void)
{
	//printf("Inite\n");

	float factor = 5.5;
	MotorControl.setServo(STRAIGHT);
	delay_ms(100);

	while(factor<9.3)
	{
		MotorControl.setServo(factor);
		factor += 0.1;
		delay_ms(50);
	}

	while(factor>5.5)
	{

		MotorControl.setServo(factor);
		factor -= 0.1;
		delay_ms(50);
	}
	MotorControl.setServo(STRAIGHT); // Set servo straight again
}
void servo_left(void)
{
	MotorControl.setServo(HARD_LEFT);
	//   delay_ms(100);

}


void servo_right(void)
{
	//printf("Inite\n");


	MotorControl.setServo(HARD_RIGHT);
	//    delay_ms(100);
}
void servo_straight(void)
{
	//printf("Inite\n");


	MotorControl.setServo(STRAIGHT);
	//  delay_ms(100);

}

void dc_accelerate(void)
{
	MotorControl.setDC(DC_SUPER_SLOW);
	delay_ms(100);
	//MotorControl.setDC(DC_STOP);

}
void dc_stop(void)
{

	MotorControl.setDC(DC_STOP);
	delay_ms(100);
}




