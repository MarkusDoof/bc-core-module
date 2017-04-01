//REMOTE
#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Relay instance
bc_module_relay_t relay_primary;
bc_module_relay_t relay_secondary;

//Valve instance
bc_irrigation_valve valve_1;
bc_irrigation_valve valve_2;


static int relays_state = 0;

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);
    
    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);
    
    // Initialize radio
    bc_radio_init();
    bc_radio_set_event_handler(radio_event_handler, NULL);
    bc_radio_listen();
    
    //Inicialize primary relay
    bc_module_primary_relay_init(&relay_primary);
    
    
    //Inicialize secondary relay
    bc_module_secondary_relay_init(&relay_secondary);
    
    //Inicializace power modulu
    bc_module_power_init();
    
    //Nastav vsechna rele na false neboli do zakladniho stavu
    //reset_switches();
    bc_module_power_relay_set_state(false);
    bc_module_relay_pulse(&relay_primary, false, 1000);
    bc_module_relay_pulse(&relay_secondary, false, 1000);
    
    
    //Inicializace objektu valve
    bc_irrigation_valve_init(&valve_1);
    bc_irrigation_valve_init(&valve_2);
    
    //Nastavení pomocného objektu
    bc_irrigation_valve_set_state(&valve_1, VALVE_CLOSED);
    bc_irrigation_valve_set_state(&valve_2, VALVE_CLOSED);
    
    relays_state = 0;
    
}



void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    
    
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);
        
        if (relays_state == 1)
        {
            bc_radio_on_irrigation_primary_switch_on();
        } else if (relays_state == 2)
        {
            bc_radio_on_irrigation_primary_switch_off();
        } else if (relays_state == 3)
        {
            bc_radio_on_irrigation_secondary_switch_on();
        } else if (relays_state == 4)
        {
            bc_radio_on_irrigation_secondary_switch_off();
        } else
        {
            relays_state = 0;
        }
        relays_state++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enrollment_start();
        
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
    
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;
    
    if (event == BC_RADIO_EVENT_PAIR_SUCCESS)
    {
        bc_radio_enrollment_stop();
        
        bc_led_pulse(&led, 1000);
        
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}

void reset_switches()
{
    //bc_module_relay_set_state(&relay_primary, false);
    //bc_module_relay_set_state(&relay_secondary, false);
    //bc_module_power_relay_set_state(false);
    
}

void bc_irrigation_valve_init(bc_irrigation_valve *self)
{
    // Init instance, set state machine initial state
    memset(self, 0, sizeof(*self));
    self->_valve_state = VALVE_CLOSED;
}

void bc_irrigation_valve_set_state(bc_irrigation_valve *self, bc_irrigation_valve_state_t valve_state)
{
    self->_valve_state = valve_state;
}

void bc_radio_on_irrigation_primary_switch_on()
{
    bc_module_power_relay_set_state(true);
    bc_module_relay_pulse(&relay_primary, true, 1000);
    if (valve_2._valve_state == VALVE_OPEN)
    {
        bc_module_relay_pulse(&relay_secondary, true, 1000);
    } else
    {
        bc_module_relay_set_state(&relay_secondary, false);
    }
    bc_irrigation_valve_set_state(&valve_1, VALVE_OPEN);
}

void bc_radio_on_irrigation_primary_switch_off()
{
    bc_module_power_relay_set_state(false);
    bc_module_relay_pulse(&relay_primary, false, 1000);
    bc_module_relay_pulse(&relay_secondary, false, 1000);
    bc_irrigation_valve_set_state(&valve_1, VALVE_CLOSED);
    bc_irrigation_valve_set_state(&valve_2, VALVE_CLOSED);
}

void bc_radio_on_irrigation_secondary_switch_on()
{
    bc_module_power_relay_set_state(true);
    if (valve_1._valve_state == VALVE_OPEN)
    {
        bc_module_relay_pulse(&relay_primary, true, 1000);
    } else
    {
        bc_module_relay_set_state(&relay_primary, false);
    }
    bc_module_relay_pulse(&relay_secondary, true, 1000);
    bc_irrigation_valve_set_state(&valve_2, VALVE_OPEN);
}

void bc_radio_on_irrigation_secondary_switch_off()
{
    bc_module_power_relay_set_state(false);
    bc_module_relay_pulse(&relay_primary, false, 1000);
    bc_module_relay_pulse(&relay_secondary, false, 1000);
    bc_irrigation_valve_set_state(&valve_1, VALVE_CLOSED);
    bc_irrigation_valve_set_state(&valve_2, VALVE_CLOSED);
}
