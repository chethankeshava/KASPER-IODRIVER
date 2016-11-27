#ifndef L5_APPLICATION_MOTOR_HPP_
#define L5_APPLICATION_MOTOR_HPP_
#include "io.hpp"
#include "task.h"
#include "utilities.h"
#include "lpc_pwm.hpp"
#include <stdio.h>
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
	FAST
};
class MotorController : public SingletonTemplate<MotorController>
{
public:
	void setDC(float v);
	void setServo(float v);
private:
	PWM driveMotor;
	PWM steerMotor;
	MotorController();
	friend class SingletonTemplate<MotorController>;
};
#define MotorControl MotorController::getInstance()
#ifdef __cplusplus
extern "C"{
#endif
void servo_left(void);
void servo_right(void);
void servo_straight(void);
void dc_accelerate(float pwmValue);
void dc_stop(void);
void drive_car(void);
void handle_motor_mia(void);
bool rpm_sensor(void);
void dcmotor_init(void);

#ifdef __cplusplus
}
#endif
#endif /* L5_APPLICATION_MOTOR_HPP_ */
