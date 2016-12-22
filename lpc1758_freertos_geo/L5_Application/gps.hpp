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

/*********************************************************************//**
 * Macro defines for Macro defines for UARTn Transmit Holding Register
 **********************************************************************/
#define UART_THR_MASKBIT   	((uint8_t)0xFF) 		/*!< UART Transmit Holding mask bit (8 bits) */

#define UART_IIR_INTSTAT_PEND	((uint32_t)(1<<0))	/*!<Interrupt Status - Active low */
#define UART_IIR_INTID_RLS		((uint32_t)(3<<1)) 	/*!<Interrupt identification: Receive line status*/
#define UART_IIR_INTID_RDA		((uint32_t)(2<<1)) 	/*!<Interrupt identification: Receive data available*/
#define UART_IIR_INTID_CTI		((uint32_t)(6<<1)) 	/*!<Interrupt identification: Character time-out indicator*/
#define UART_IIR_INTID_THRE		((uint32_t)(1<<1)) 	/*!<Interrupt identification: THRE interrupt*/
#define UART1_IIR_INTID_MODEM	((uint32_t)(0<<1)) 	/*!<Interrupt identification: Modem interrupt*/
#define UART_IIR_INTID_MASK		((uint32_t)(7<<1))	/*!<Interrupt identification: Interrupt ID mask */
#define UART_IIR_FIFO_EN		((uint32_t)(3<<6)) 	/*!<These bits are equivalent to UnFCR[0] */
#define UART_IIR_ABEO_INT		((uint32_t)(1<<8)) 	/*!< End of auto-baud interrupt */
#define UART_IIR_ABTO_INT		((uint32_t)(1<<9)) 	/*!< Auto-baud time-out interrupt */
#define UART_IIR_BITMASK		((uint32_t)(0x3CF))	/*!< UART interrupt identification register bit mask */

/*********************************************************************//**
 * Macro defines for Macro defines for UARTn Receiver Buffer Register
 **********************************************************************/
#define UART_RBR_MASKBIT   	((uint8_t)0xFF) 		/*!< UART Received Buffer mask bit (8 bits) */

#define UART_BLOCKING_TIMEOUT 0xffffffff
/*********************************************************************//**
 * Macro defines for Macro defines for UART line status register
 **********************************************************************/
#define UART_LSR_RDR		((uint8_t)(1<<0)) 	/*!<Line status register: Receive data ready*/
#define UART_LSR_OE			((uint8_t)(1<<1)) 	/*!<Line status register: Overrun error*/
#define UART_LSR_PE			((uint8_t)(1<<2)) 	/*!<Line status register: Parity error*/
#define UART_LSR_FE			((uint8_t)(1<<3)) 	/*!<Line status register: Framing error*/
#define UART_LSR_BI			((uint8_t)(1<<4)) 	/*!<Line status register: Break interrupt*/
#define UART_LSR_THRE		((uint8_t)(1<<5)) 	/*!<Line status register: Transmit holding register empty*/
#define UART_LSR_TEMT		((uint8_t)(1<<6)) 	/*!<Line status register: Transmitter empty*/
#define UART_LSR_RXFE		((uint8_t)(1<<7)) 	/*!<Error in RX FIFO*/
#define UART_LSR_BITMASK	((uint8_t)(0xFF)) 	/*!<UART Line status bit mask */


class geoTask
{
    public:
		geoTask();
        bool init(void);
        //bool init();
        bool readGpsData();
        void sendGpsData();
        void calculateBearing();
        bool parseGpsData(char *buffer);
        void calculateDistance();
        void sendCompassData();
        void setChkPointData(float chkLatitude,float chkLongitude);

    private:
		Uart2 &gpsUart;
		static const int rx_q = 512;
		static const int tx_q = 512;
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

