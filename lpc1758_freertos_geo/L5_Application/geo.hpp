/*
 * geo.hpp
 *
 *  Created on: Nov 27, 2016
 *      Author: ankit
 */

#ifndef L5_APPLICATION_GEO_HPP_
#define L5_APPLICATION_GEO_HPP_


//#include "gps.hpp"

#define 	GEO_CAN_BUS						can1


typedef struct {
	float curLatitude;			///< Current Latitude of the vehicle
	float curLongitude;			///< Current Longitude of the vehicle
	float chkPointLatitude;		///< Next way pint Latitude
	float chkPointLongitude;	///< Next way pint Longitude
	float bearing;  			///< Current vehicle bearing in degrees
	uint8_t speed;             	///< Current speed of the vehicle
	uint16_t heading;          	///< Current vehicle heading in degrees
	float distance;				///< Current distance to next way point in meters
}geodata;


void geoCalculateDistance(geodata *geoData);
void calculateBearing(geodata *geoData);
bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8]);
void geoSendGpsData(geodata *geoData);
void geoSendCompassData(geodata *geoData);
void geoSendHeartBeat();

#endif /* L5_APPLICATION_GEO_HPP_ */
