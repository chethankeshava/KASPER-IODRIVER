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

GPIO   LeftSIG(P2_2);   // Left SIG pin
GPIO CenterSIG(P2_0); // Center SIG pin
GPIO  RightSIG(P2_4); // Right SIG pin
//GPIO  BackSIG(P2_6);  //Back SIG pin

SENSOR_SONIC_t sensor_msg={0};

void Sensor()
{
	static int sen_count=0;
	if(sen_count%2)
	{
		Sensor_left();
		Sensor_right();

	}
	else
	{
		Sensor_center();
		//Sensor_back();

	}

	sen_count++;
if(sen_count>1000)
	{
		sen_count=0;
	}
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
		LD.setNumber(8);
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
SoftTimer lpc_timer_left;

void Sensor_left(void)
{
	ping_duration_left.reset(19); /****/
	ping_duration_left.restart();
	while(1)
	{
		if(flag_left)
		{
			LeftSIG.setAsOutput(); //make pin output
			/**make low for 2us and then high for clean high**/
			LeftSIG.setHigh(); // enable Ranging   (enable left sonar)
			delay_us(5);
			LeftSIG.setLow();
			LeftSIG.setAsInput();
			flag_left = false;
		}

		if((LPC_GPIO2->FIOPIN & (1 << 2)) && !flag_ping_high_left)
		{
			lpc_timer_enable(lpc_timer0,1);
			lpc_timer_set_value(lpc_timer0,0);
			flag_ping_high_left = true;
		}

		if(!(LPC_GPIO2->FIOPIN & (1 << 2)) && flag_ping_high_left)
		{
			time_left = lpc_timer_get_value(lpc_timer0);
			distance_left = (0.017)*time_left;
	//		printf("left: %d ",distance_left);
			sensor_msg.SENSORS_SONIC_front_left=distance_left;
			flag_left = true;
			flag_ping_high_left = false;
			break;
		}

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
SoftTimer lpc_timer_center;

void Sensor_center(void)
{
	ping_duration_center.reset(19);
	ping_duration_center.restart();
	while(1)
	{
		if(flag_center)
		{
			CenterSIG.setAsOutput();
			CenterSIG.setHigh(); // enable Ranging   (enable left sonar)
			delay_us(5);
			CenterSIG.setLow();
			CenterSIG.setAsInput();
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
			distance_center = (0.017)*time_center;
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
SoftTimer lpc_timer_right;


void Sensor_right(void)
{
	ping_duration_right.reset(19);
	ping_duration_right.restart();
	while(1)
	{
		if(flag_right)
		{
			RightSIG.setAsOutput();
			RightSIG.setHigh(); // enable Ranging   (enable left sonar)
			delay_us(5);
			RightSIG.setLow();
			RightSIG.setAsInput();
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
			distance_right = (0.017)*time_right;
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
//	ping_duration_back.reset(19);
//		ping_duration_back.restart();
//		while(1)
//		{
//			if(flag_back)
//			{
//				BackSIG.setAsOutput();
//				BackSIG.setHigh(); // enable Ranging   (enable left sonar)
//				delay_us(5);
//				BackSIG.setLow();
//				BackSIG.setAsInput();
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
//				distance_back = (0.017)*time_back;
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



