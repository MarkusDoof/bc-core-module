#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Relay instance
bc_module_relay_t relay;

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
    
    //Inicialize relay
    bc_module_relay_init(&relay);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    bc_module_relay_state_t relay_state = bc_module_relay_get_state(&relay);
    if (relay_state == BC_MODULE_RELAY_STATE_UNKNOWN) {
        bc_module_relay_set_state(&relay, true);
    }
    
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
    }
}
