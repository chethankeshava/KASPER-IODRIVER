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
		float curLatitude =0.0;
		float curLongitude =0.0;
		float chkPointLatitude =0.0;
		float chkPointLongitude =0.0;
		float bearing=0.0;
		float heading=0.0;
		uint16_t distance=0;
		uint8_t speed=0;
		//compass compassObject;
};


#endif /* L5_APPLICATION_GPS_HPP_ */

