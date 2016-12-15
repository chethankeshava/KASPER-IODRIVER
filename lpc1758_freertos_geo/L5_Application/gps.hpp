/*
 * gps.hpp GPS header file
 *
 *  Created on: Oct 17, 2016
 *      Author: ankit
 */

#ifndef L5_APPLICATION_GPS_HPP_
#define L5_APPLICATION_GPS_HPP_

#include "scheduler_task.hpp"
#include "uart2.hpp"
#include "geo.hpp"

#define EARTH_RADIUS_KM 6371

class geoTask
{
    public:
		geoTask();

        bool init(void);
        bool readGpsData();
        void sendGpsData();
        void calculateBearing();
        bool parseGpsData(char *buffer);
        void calculateDistance();
        void sendCompassData();
        void setChkPointData(float chkLatitude,float chkLongitude);

    private:
		Uart2 &gpsUart;
		static const int rx_q = 100;
		static const int tx_q = 100;
		float curLatitude = 37.335187;
		float curLongitude = -121.881071;
		float chkPointLatitude = 37.335187;
		float chkPointLongitude = -121.881071;
		float bearing=0.0;
		float heading=0.0;
		float distance=0.0;
		uint8_t speed=0;
		//compass compassObject;
};


#endif /* L5_APPLICATION_GPS_HPP_ */

