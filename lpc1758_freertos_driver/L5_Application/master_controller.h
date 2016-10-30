

//#include <../_can_dbc/generated_can.h>

#define status_t bool
/* Use FullCan */
//#define MASTER_FULLCAN
/* Enable periodic function calls for master controller */
#define MASTER
#define DEBUG_PRINTF


/* Check if can bus is in bus-off state */
void is_bus_off(void);
/* Initialise can for master controller */
status_t init_can_master(void);

status_t check_controller_powerdup(void);

status_t read_controller_heartbeat(void);

status_t send_bridge_ack(void);

status_t send_reset(void);

