/*
 * bluetooth.cpp
 *
 *  Created on: Oct 20, 2016
 *      Author: Bharat Khanna
 */

#include"bluetooth.hpp"
#include<uart2.hpp>
#include<string.h>
#include "printf_lib.h"
SemaphoreHandle_t Bluetooth_Lat_Lon_Semaphore = NULL;

char Bluetooth_Buffer[96];
char stored_Bluetooth_data[96];
Bluetooth_Received *Bluetooth_Rec;
SemaphoreHandle_t BluetoothSemaphore = NULL;

bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8])
{
	can_msg_t can_msg = { 0 };
	can_msg.msg_id                = mid;
	can_msg.frame_fields.data_len = dlc;
	memcpy(can_msg.data.bytes, bytes, dlc);

	return CAN_tx(can1, &can_msg, 0);
}

void canBusErrorCallBackRx(uint32_t ibits)
{
	const uint32_t errbit = (ibits >> 16) & 0x1F;
	const char * rxtx = ((ibits >> 21) & 1) ? "RX" : "TX";
	const uint32_t errc = (ibits >> 22) & 3;

	u0_dbg_put("\n\n ***** CAN BUS ENTERED ERROR STATE!\n");
	u0_dbg_printf("ERRC = %#x, ERRBIT = %#x while %s\n", errc, errbit, rxtx);
}

bool uart_putchar(char character)
{
	Uart2& Bluetooth_uart_2 = Uart2::getInstance();
	Bluetooth_uart_2.init(115200);
	Bluetooth_uart_2.putChar(character);
	while(! (LPC_UART2->LSR & (1 << 6)));
	return true;
}

Bluetooth_Enable::Bluetooth_Enable(uint8_t priority) :
       scheduler_task("Bluetooth_Enable", 2000, priority),Bluetooth_uart_2(Uart2::getInstance())
{
	Bluetooth_uart_2.init(Baud_Rate_Bluetooth_Uart2,Rx_Q_Size,Tx_Q_Size);
}
bool Bluetooth_Enable::init(void)
{
	   return true;
}


bool Bluetooth_Enable::run(void *p)
{
	Bluetooth_uart_2.gets(Bluetooth_Buffer, sizeof(Bluetooth_Buffer), portMAX_DELAY);
	xSemaphoreGiveFromISR(Bluetooth_Lat_Lon_Semaphore,NULL);
	strcpy(stored_Bluetooth_data,Bluetooth_Buffer);
	u0_dbg_printf("I have crossed the gets function %s\n",stored_Bluetooth_data);
	return true;
}
