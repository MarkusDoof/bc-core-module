#include <usb_talk.h>
#include <bc_scheduler.h>
#include <bc_usb_cdc.h>
#include <jsmn.h>
#include <base64.h>
#include <bc_led.h>
// TODO
// #include "bc_module_power.h"

#define USB_TALK_TOKEN_ARRAY         0
#define USB_TALK_TOKEN_TOPIC         1
#define USB_TALK_TOKEN_PAYLOAD       2
#define USB_TALK_TOKEN_PAYLOAD_KEY   3
#define USB_TALK_TOKEN_PAYLOAD_VALUE 4

#define USB_TALK_UINT_VALUE_NULL -1
#define USB_TALK_UINT_VALUE_INVALID -2

usb_talk_t usb_talk;
char debug_message[256];

// LED instance
bc_led_t led;

static void _usb_talk_task(void *param);
static void usb_talk_process_character(char character);
static void usb_talk_process_message(char *message, size_t length);
static bool usb_talk_on_message_light_set(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_is_string_token_equal(const char *buffer, jsmntok_t *token, const char *value);
static void usb_talk_send_string(const char *buffer);

void usb_talk_init()
{
    memset(&usb_talk, 0, sizeof(usb_talk));
    
    bc_usb_cdc_init();
    
    bc_scheduler_register(_usb_talk_task, NULL, 0);
}

void usb_talk_publish_push_button(const char *prefix, uint16_t *event_count)
{
    if (event_count != NULL)
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%spush-button/-\", {\"event-count\": %" PRIu16 "}]\n", prefix, *event_count);
    }
    else
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"%spush-button/-\", {\"event-count\": null}]\n",
                 prefix);
    }
    
    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_simple_push_button(const char *topic)
{
    
    snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"%s\", {\"\"}]\n", topic);
    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_input_change(const char *prefix, uint16_t *value)
{
    
    snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
             "[\"%sinput-change/-\", {\"value\": %" PRIu16 "}]\n", prefix, *value);
    
    
    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

static void _usb_talk_task(void *param)
{
    (void) param;
    
    while (true)
    {
        static uint8_t buffer[16];
        
        size_t length = bc_usb_cdc_read(buffer, sizeof(buffer));
        
        if (length == 0)
        {
            break;
        }
        
        for (size_t i = 0; i < length; i++)
        {
            usb_talk_process_character((char) buffer[i]);
        }
    }
    
    // TODO
    bc_scheduler_plan_current_now();
}

static void usb_talk_process_character(char character)
{
    
    //kontrola přetečení bufferu
    if (usb_talk.rx_length == sizeof(usb_talk.rx_buffer))
    {
        usb_talk.rx_error = true;
    }
    else
    {
        //Pokud není chybový stav zapiš do bufferu znak
        if (!usb_talk.rx_error)
        {
            usb_talk.rx_buffer[usb_talk.rx_length++] = character;
        }
    }
    
    //Jestli je konec řádku
    if (character == '\n')
    {
        //Jestli není chybový stav a buffer má nenulovou délku
        if (!usb_talk.rx_error && usb_talk.rx_length > 0)
        {
            usb_talk_process_message(usb_talk.rx_buffer, usb_talk.rx_length);
        }
        
        usb_talk.rx_length = 0;
        usb_talk.rx_error = false;
        //memset(usb_talk.rx_buffer, 0, sizeof(usb_talk.rx_buffer));
        return;
    }
}

static void usb_talk_process_message(char *message, size_t length)
{
    
    static jsmn_parser parser;
    static jsmntok_t tokens[16];
    
    jsmn_init(&parser);
    
    int token_count = jsmn_parse(&parser, (const char *) message, length, tokens, 16);
    
    //    memset(debug_message, 0, sizeof(debug_message));
    //    snprintf(debug_message, sizeof(debug_message), "message:%s, token count:%d \n", message, token_count);
    //    usb_talk_send_string(debug_message);
    
    
    if (token_count < 3)
    {
        return;
    }
    
    if (tokens[USB_TALK_TOKEN_ARRAY].type != JSMN_ARRAY || tokens[USB_TALK_TOKEN_ARRAY].size != 2)
    {
        return;
    }
    
    if (tokens[USB_TALK_TOKEN_TOPIC].type != JSMN_STRING || tokens[USB_TALK_TOKEN_TOPIC].size != 0)
    {
        return;
    }
    
    //MQTT switch je třeba správně nakonfigurovat včetně {}
    if (tokens[USB_TALK_TOKEN_PAYLOAD].type != JSMN_OBJECT)
    {
        return;
    }
    
    if (usb_talk_on_message_light_set((const char *) message, token_count, tokens))
    {
        return;
    }
    
}

static bool usb_talk_on_message_light_set(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 5)
    {
        return false;
    }
    
    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 1)
    {
        return false;
    }
    //Pozor gateway oseká nodes takže v podmínce musí  být už jen toto
    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/irrigation/-/set"))
    {
        return false;
    }
    
    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_KEY], "state"))
    {
        return false;
    }
    
    if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "on"))
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);
    }
    else if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "off"))
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
    return true;
}

static bool usb_talk_is_string_token_equal(const char *buffer, jsmntok_t *token, const char *value)
{
    size_t token_length;
    
    token_length = (size_t) (token->end - token->start);
    
    if (strlen(value) != token_length)
    {
        return false;
    }
    
    if (strncmp(value, &buffer[token->start], token_length) != 0)
    {
        return false;
    }
    
    return true;
}

// TODO I think there is possibly buffer overflow and also will not work for uint32 since we are storing to int and then comparing to < 0

static void usb_talk_send_string(const char *buffer)
{
    bc_usb_cdc_write(buffer, strlen(buffer));
}

