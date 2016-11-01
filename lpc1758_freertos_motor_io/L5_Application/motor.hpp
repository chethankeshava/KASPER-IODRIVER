/*
 * motor.hpp
 *
 *  Created on: Oct 31, 2016
 *      Author: prash
 */

#ifndef L5_APPLICATION_MOTOR_HPP_
#define L5_APPLICATION_MOTOR_HPP_

#include "io.hpp"
#include "task.h"
#include "utilities.h"
#include "lpc_pwm.hpp"
#include <stdio.h>


class MotorController : public SingletonTemplate<MotorController>
{
    public:

        void setDC(float v)
        {
        	driveMotor.set(v);
        }
        void setServo(float v)
        {
        	steerMotor.set(v);
        }

    private:
        PWM driveMotor;
        PWM steerMotor;

        MotorController() : driveMotor(PWM::pwm2), steerMotor(PWM::pwm1)
        {

        }

        friend class SingletonTemplate<MotorController>; ///< Friend class used for Singleton Template
};

#define MotorControl MotorController::getInstance()

#ifdef __cplusplus
extern "C"{
#endif

void servo_init(void);
void dc_check(void);

void servo_left(void);
void servo_right(void);
void servo_straight(void);
void dc_accelerate(void);
void dc_stop(void);



#ifdef __cplusplus
}
#endif



#endif /* L5_APPLICATION_MOTOR_HPP_ */
