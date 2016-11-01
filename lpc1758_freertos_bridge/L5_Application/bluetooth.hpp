/*
 * bluetooth.hpp
 *
 *  Created on: Oct 20, 2016
 *      Author: Bharat Khanna
 */

#ifndef L5_APPLICATION_BLUETOOTH_HPP_
#define L5_APPLICATION_BLUETOOTH_HPP_

#include "examples/examples.hpp"
#include <stdlib.h>
#include "printf_lib.h"
#include "uart2.hpp"
#include "uart_dev.hpp"
#include "can.h"
#include "periodic_scheduler/periodic_callback.h"


#define 	UART_THR_MASKBIT   		((uint8_t)0xFF) 	/*!< UART Transmit Holding mask bit (8 bits) */
#define 	UART_IIR_INTID_RLS		((uint32_t)(3<<1)) 	/*!<Interrupt identification: Receive line status*/
#define 	UART_IIR_INTID_RDA		((uint32_t)(2<<1)) 	/*!<Interrupt identification: Receive data available*/
#define 	UART_IIR_INTID_CTI		((uint32_t)(6<<1)) 	/*!<Interrupt identification: Character time-out indicator*/
#define 	UART_IIR_INTID_MASK		((uint32_t)(7<<1))	/*!<Interrupt identification: Interrupt ID mask */
#define 	UART_LSR_OE   			((uint8_t)(1<<1))
#define 	UART_LSR_PE   			((uint8_t)(1<<2))
#define 	UART_LSR_FE   			((uint8_t)(1<<3))
#define 	UART_LSR_BI   			((uint8_t)(1<<4))
#define 	UART_LSR_RXFE   		((uint8_t)(1<<7))
#define 	UART_LSR_BITMASK   		((uint8_t)(0xFF))
#define UART_LSR_RDR   				((uint8_t)(1<<0))
#define UART_IIR_INTID_THRE			((uint32_t)(1<<1)) 	/*!<Interrupt identification: THRE interrupt*/

typedef struct sensorData{
	float latitude;				///< Stores Latitude
	float longitude;			///< Stores Longitude
}Bluetooth_Received;

bool uart_putchar(char character);
bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8]);
void canBusErrorCallBackRx(uint32_t ibits);
class Bluetooth_Enable : public scheduler_task
{
	private:
	Uart2 &Bluetooth_uart_2;
	static const int Rx_Q_Size = 128;
	static const int Tx_Q_Size = 128;
	static const int Baud_Rate_Bluetooth_Uart2 = 115200;
    public:
	Bluetooth_Enable(uint8_t priority) ;
           /* scheduler_task("UART_Initialize", 2000, priority),uart2(Uart2::getInstance())
        {
        	//uart2.init(115200,rx_q,tx_q);
        	uart2.init(9600,rx_q,tx_q);
        }*/
        bool init(void);
        bool run(void *p);
};


#endif /* L5_APPLICATION_BLUETOOTH_HPP_ */
