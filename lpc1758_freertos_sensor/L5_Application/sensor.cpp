/*
 * sensor.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: Rimjhim & Shruthi
 */

#include "sensor.hpp"
#include "string.h"
#include "gpio.hpp"
#include "lpc_timers.h"
#include "soft_timer.hpp"
#define Distance_Scale 0.017 /** (340*100*0.000001)/2, speed of sound=340m/s, 1m=100cm, 1us=0.000001s  **/
#define Echo_Return_Pulse 19 /**Echo Return Pulse Maximum 18.5 ms **/

GPIO   LeftSIG(P2_2);   // Left SIG pin
GPIO CenterSIG(P2_0); // Center SIG pin
GPIO  RightSIG(P2_4); // Right SIG pin
//GPIO  BackSIG(P2_6);  //Back SIG pin

SENSOR_SONIC_t sensor_msg={0};

void Sensor(){
	/**
	 * todo: If you are going to use delays here ensure that you do not run over in time.
	 */
	static int sen_count=0;
		if(sen_count%2)
		{
	Sensor_left();
	Sensor_right();
	delay_ms(40);
		}
		else
		{
	Sensor_center();
	//Sensor_back();
	delay_ms(40);
		}

	Transmit();

	sen_count++;
		if(sen_count>1000)
		{
			sen_count=0;
		}

void Sensor()
{
	Sensor_left();
	Sensor_right();
	Sensor_center();
	//Sensor_back();
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
		printf("R:%d ",sensor_msg.SENSORS_SONIC_front_right);
		printf("C:%d\n",sensor_msg.SENSORS_SONIC_front_center);
		//printf("B:%d\n",sensor_msg.SENSORS_SONIC_back);
	}
	else
		LD.setNumber(0);
}

bool flag_left = true;
bool flag_ping_high_left = true;
SoftTimer ping_duration_left;
int time_left;
int distance_left;

void Sensor_left(void)
{
	ping_duration_left.reset(Echo_Return_Pulse);
	ping_duration_left.restart();
	while(1)
	{
		/**ranging**/
		if(flag_left)
		{
			LeftSIG.setAsOutput(); //make SIGpin output
			LeftSIG.setLow(); /**make low for 2us and then high for clean high**/
			delay_us(2);
			LeftSIG.setHigh(); /** enable Ranging (enable left sonar). trigger/make high for 2 탎 (min), 5 탎 typical **/
			delay_us(5);
			LeftSIG.setLow();/**disable ranging**/
			LeftSIG.setAsInput(); //make SIGpin input
			flag_left = false;
		}

		/**start timer when echo pulse is high**/
		if((LPC_GPIO2->FIOPIN & (1 << 2)) && !flag_ping_high_left)
		{
			lpc_timer_enable(lpc_timer0,1);
			lpc_timer_set_value(lpc_timer0,0);
			flag_ping_high_left = true;
		}

		/**calculate distance when echo pulse goes low**/
		if(!(LPC_GPIO2->FIOPIN & (1 << 2)) && flag_ping_high_left)
		{
			time_left = lpc_timer_get_value(lpc_timer0);
			distance_left = (Distance_Scale)*time_left;
			//		printf("left: %d ",distance_left);
			sensor_msg.SENSORS_SONIC_front_left=distance_left;
			flag_left = true;
			flag_ping_high_left = false;
			break;
		}

		/**no echo**/
		if(ping_duration_left.expired())
		{
			flag_left = true;
			flag_ping_high_left = false;
			break;
		}
	}
}

bool flag_center = true;
bool flag_ping_high_center = true;
SoftTimer ping_duration_center;
int time_center;
int distance_center;

void Sensor_center(void)
{
	ping_duration_center.reset(Echo_Return_Pulse);
	ping_duration_center.restart();
	while(1)
	{
		if(flag_center)
		{
			CenterSIG.setAsOutput(); //make SIGpin output
			CenterSIG.setLow();/**make low for 2us and then high for clean high**/
			delay_us(2);
			CenterSIG.setHigh();/** enable Ranging (enable center sonar). make high for 2 탎 (min), 5 탎 typical **/
			delay_us(5);
			CenterSIG.setLow();/**disable ranging**/
			CenterSIG.setAsInput(); //make SIGpin input
			flag_center = false;
		}

		if((LPC_GPIO2->FIOPIN & (1 << 0)) && !flag_ping_high_center)
		{
			lpc_timer_enable(lpc_timer0,1);
			lpc_timer_set_value(lpc_timer0,0);
			flag_ping_high_center = true;
		}
		if(!(LPC_GPIO2->FIOPIN & (1 << 0)) && flag_ping_high_center)
		{
			time_center = lpc_timer_get_value(lpc_timer0);
			distance_center = (Distance_Scale)*time_center;
			//printf("center: %d\n",distance_center);
			sensor_msg.SENSORS_SONIC_front_center=distance_center;
			flag_center = true;
			flag_ping_high_center = false;
			break;
		}
		if(ping_duration_center.expired())
		{
			flag_center = true;
			flag_ping_high_center = false;
			break;
		}
	}
}

bool flag_right = true;
bool flag_ping_high_right = true;
SoftTimer ping_duration_right;
int time_right;
int distance_right;

void Sensor_right(void)
{
	ping_duration_right.reset(Echo_Return_Pulse);
	ping_duration_right.restart();
	while(1)
	{
		if(flag_right)
		{
			RightSIG.setAsOutput();//make SIGpin output
			RightSIG.setLow();/**make low for 2us and then high for clean high**/
			delay_us(2);
			RightSIG.setHigh();/** enable Ranging (enable right sonar). make high for 2 탎 (min), 5 탎 typical **/
			delay_us(5);
			RightSIG.setLow();/**disable ranging**/
			RightSIG.setAsInput(); //make SIGpin input
			flag_right = false;
		}

		if((LPC_GPIO2->FIOPIN & (1 << 4)) && !flag_ping_high_right)
		{
			lpc_timer_enable(lpc_timer0,1);
			lpc_timer_set_value(lpc_timer0,0);
			flag_ping_high_right = true;
		}
		if(!(LPC_GPIO2->FIOPIN & (1 << 4)) && flag_ping_high_right)
		{
			time_right = lpc_timer_get_value(lpc_timer0);
			distance_right = (Distance_Scale)*time_right;
			//printf("right: %d ",distance_right);
			sensor_msg.SENSORS_SONIC_front_right=distance_right;
			flag_right = true;
			flag_ping_high_right = false;
			break;
		}
		if(ping_duration_right.expired())
		{
			flag_right = true;
			flag_ping_high_right = false;
			break;
		}
	}
}

//bool flag_back = true;
//bool flag_ping_high_back = true;
//SoftTimer ping_duration_back;
//int time_back;
//int distance_back;
//SoftTimer lpc_timer_back;
//void Sensor_back(void)
//{
//	ping_duration_back.reset(Echo_Return_Pulse);
//		ping_duration_back.restart();
//		while(1)
//		{
//			if(flag_back)
//			{
//				BackSIG.setAsOutput();
//			BackSIG.setLow();/**make low for 2us and then high for clean high**/
//			delay_us(2);
//				BackSIG.setHigh();/** enable Ranging (enable left sonar). make high for 2 탎 (min), 5 탎 typical **/
//				delay_us(5);
//				BackSIG.setLow();/**disable ranging**/
//				BackSIG.setAsInput();//make SIGpin input
//				flag_back = false;
//			}
//
//			if((LPC_GPIO2->FIOPIN & (1 << 0)) && !flag_ping_high_back)
//			{
//				lpc_timer_enable(lpc_timer0,1);
//				lpc_timer_set_value(lpc_timer0,0);
//				flag_ping_high_back = true;
//			}
//			if(!(LPC_GPIO2->FIOPIN & (1 << 0)) && flag_ping_high_back)
//			{
//				time_back = lpc_timer_get_value(lpc_timer0);
//				distance_back = (Distance_Scale)*time_back;
//		//printf("back: %d\n",distance_back);
//				sensor_msg.SENSORS_SONIC_back=distance_back;
//				flag_back = true;
//				flag_ping_high_back = false;
//				break;
//			}
//			if(ping_duration_back.expired())
//			{
//				flag_back = true;
//				flag_ping_high_back = false;
//				break;
//			}
//		}
//}
