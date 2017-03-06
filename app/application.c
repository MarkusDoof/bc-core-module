//BASE
#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;


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
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    
    //Pokud podržím button tak roluji na gateway = base, pokud jen stisknu tak vyvolám akci dle těla podmínky
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);
        
        static uint16_t event_count = 0;
        bc_radio_pub_push_button(&event_count);
        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}

