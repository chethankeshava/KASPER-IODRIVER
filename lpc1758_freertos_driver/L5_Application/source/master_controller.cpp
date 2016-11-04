/*
 * File: master_controller.cpp
 *
 * Implements the functionality for Master Controller.
 *
 *  Created on: 27-Oct-2016
 */

#include <master_controller.h>
#include <can.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include <string.h>

SENSOR_HEARTBEAT_t can_msg_sensor_hb= {0};
MOTORIO_HEARTBEAT_t can_msg_motorio_hb= {0};
BRIDGE_HEARTBEAT_t can_msg_bridge_hb= {0};
GEO_HEARTBEAT_t can_msg_geo_hb= {0};


const uint32_t                             MOTORIO_HEARTBEAT__MIA_MS= 3000;
const MOTORIO_HEARTBEAT_t                  MOTORIO_HEARTBEAT__MIA_MSG={0};
const uint32_t                             SENSOR_HEARTBEAT__MIA_MS= 3000;
const SENSOR_HEARTBEAT_t                   SENSOR_HEARTBEAT__MIA_MSG={0};
const uint32_t                             BRIDGE_HEARTBEAT__MIA_MS= 3000;
const BRIDGE_HEARTBEAT_t                   BRIDGE_HEARTBEAT__MIA_MSG={0};
const uint32_t                             GEO_HEARTBEAT__MIA_MS = 3000;
const GEO_HEARTBEAT_t                      GEO_HEARTBEAT__MIA_MSG={0};


bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id                = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(can1, &can_msg, 0);
}

/* init_can_master : Initialise CAN for master controller
 *
 *  return bool
 */
status_t init_can_master(void)
{
	status_t status = true;
	/*Use can1 for master controller */
	status = CAN_init(can1, 100, 10, 10, NULL, NULL);
	CAN_reset_bus(can1);
	CAN_bypass_filter_accept_all_msgs();

	return status;
}

/*
 * read_controller_heartbeat:
 * Functionality: Receive the heartbeats of all controllers. Called every 1 second.
 * @params void
 * Return type: status_t (bool)
 */
status_t read_controller_heartbeat(void)
{
	status_t status = true, miastatus = false;
	can_msg_t can_msg;
	while (CAN_rx(can1, &can_msg, 0))
	{
		// Form the message header from the metadata of the arriving message
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;
#ifdef DEBUG_PRINTF
		printf("\nReceived HeartBeat");
#endif
		if(can_msg_hdr.mid == SENSOR_HEARTBEAT_HDR.mid )
		{
#ifdef DEBUG_PRINTF
			printf("\nReceived SENSOR HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == MOTORIO_HEARTBEAT_HDR.mid )
		{
#ifdef DEBUG_PRINTF
			printf("\nReceived MOTORIO HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == BRIDGE_HEARTBEAT_HDR.mid )
		{
#ifdef DEBUG_PRINTF
			printf("\nReceived BRIDGE HeartBeat");
#endif
		}
		else if(can_msg_hdr.mid == GEO_HEARTBEAT_HDR.mid )
		{
#ifdef DEBUG_PRINTF
			printf("\nReceived GEO HeartBeat");
#endif
		}
	}

	/* Handle MIA 's for controller heartbeat */
	miastatus = dbc_handle_mia_SENSOR_HEARTBEAT(&can_msg_sensor_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nSensor HeartBeat MIA");
#endif
		LOG_INFO("\nSensor heartBeat MIA Occurred");
	}
	miastatus = dbc_handle_mia_MOTORIO_HEARTBEAT(&can_msg_motorio_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nMotorio HeartBeat MIA");
#endif
		LOG_INFO("\nMotorio heartBeat MIA Occurred\n");
	}
	miastatus = dbc_handle_mia_BRIDGE_HEARTBEAT(&can_msg_bridge_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nBridge HeartBeat MIA");
#endif
		LOG_INFO("\nBridge heartBeat MIA Occurred");
	}
	miastatus = dbc_handle_mia_GEO_HEARTBEAT(&can_msg_geo_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("\nGeo HeartBeat MIA");
#endif
		LOG_INFO("\nGeo heartBeat MIA Occurred");
	}

	return status;
}

/*
 * send_bridge_ack:
 * Functionality: Send acknowledgement to the bridge to indicate that master
 * 				  is ready to receive data
 * @params void
 * Return type: status_t (bool)
 */
status_t send_bridge_ack(void)
{
	status_t status = true;

	RECEIVE_START_ACK_t ack_data = {0};
	dbc_encode_and_send_RECEIVE_START_ACK(&ack_data);
#ifdef DEBUG_PRINTF
	printf("Send ACK BRidge\n");
#endif
	return status;
}

/*
 * is_bus_off:
 * Functionality: Check if bus if in the OFF state
 * @params void
 * Return type: void
 */
void is_bus_off(void)
{
	if(CAN_is_bus_off(can1))
	{
#ifdef DEBUG_PRINTF
		printf("Bus is Off\n");
#endif
		CAN_reset_bus(can1);
	}
}

