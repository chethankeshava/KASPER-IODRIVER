/*
 * sensor.hpp
 *
 *  Created on: Nov 28, 2016
 *      Author: Rimjhim & Shruthi
 */

#ifndef L5_APPLICATION_SENSOR_HPP_
#define L5_APPLICATION_SENSOR_HPP_

#include <stdio.h>
#include "io.hpp"
#include "lpc_sys.h"
#include "gpio.hpp"
#include "_can_dbc//generated_can.h"
#include "utilities.h" //delay lib
#include <L3_Utils/singleton_template.hpp>
#include "can.h"
#include "eint.h"

extern GPIO  LeftSIG;   // left SIGpin USE EXTERN so they wont be redeclared every time "sensor.hpp" is called!
extern GPIO CenterSIG; // Center SIGpin USE EXTERN so they wont be redeclared every time "sensor.hpp" is called!
extern GPIO RightSIG; // Right SIGpin USE EXTERN so they wont be redeclared every time "sensor.hpp" is called!
//extern GPIO BackSIG;  //Back SIGpin USE EXTERN so they wont be redeclared every time "sensor.hpp" is called!

void Sensor_left(void);
void Sensor_center(void);
void Sensor_right(void);
//void Sensor_back(void);

void Sensor(void);
void Transmit(void);
#endif /* L5_APPLICATION_SENSOR_HPP_ */
