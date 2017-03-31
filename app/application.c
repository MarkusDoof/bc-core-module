//REMOTE
#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Relay instance
bc_module_relay_t relay_primary;
bc_module_relay_t relay_secondary;

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
    //Nastav rele na vypnuto
    bc_module_relay_state_t primary_relay_state = bc_module_relay_get_state(&relay_primary);
    if (primary_relay_state == BC_MODULE_RELAY_STATE_UNKNOWN) {
        bc_module_relay_set_state(&relay_primary, false);
    }
    
    //Inicialize secondary relay
    bc_module_secondary_relay_init(&relay_secondary);
    //Nastav rele na vypnuto
    bc_module_relay_state_t secondary_relay_state = bc_module_relay_get_state(&relay_secondary);
    if (secondary_relay_state == BC_MODULE_RELAY_STATE_UNKNOWN) {
        bc_module_relay_set_state(&relay_secondary, false);
    }
    
    //Inicializace power modulu
    
    relays_state = 0;
    
}



void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    
    
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);
        
        if (relays_state == 0) {
            bc_module_relay_set_state(&relay_primary, true);
            bc_module_relay_set_state(&relay_secondary, false);
        } else if (relays_state == 1) {
            bc_module_relay_set_state(&relay_primary, false);
            bc_module_relay_set_state(&relay_secondary, true);
        } else {
            bc_module_relay_set_state(&relay_primary, false);
            bc_module_relay_set_state(&relay_secondary, false);
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
    
}

void bc_radio_on_irrigation_primary_switch_on()
{
    bc_led_set_mode(&led, BC_LED_MODE_ON);
    bc_module_relay_set_state(&relay_primary, true);
    bc_module_relay_set_state(&relay_secondary, false);
}

void bc_radio_on_irrigation_primary_switch_off()
{
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    bc_module_relay_set_state(&relay_primary, false);
    bc_module_relay_set_state(&relay_secondary, false);
}

void bc_radio_on_irrigation_secondary_switch_on()
{
    bc_led_set_mode(&led, BC_LED_MODE_ON);
    bc_module_relay_set_state(&relay_primary, false);
    bc_module_relay_set_state(&relay_secondary, true);
}

void bc_radio_on_irrigation_secondary_switch_off()
{
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    bc_module_relay_set_state(&relay_primary, false);
    bc_module_relay_set_state(&relay_secondary, false);
}
