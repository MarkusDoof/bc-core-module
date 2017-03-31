#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <bcl.h>

typedef struct bc_module_relay_t bc_module_relay_t;
typedef struct bc_irrigation_valve bc_irrigation_valve;

typedef enum
{
    VALVE_OPEN,
    VALVE_CLOSED
    
} bc_irrigation_valve_state_t;

struct bc_irrigation_valve
{
    bc_irrigation_valve_state_t _valve_state;
};


void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void radio_event_handler(bc_radio_event_t event, void *event_param);
void reset_switches();
void bc_irrigation_valve_init(bc_irrigation_valve *valve);
void estimate_valve_state();

#endif // _APPLICATION_H
