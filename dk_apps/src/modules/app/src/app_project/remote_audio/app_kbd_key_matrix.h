/**
 ****************************************************************************************
 *
 * @file app_kbd_key_matrix.h
 *
 * @brief HID Keyboard key scan matrix definitions.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_KBD_KEY_MATRIX_H_
#define APP_KBD_KEY_MATRIX_H_

#include "gpio.h"

#define INIT_LED_PINS       0

#if MATRIX_SETUP == 0
        // FREE - Left for customer applications
#include "app_kbd_key_matrix_setup_0.h"
#elif MATRIX_SETUP == 1
        // Rev.3 DK with QFN48 (keyboard #1)
#include "app_kbd_key_matrix_setup_1.h"
#elif MATRIX_SETUP == 2
        // Rev.3 DK with QFN48 (keyboard #2)
#include "app_kbd_key_matrix_setup_2.h"
#elif MATRIX_SETUP == 3
        // Rev.3 DK with QFN48 (keyboard #3)
#include "app_kbd_key_matrix_setup_3.h"
#elif MATRIX_SETUP == 4
        // Rev.3 DK with QFN48 (keyboard #1 with EEPROM)
#include "app_kbd_key_matrix_setup_4.h"
#elif MATRIX_SETUP == 5
        // Rev.3 DK with QFN48 (keyboard #1 for Apple products)
#include "app_kbd_key_matrix_setup_5.h"
#elif MATRIX_SETUP == 6
        // Rev.3 DK with QFN48 (keyboard #1 for Apple products)
#include "app_kbd_key_matrix_setup_6.h"
#elif MATRIX_SETUP == 7
        // Rev.3 DK with QFN48 (keyboard #2 with EEPROM)
#include "app_kbd_key_matrix_setup_7.h"
#elif MATRIX_SETUP == 8
        // Reference Design (#1)
#include "app_kbd_key_matrix_setup_8.h"
#elif MATRIX_SETUP == 9
        // RC 12-keys with QFN48 (based on keyboard #3)
#include "app_kbd_key_matrix_setup_9.h"
#elif MATRIX_SETUP == 10
        // DK 2-keys with QFN48
#include "app_kbd_key_matrix_setup_10.h"
#elif MATRIX_SETUP == 11
        // 5-keys with QFN48
#include "app_kbd_key_matrix_setup_11.h"
#elif MATRIX_SETUP == 12
        // DA14580 RCU Reference design
#include "app_kbd_key_matrix_setup_12.h"
#elif MATRIX_SETUP == 16
        // DA14582 RCU Reference design
#include "app_kbd_key_matrix_setup_16.h"
#endif // MATRIX_SETUP

        // dummy values. If LEDS are needed, INIT_LED_PINS must be 1
        // and pins must be defined in app_kbd_key_matrix_setup_XX.h 
#if !(INIT_LED_PINS)
#define KBD_GREEN_LED_PORT  GPIO_PORT_1
#define KBD_GREEN_LED_PIN   GPIO_PIN_8

#define KBD_RED_LED_PORT    GPIO_PORT_1
#define KBD_RED_LED_PIN     GPIO_PIN_9
    
enum led_state {
        LED_STATE_ON    = false,            // LEDs are active low
        LED_STATE_OFF   = true
};
#endif

#endif // APP_KBD_KEY_MATRIX_H_
