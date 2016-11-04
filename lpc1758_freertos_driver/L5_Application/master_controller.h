#ifndef MASTER_CONTROLLER_H_
#define MASTER_CONTROLLER_H_


#include <../_can_dbc/generated_can.h>

typedef bool status_t;
#define MASTER
#define DEBUG_PRINTF


/* Check if can bus is in bus-off state */
void is_bus_off(void);

/* Initialize can for master controller */
status_t init_can_master(void);

/* Check for periodic heartbeat from each controller */
status_t read_controller_heartbeat(void);

/* Send acknowledgement to the bridge indicating that master is ready for check-points */
status_t send_bridge_ack(void);


#endif
