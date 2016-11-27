/*
 * sensor.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: Rimjhim & Shruthi
 */

#include "sensor.hpp"
#include "string.h"
#include "gpio.hpp"
#include "lpc_timers.h"
#include "soft_timer.hpp"
/**
 * todo: try not to use global variables.
 * 		 Use unsigned numbers here. You don't want some number rollover incident to cause
 * 		 you to crash.
 */

int Trigger_left, Trigger_center, Trigger_right;
//Trigger_back;
int Distance_left, Distance_center, Distance_right;
// Distance_back;

GPIO  LeftSIG(P1_19);   // Left RX pin
GPIO CenterRX(P2_0); // Center RX pin
GPIO  RightRX(P2_4); // Right RX pin
//GPIO  BackRX(P2_6);  //Rear

SENSOR_SONIC_t sensor_msg={0};

void Sensor(){
	Sensor_left();
	/**
	 * todo: If you are going to use delays here ensure that you do not run over in time.
	 */
	//	delay_ms(15);
	//	Sensor_center();
	//	delay_ms(15);
	//	Sensor_right();
	//	delay_ms(15);
	//	Sensor_right();
	//	delay_ms(15);
	//	Transmit();
}

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id  = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);
	// printf("sending..");
	return CAN_tx(can1, &can_msg, 0);
}

void Transmit(void)
{
	if(dbc_encode_and_send_SENSOR_SONIC(&sensor_msg))
	{
		LD.setNumber(88);
		printf("L:%d ",sensor_msg.SENSORS_SONIC_front_left);
		printf("C:%d ",sensor_msg.SENSORS_SONIC_front_center);
		printf("R:%d\n",sensor_msg.SENSORS_SONIC_front_right);
		//printf("B:%d\n",sensor_msg.SENSORS_SONIC_back);
	}
}

//void Calculate_Distance_left(void)
//{
//	/**
//	 * todo: avoid using magic numbers like 147.
//	 */
//	Distance_left = ((sys_get_uptime_us() - Trigger_left)/147) ; //each 147uS is 1 inch (Datasheet)
//	sensor_msg.SENSORS_SONIC_front_left=Distance_left;
//}
//
//void Calculate_Distance_center(void)
//{
//	Distance_center = ((sys_get_uptime_us() - Trigger_center) / 147) ; //each 147uS is 1 inch (Datasheet)
//	sensor_msg.SENSORS_SONIC_front_center=Distance_center;
//}
//
//void Calculate_Distance_right(void)
//{
//	Distance_right = ((sys_get_uptime_us() - Trigger_right) / 147) ; //each 147uS is 1 inch (Datasheet)
//	sensor_msg.SENSORS_SONIC_front_right=Distance_right;
//}

//void Calculate_Distance_back(void)
//{
//    Distance_back = ((sys_get_uptime_us() - Trigger_back) / 147) ; //each 147uS is 1 inch (Datasheet)
// sensor_msg.SENSORS_SONIC_back=Distance_back;
//}

bool flag = true;
bool flag_ping_high = true;
SoftTimer ping_duration;
int time_left;
int distance_left;
SoftTimer lpc_timer;

void Sensor_left(void)
{
	ping_duration.reset(20);
	ping_duration.restart();
	while(1)
	{
		if(flag)
		{
			LeftSIG.setAsOutput();
			LeftSIG.setHigh(); // enable Ranging   (enable left sonar)
			delay_us(5);
			LeftSIG.setLow();
			LeftSIG.setAsInput();
			flag = false;
		}

		if((LPC_GPIO1->FIOPIN & (1 << 19)) && !flag_ping_high)
		{
			lpc_timer_enable(lpc_timer0,1);
			lpc_timer_set_value(lpc_timer0,0);
			flag_ping_high = true;
		}
		if(!(LPC_GPIO1->FIOPIN & (1 << 19)) && flag_ping_high)
		{
			time_left = lpc_timer_get_value(lpc_timer0);
			distance_left = (340)*(100)*time_left * (0.000001)/2;
			printf("center: %d\n",distance_left);
			flag = true;
			flag_ping_high = false;
			break;
		}
		if(ping_duration.expired())
		{
			flag = true;
			flag_ping_high = false;
			break;
		}
	}
}

/**
 * todo: magic number - 21
 */
//delay_us(21); //hold high  >20uS to enable ranging

//Trigger_left = sys_get_uptime_us(); //get timer at the moment ranging starts
//LeftRX.setLow(); // disable ranging of left sonar
//}

//void Sensor_center(void)
//{
//	CenterRX.setHigh(); // enable Ranging   (enable center sonar)
//	delay_us(21); //hold high  >20uS to enable ranging
//	Trigger_center = sys_get_uptime_us(); //get timer at the moment ranging starts
//	CenterRX.setLow(); // disable ranging of center sonar
//}
//
//void Sensor_right(void)
//{
//	RightRX.setHigh(); // enable Ranging   (enable right sonar)
//	delay_us(21); //hold high  >20uS to enable ranging
//	Trigger_right = sys_get_uptime_us(); //get timer at the moment ranging starts
//	RightRX.setLow(); // disable ranging of right sonar
//}

//void Sensor_back(void)
//{
//    BackRX.setHigh(); // enable Ranging   (enable back sonar)
//    delay_us(21); //hold high  >20uS to enable ranging
//    Trigger_back = sys_get_uptime_us(); //get timer at the moment ranging starts
//    BackRX.setLow(); // disable ranging of back sonar
//}



