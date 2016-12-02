/*
 * geo.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: ankit
 */
#include <stdio.h>

#include <stdint.h>
#include <string.h>
#include "geo.hpp"
#include "can.h"
#include "../_can_dbc/generated_can.h"

#define GEO_HEARTBEAT_SIGNATURE	0xAA




/**************************************************************************************************
*
**************************************************************************************************/
bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(GEO_CAN_BUS, &can_msg, 0);
}






void geoSendHeartBeat()
{
	GEO_HEARTBEAT_t geoHeartBeat={0};

	geoHeartBeat.GEO_HEARTBEAT_data = GEO_HEARTBEAT_SIGNATURE;
	dbc_encode_and_send_GEO_HEARTBEAT(&geoHeartBeat);
}
