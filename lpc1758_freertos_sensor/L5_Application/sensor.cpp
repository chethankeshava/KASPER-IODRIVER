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
#define Echo_Return_Pulse 18.5 /**Echo Return Pulse Maximum 18.5 ms **/

GPIO LeftSIG(P2_2); // Left SIG pin
GPIO CenterSIG(P2_0); // Center SIG pin
GPIO RightSIG(P2_4); // Right SIG pin
GPIO BackSIG(P2_5); //Back SIG pin

SENSOR_SONIC_t sensor_msg={0};

void Sensor()
{
	Sensor_left();
	Sensor_right();
	Sensor_center();
	Sensor_back();
}

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id  = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);
	return CAN_tx(can1, &can_msg, 0);
}

void Transmit(void)
{
	if(dbc_encode_and_send_SENSOR_SONIC(&sensor_msg))
	{
		LD.setNumber(88);
		printf("L:%d ",sensor_msg.SENSORS_SONIC_front_left);
		printf("R:%d ",sensor_msg.SENSORS_SONIC_front_right);
		printf("C:%d ",sensor_msg.SENSORS_SONIC_front_center);
		printf("B:%d\n",sensor_msg.SENSORS_SONIC_back);

		/**LED indication for NEAR objects**/
		if(sensor_msg.SENSORS_SONIC_front_left <60 && sensor_msg.SENSORS_SONIC_front_center <60 && sensor_msg.SENSORS_SONIC_front_right <60)
		{
			LE.on(1);
			LE.on(2);
			LE.on(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_left <60 && sensor_msg.SENSORS_SONIC_front_center <60)
		{
			LE.off(1);
			LE.on(2);
			LE.on(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_right <60 && sensor_msg.SENSORS_SONIC_front_center <60)
		{
			LE.on(1);
			LE.on(2);
			LE.off(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_left <60 && sensor_msg.SENSORS_SONIC_front_right <60)
		{
			LE.on(1);
			LE.off(2);
			LE.on(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_left <60)
		{
			LE.off(1);
			LE.off(2);
			LE.on(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_right <60)
		{
			LE.on(1);
			LE.off(2);
			LE.off(3);
			LE.off(4);
		}
		else if(sensor_msg.SENSORS_SONIC_front_center <60)
		{
			LE.off(1);
			LE.on(2);
			LE.off(3);
			LE.off(4);
		}
		else
		{
			LE.off(1);
			LE.off(2);
			LE.off(3);
			LE.off(4);
		}
		if(sensor_msg.SENSORS_SONIC_back <60)
		{
			LE.on(4);
		}
	}
}

void Sensor_left(void)
{
	bool flag_left = true;
	bool flag_ping_high_left = false;
	SoftTimer ping_duration_left;
	int time_left;
	int distance_left;

	ping_duration_left.reset(Echo_Return_Pulse);
	ping_duration_left.restart();

	while(1)
	{
		/**make SIGpin output, enable ranging, make SIGpin input**/
		if(flag_left)
		{
			LeftSIG.setAsOutput(); /**make SIGpin output**/
			LeftSIG.setLow(); /**set low for 2us and then high for clean high**/
			delay_us(2);
			LeftSIG.setHigh(); /** enable Ranging (enable left sonar). trigger/set high for 2 탎 (min), 5 탎 typical, Ultrasonic burst **/
			delay_us(5);
			LeftSIG.setLow();/**set low**/
			LeftSIG.setAsInput(); /**make SIGpin input**/
			flag_left = false;
		}

		/**start timer when echo pulse is high**/
		if((LPC_GPIO2->FIOPIN & (1 << 2)) && !flag_ping_high_left)
		{
			lpc_timer_enable(lpc_timer0,1); /**enable timer0**/
			lpc_timer_set_value(lpc_timer0,0); /**set timer0 value=0**/
			flag_ping_high_left = true;
		}

		/**calculate distance when echo pulse goes low**/
		if(!(LPC_GPIO2->FIOPIN & (1 << 2)) && flag_ping_high_left)
		{
			time_left = lpc_timer_get_value(lpc_timer0);
			distance_left = (Distance_Scale)*time_left;
			//printf("left: %d ",distance_left);
			sensor_msg.SENSORS_SONIC_front_left=distance_left;
			break;
		}

		/**no echo**/
		if(ping_duration_left.expired())
		{
			break;
		}
	}
}

void Sensor_center(void)
{
	bool flag_center = true;
	bool flag_ping_high_center = false;
	SoftTimer ping_duration_center;
	int time_center;
	int distance_center;

	ping_duration_center.reset(Echo_Return_Pulse);
	ping_duration_center.restart();

	while(1)
	{
		/**make SIGpin output, enable ranging, make SIGpin input**/
		if(flag_center)
		{
			CenterSIG.setAsOutput(); /**make SIGpin output**/
			CenterSIG.setLow();/**set low for 2us and then high for clean high**/
			delay_us(2);
			CenterSIG.setHigh();/**enable Ranging (enable center sonar). set high for 2 탎 (min), 5 탎 typical, Ultrasonic burst **/
			delay_us(5);
			CenterSIG.setLow(); /**set low**/
			CenterSIG.setAsInput(); /**make SIGpin input**/
			flag_center = false;
		}

		/**start timer when echo pulse is high**/
		if((LPC_GPIO2->FIOPIN & (1 << 0)) && !flag_ping_high_center)
		{
			lpc_timer_enable(lpc_timer0,1); /**enable timer0**/
			lpc_timer_set_value(lpc_timer0,0); /**set timer0 value=0**/
			flag_ping_high_center = true;
		}

		/**calculate distance when echo pulse goes low**/
		if(!(LPC_GPIO2->FIOPIN & (1 << 0)) && flag_ping_high_center)
		{
			time_center = lpc_timer_get_value(lpc_timer0);
			distance_center = (Distance_Scale)*time_center;
			//printf("center: %d\n",distance_center);
			sensor_msg.SENSORS_SONIC_front_center=distance_center;
			break;
		}

		/**no echo**/
		if(ping_duration_center.expired())
		{
			break;
		}
	}
}

void Sensor_right(void)
{
	bool flag_right = true;
	bool flag_ping_high_right = false;
	SoftTimer ping_duration_right;
	int time_right;
	int distance_right;

	ping_duration_right.reset(Echo_Return_Pulse);
	ping_duration_right.restart();

	while(1)
	{
		/**make SIGpin output, enable ranging, make SIGpin input**/
		if(flag_right)
		{
			RightSIG.setAsOutput();/**make SIGpin output**/
			RightSIG.setLow();/**set low for 2us and then high for clean high**/
			delay_us(2);
			RightSIG.setHigh();/** enable Ranging (enable right sonar). set high for 2 탎 (min), 5 탎 typical, Ultrasonic burst **/
			delay_us(5);
			RightSIG.setLow(); /**set low**/
			RightSIG.setAsInput(); /**make SIGpin input**/
			flag_right = false;
		}

		/**start timer when echo pulse is high**/
		if((LPC_GPIO2->FIOPIN & (1 << 4)) && !flag_ping_high_right)
		{
			lpc_timer_enable(lpc_timer0,1); /**enable timer0**/
			lpc_timer_set_value(lpc_timer0,0); /**set timer0 value=0**/
			flag_ping_high_right = true;
		}

		/**calculate distance when echo pulse goes low**/
		if(!(LPC_GPIO2->FIOPIN & (1 << 4)) && flag_ping_high_right)
		{
			time_right = lpc_timer_get_value(lpc_timer0);
			distance_right = (Distance_Scale)*time_right;
			//printf("right: %d ",distance_right);
			sensor_msg.SENSORS_SONIC_front_right=distance_right;
			break;
		}

		/**no echo**/
		if(ping_duration_right.expired())
		{
			break;
		}
	}
}

void Sensor_back(void)
{
	bool flag_back = true;
	bool flag_ping_high_back = false;
	SoftTimer ping_duration_back;
	int time_back;
	int distance_back;

	ping_duration_back.reset(Echo_Return_Pulse);
	ping_duration_back.restart();

	while(1)
	{
		/**make SIGpin output, enable ranging, make SIGpin input**/
		if(flag_back)
		{
			BackSIG.setAsOutput(); /**make SIGpin output**/
			BackSIG.setLow();/**set low for 2us and then high for clean high**/
			delay_us(2);
			BackSIG.setHigh();/** enable Ranging (enable left sonar). set high for 2 탎 (min), 5 탎 typical, Ultrasonic burst **/
			delay_us(5);
			BackSIG.setLow(); /**set low**/
			BackSIG.setAsInput();/**make SIGpin input**/
			flag_back = false;
		}

		/**start timer when echo pulse is high**/
		if((LPC_GPIO2->FIOPIN & (1 << 5)) && !flag_ping_high_back)
		{
			lpc_timer_enable(lpc_timer0,1); /**enable timer0**/
			lpc_timer_set_value(lpc_timer0,0); /**set timer0 value=0**/
			flag_ping_high_back = true;
		}

		/**calculate distance when echo pulse goes low**/
		if(!(LPC_GPIO2->FIOPIN & (1 << 5)) && flag_ping_high_back)
		{
			time_back = lpc_timer_get_value(lpc_timer0);
			distance_back = (Distance_Scale)*time_back;
			//printf("back: %d\n",distance_back);
			sensor_msg.SENSORS_SONIC_back=distance_back;
			break;
		}

		/**no echo**/
		if(ping_duration_back.expired())
		{
			break;
		}
	}
}

