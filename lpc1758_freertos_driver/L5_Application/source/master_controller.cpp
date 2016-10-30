/*
 * master_controller.cpp
 *
 *  Created on: 27-Oct-2016
 */
#include <master_controller.h>
#include <can.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <../_can_dbc/generated_can.h>
#include "file_logger.h"
#include "io.hpp"
#include <string.h>

#ifdef MASTER_FULLCAN
can_std_id_t motorio_can_id;
can_std_id_t bridge_can_id;
can_std_id_t geo_can_id;
can_std_id_t sensor_can_id;

can_std_id_t motorio_power_can_id;
can_std_id_t bridge_power_can_id;
can_std_id_t geo_power_can_id;
can_std_id_t sensor_power_can_id;
#else
SENSOR_POWER_SYNC_t can_msg_sensor_ps = {0};
MOTOR_POWER_SYNC_t can_msg_motorio_ps= {0};
BRIDGE_POWER_SYNC_t can_msg_bridge_ps= {0};
GEO_POWER_SYNC_t can_msg_geo_ps= {0};

SENSOR_HEARTBEAT_t can_msg_sensor_hb= {0};
MOTORIO_HEARTBEAT_t can_msg_motorio_hb= {0};
BRIDGE_HEARTBEAT_t can_msg_bridge_hb= {0};
GEO_HEARTBEAT_t can_msg_geo_hb= {0};

#if 0
const uint32_t                             MOTOR_POWER_SYNC__MIA_MS;
const MOTOR_POWER_SYNC_t                   MOTOR_POWER_SYNC__MIA_MSG ={0};
const uint32_t                             SENSOR_POWER_SYNC__MIA_MS;
const SENSOR_POWER_SYNC_t                  SENSOR_POWER_SYNC__MIA_MSG={0};
const uint32_t                             GEO_POWER_SYNC__MIA_MS;
const GEO_POWER_SYNC_t                     GEO_POWER_SYNC__MIA_MSG={0};
const uint32_t                             BRIDGE_POWER_SYNC__MIA_MS;
const BRIDGE_POWER_SYNC_t                  BRIDGE_POWER_SYNC__MIA_MSG={0};
#endif
const uint32_t                             MOTORIO_HEARTBEAT__MIA_MS= 3000;
const MOTORIO_HEARTBEAT_t                  MOTORIO_HEARTBEAT__MIA_MSG={0};
const uint32_t                             SENSOR_HEARTBEAT__MIA_MS= 3000;
const SENSOR_HEARTBEAT_t                   SENSOR_HEARTBEAT__MIA_MSG={0};
const uint32_t                             BRIDGE_HEARTBEAT__MIA_MS= 3000;
const BRIDGE_HEARTBEAT_t                   BRIDGE_HEARTBEAT__MIA_MSG={0};
const uint32_t                             GEO_HEARTBEAT__MIA_MS = 3000;
const GEO_HEARTBEAT_t                      GEO_HEARTBEAT__MIA_MSG={0};
#endif

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
#ifdef MASTER_FULLCAN
	/* Power Sync Can ID's */
	motorio_power_can_id = CAN_gen_sid(can1,MOTORIO_HEARTBEAT_HDR.mid);
	bridge_power_can_id  = CAN_gen_sid(can1,BRIDGE_HEARTBEAT_HDR.mid);
	geo_power_can_id = CAN_gen_sid(can1,GEO_HEARTBEAT_HDR.mid);
	sensor_power_can_id = CAN_gen_sid(can1,SENSOR_HEARTBEAT_HDR.mid);
	/* HeartBeat CAN ID's */
	motorio_can_id = CAN_gen_sid(can1,MOTORIO_HEARTBEAT_HDR.mid);
	bridge_can_id  = CAN_gen_sid(can1,BRIDGE_HEARTBEAT_HDR.mid);
	geo_can_id = CAN_gen_sid(can1,GEO_HEARTBEAT_HDR.mid);
	sensor_can_id = CAN_gen_sid(can1,SENSOR_HEARTBEAT_HDR.mid);


	status = CAN_fullcan_add_entry(can1,motorio_power_can_id,bridge_power_can_id);
	status = CAN_fullcan_add_entry(can1,geo_power_can_id,sensor_power_can_id);
	status = CAN_fullcan_add_entry(can1,motorio_can_id,bridge_can_id);
	status = CAN_fullcan_add_entry(can1,geo_can_id,sensor_can_id);
#endif

	return status;

}
/* check_controller_powerdup : Check for All controllers powerd up
 *
 *  return bool
 */
status_t check_controller_powerdup(void)
{
	status_t status = true,miastatus = false;
#ifdef MASTER_FULLCAN
	can_fullcan_msg_t *can_msg_ptr;
	can_fullcan_msg_t can_msg_power_sync;

	/* Check if sensor is powered up */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(sensor_power_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_power_sync);
	if(status != true)
	{
		/* Missed Sensor Power Sync */
	}
	/* Check if motorio is powered up */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(motorio_power_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_power_sync);
	if(status != true)
	{
		/* Missed Motorio power sync */
	}
	/* Check if Geo is powered up */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(geo_power_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_power_sync);
	if(status != true)
	{
		/* Missed Geo power sync */
	}
	/* Check if bridge is powered up */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(bridge_power_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_power_sync);
	if(status != true)
	{
		/* Missed Bridge power sync */
	}

	if(status == true)
		LOG_INFO(" All the controllers are powerd up");
#else
	can_msg_t can_msg;
	while (CAN_rx(can1, &can_msg, 0))
	{
		// Form the message header from the metadata of the arriving message
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;

		if(can_msg_hdr.mid == SENSOR_POWER_SYNC_HDR.mid )
		{
			dbc_decode_SENSOR_POWER_SYNC(&can_msg_sensor_ps,can_msg.data.bytes, &can_msg_hdr);
		}
		else if(can_msg_hdr.mid == MOTOR_POWER_SYNC_HDR.mid )
		{
			dbc_decode_MOTOR_POWER_SYNC(&can_msg_motorio_ps,can_msg.data.bytes, &can_msg_hdr);
		}
		else if(can_msg_hdr.mid == BRIDGE_POWER_SYNC_HDR.mid )
		{
			dbc_decode_BRIDGE_POWER_SYNC(&can_msg_bridge_ps,can_msg.data.bytes, &can_msg_hdr);
		}
		else if(can_msg_hdr.mid == GEO_POWER_SYNC_HDR.mid )
		{
			dbc_decode_GEO_POWER_SYNC(&can_msg_geo_ps,can_msg.data.bytes, &can_msg_hdr);
		}
	}
#if 0 /* Enable this when power sync is called from right periodic callback */
	/* Handle MIA 's for powersync */
	miastatus = dbc_handle_mia_SENSOR_POWER_SYNC(&can_msg_sensor_ps, 10);
	miastatus |= dbc_handle_mia_MOTOR_POWER_SYNC(&can_msg_motorio_ps, 10);
	miastatus |= dbc_handle_mia_BRIDGE_POWER_SYNC(&can_msg_bridge_ps, 10);
	miastatus |= dbc_handle_mia_GEO_POWER_SYNC(&can_msg_geo_ps, 10);
	if(miastatus == true)
	{
		LOG_INFO("PowerSync MIA Occurred\n");
	}
#endif
#endif
	return status;
}
/* read_controller_heartbeat : Read Heartbeat of controller periodically
 *
 *  return bool
 */
status_t read_controller_heartbeat(void)
{
	status_t status = true,miastatus = false;
#ifdef MASTER_FULLCAN
	can_fullcan_msg_t *can_msg_ptr;
	can_fullcan_msg_t can_msg_heartbeat;

	/* read Sensor HeartBeat */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(sensor_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_heartbeat);
	if(status != true)
	{
		/* Missed Sensor HeartBeat */
	}
	/* read Motorio HeartBeat */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(motorio_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_heartbeat);
	if(status != true)
	{
		/* Missed Motorio HeartBeat */
	}
	/* read Bridge HeartBeat */
	can_msg_ptr = CAN_fullcan_get_entry_ptr(bridge_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_heartbeat);
	if(status != true)
	{
		/* Missed Bridge HeartBeat */
	}
	can_msg_ptr = CAN_fullcan_get_entry_ptr(geo_can_id);
	status = CAN_fullcan_read_msg_copy(can_msg_ptr, &can_msg_heartbeat);
	if(status != true)
	{
		/* Missed Geo HeartBeat */
	}
#else
	can_msg_t can_msg;
	while (CAN_rx(can1, &can_msg, 0))
	{
		// Form the message header from the metadata of the arriving message
		dbc_msg_hdr_t can_msg_hdr;
		can_msg_hdr.dlc = can_msg.frame_fields.data_len;
		can_msg_hdr.mid = can_msg.msg_id;
#ifdef DEBUG_PRINTF
		printf("Receieved HeartBeat\n");
#endif
		if(can_msg_hdr.mid == SENSOR_HEARTBEAT_HDR.mid )
		{
			dbc_decode_SENSOR_HEARTBEAT(&can_msg_sensor_hb,can_msg.data.bytes, &can_msg_hdr);
			LD.setNumber(11);
#ifdef DEBUG_PRINTF
			printf("Received SENSOR HeartBeat\n");
#endif

		}
		else if(can_msg_hdr.mid == MOTORIO_HEARTBEAT_HDR.mid )
		{
			dbc_decode_MOTORIO_HEARTBEAT(&can_msg_motorio_hb,can_msg.data.bytes, &can_msg_hdr);
			LD.setNumber(22);
#ifdef DEBUG_PRINTF
			printf("Received MOTORIO HeartBeat\n");
#endif
		}
		else if(can_msg_hdr.mid == BRIDGE_HEARTBEAT_HDR.mid )
		{
			dbc_decode_BRIDGE_HEARTBEAT(&can_msg_bridge_hb,can_msg.data.bytes, &can_msg_hdr);
			LD.setNumber(33);
#ifdef DEBUG_PRINTF
			printf("Received BRIDGE HeartBeat\n");
#endif
		}
		else if(can_msg_hdr.mid == GEO_HEARTBEAT_HDR.mid )
		{
			dbc_decode_GEO_HEARTBEAT(&can_msg_geo_hb,can_msg.data.bytes, &can_msg_hdr);
			LD.setNumber(44);
#ifdef DEBUG_PRINTF
			printf("Received GEO HeartBeat\n");
#endif
		}
	}

	/* Handle MIA 's for controller heartbeat */
	miastatus = dbc_handle_mia_SENSOR_HEARTBEAT(&can_msg_sensor_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("Sensor HeartBeat MIA\n");
#endif
		LOG_INFO("sensor heartBeat MIA Occurred\n");
	}
	miastatus = dbc_handle_mia_MOTORIO_HEARTBEAT(&can_msg_motorio_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("Motorio HeartBeat MIA\n");
#endif
		LOG_INFO("motorio heartBeat MIA Occurred\n");
	}
	miastatus = dbc_handle_mia_BRIDGE_HEARTBEAT(&can_msg_bridge_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("bridge HeartBeat MIA\n");
#endif
		LOG_INFO("bridge heartBeat MIA Occurred\n");
	}
	miastatus = dbc_handle_mia_GEO_HEARTBEAT(&can_msg_geo_hb, 1000);
	if(miastatus == true)
	{
#ifdef DEBUG_PRINTF
		printf("geo HeartBeat MIA\n");
#endif
		LOG_INFO("geo heartBeat MIA Occurred\n");
	}
#endif
	return status;

}
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
status_t send_reset(void)
{
	status_t status = true;

	RESET_t reset_data = {0};
	dbc_encode_and_send_RESET(&reset_data);
#ifdef DEBUG_PRINTF
	printf("Send Reset to All Controllers\n");
#endif
	return status;
}
/* is_bus_off :check if bus is in off state
 *
 *  return bool
 */
void is_bus_off(void)
{

	if(CAN_is_bus_off(can1))
	{
#ifdef DEBUG_PRINTF
		printf("Bus is OFf\n");
#endif
		CAN_reset_bus(can1);
	}
}
