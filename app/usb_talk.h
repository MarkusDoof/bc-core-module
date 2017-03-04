#ifndef _USB_TALK_H
#define _USB_TALK_H

#include <bc_common.h>
#include <bc_led.h>

typedef struct
{
    char tx_buffer[1024];
    char rx_buffer[1024];
    size_t rx_length;
    bool rx_error;

    uint8_t pixels[150 * 4];
    bool light_is_on;

} usb_talk_t;

extern usb_talk_t usb_talk;

void usb_talk_init();
void usb_talk_publish_push_button(const char *prefix, uint16_t *event_count);
void usb_talk_publish_simple_push_button(const char *prefix);
void usb_talk_received_data(uint8_t *buffer, size_t length);
void usb_talk_publish_input_change(const char *prefix, uint16_t *event_count);


#endif /* _USB_TALK_H */
