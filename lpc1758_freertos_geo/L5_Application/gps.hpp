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

#define 	GPS_CAN_BUS						can1

void geoSendGpsData();
void geoSendHeartBeat();

class gpsTask
{
    public:
		gpsTask();
        bool init(void);
        bool readGpsData();

    private:
		Uart2 &gpsUart;
		static const int rx_q = 100;
		static const int tx_q = 100;
};
#endif /* L5_APPLICATION_GPS_HPP_ */

