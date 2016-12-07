/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 * 			FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 * 			@see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "gps.hpp"
#include "compass.hpp"
#include "adc0.h"
#include "utilities.h"
#include "io.hpp"
#include "printf_lib.h"
#include <stdio.h>

char ack=0;
//uint8_t angleXMsb = 0;
//uint8_t angleXLsb = 0;
//uint8_t angleYMsb = 0;
//uint8_t angleYLsb = 0;
//uint8_t angleZMsb = 0;
//uint8_t angleZLsb = 0;
//int16_t X = 0;
//int16_t Y = 0;
//int16_t Z = 0;

char lcdStringBuffer[30]={0};

#if 0
class lsm303_sensor : public scheduler_task
{
	public:
		lsm303_sensor (uint8_t priority):scheduler_task("process", 2048, priority)
	{

	}

	bool init(void)
	{
		LSM.init();
		return true;
	}

	bool run(void *p)
	{
		while((LSM.getStatus() & 0x01 ) != 0x01);

		angleXMsb = LSM.getXAngleMsb();
		angleXLsb = LSM.getXAngleLsb();
		angleYMsb = LSM.getYAngleMsb();
		angleYLsb = LSM.getYAngleLsb();
		angleZMsb = LSM.getZAngleMsb();
		angleZLsb = LSM.getZAngleLsb();

		//X = LSM.getX();
		//Y = LSM.getY();
		//Z = LSM.getZ();
		//temperatureMsb = LSM.getTemperatureMsb();
		//temperatureLsb = LSM.getTemperatureLsb();

		//printf("X is %x\n",X);
		//printf("Y is %x\n",Y);
		//printf("Z is %x\n",Z);
		printf("Angle X MSB is %d\n",angleXMsb);
		printf("Angle X LSB is %d\n",angleXLsb);
		printf("Angle Y MSB is %d\n",angleYMsb);
		printf("Angle Y LSB is %d\n",angleYLsb);
		printf("Angle Z MSB is %d\n",angleZMsb);
		printf("Angle Z LSB is %d\n",angleZLsb);

		//printf("Temperature MSB is %d\n",temperatureMsb);
		//printf("Temperature LSB is %d\n",temperatureLsb);

		vTaskDelay(20000);

		return true;
	}

};
#endif

/**
 * The main() creates tasks or "threads".  See the documentation of scheduler_task class at scheduler_task.hpp
 * for details.  There is a very simple example towards the beginning of this class's declaration.
 *
 * @warning SPI #1 bus usage notes (interfaced to SD & Flash):
 *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
 *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
 *
 * @warning SPI #0 usage notes (Nordic wireless)
 *      - This bus is more tricky to use because if FreeRTOS is not running, the RIT interrupt may use the bus.
 *      - If FreeRTOS is running, then wireless task may use it.
 *        In either case, you should avoid using this bus or interfacing to external components because
 *        there is no semaphore configured for this bus and it should be used exclusively by nordic wireless.
 */
int main(void)
{
    /**
     * A few basic tasks for this bare-bone system :
     *      1.  Terminal task provides gateway to interact with the board through UART terminal.
     *      2.  Remote task allows you to use remote control to interact with the board.
     *      3.  Wireless task responsible to receive, retry, and handle mesh network.
     *
     * Disable remote task if you are not using it.  Also, it needs SYS_CFG_ENABLE_TLM
     * such that it can save remote control codes to non-volatile memory.  IR remote
     * control codes can be learned by typing the "learn" terminal command.
     */

    //scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    //scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));


	#if 1
	const bool run_1Khz = false;
	scheduler_add_task(new periodicSchedulerTask(run_1Khz));
	#endif


	//scheduler_start();

    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */
    #if 0
    const bool run_1Khz = false;
    scheduler_add_task(new periodicSchedulerTask(run_1Khz));
    #endif

    /* The task for the IR receiver to "learn" IR codes */
    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));

    /* Your tasks should probably used PRIORITY_MEDIUM or PRIORITY_LOW because you want the terminal
     * task to always be responsive so you can poke around in case something goes wrong.
     */

    /**
     * This is a the board demonstration task that can be used to test the board.
     * This also shows you how to send a wireless packets to other boards.
     */
    #if 0
        scheduler_add_task(new example_io_demo());
    #endif

    /**
     * Change "#if 0" to "#if 1" to enable examples.
     * Try these examples one at a time.
     */
    #if 0
        scheduler_add_task(new example_task());
        scheduler_add_task(new example_alarm());
        scheduler_add_task(new example_logger_qset());
        scheduler_add_task(new example_nv_vars());
    #endif

    /**
	 * Try the rx / tx tasks together to see how they queue data to each other.
	 */
    #if 0
        scheduler_add_task(new queue_tx());
        scheduler_add_task(new queue_rx());
    #endif

    /**
     * Another example of shared handles and producer/consumer using a queue.
     * In this example, producer will produce as fast as the consumer can consume.
     */
    #if 0
        scheduler_add_task(new producer());
        scheduler_add_task(new consumer());
    #endif

    /**
     * If you have RN-XV on your board, you can connect to Wifi using this task.
     * This does two things for us:
     *   1.  The task allows us to perform HTTP web requests (@see wifiTask)
     *   2.  Terminal task can accept commands from TCP/IP through Wifly module.
     *
     * To add terminal command channel, add this at terminal.cpp :: taskEntry() function:
     * @code
     *     // Assuming Wifly is on Uart3
     *     addCommandChannel(Uart3::getInstance(), false);
     * @endcode
     */
    #if 0
        Uart3 &u3 = Uart3::getInstance();
        u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
        scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
    #endif

    scheduler_start(); ///< This shouldn't return
    return -1;
}
