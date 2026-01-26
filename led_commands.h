#ifndef LEDCOMMANDS_H
#define LEDCOMMANDS_H

#include "led.h"
#include "jvs.h"


#define FADE_TIMER_DEFAULT 80

#define FADE_MODE_NONE 0
#define FADE_MODE_DECREASE 1
#define FADE_MODE_INCREASE 2

static uint8_t fade_mode_1 = FADE_MODE_NONE;
static uint8_t fade_timer_1 = FADE_TIMER_DEFAULT;
static int16_t fade_value_1 = 200;
static int16_t fade_modifier_1 = 1;
static uint8_t settings_timeout1;
static bool setting_disable_resp_1 = false;

static bool setting_disable_resp_2 = false;
static uint8_t settings_timeout2;

void handle_led_command(jvs_req_any* req, jvs_resp_any* resp, int led_board);

#endif