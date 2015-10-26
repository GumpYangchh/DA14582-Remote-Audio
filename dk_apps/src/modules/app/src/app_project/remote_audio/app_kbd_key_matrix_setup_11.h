/**
 ****************************************************************************************
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

#ifndef _APP_KBD_KEY_MATRIX_SETUP_11_H_
        #define _APP_KBD_KEY_MATRIX_SETUP_11_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup HID
 *
 * @brief HID (Keyboard) Application GPIO reservations for the matrix of setup #11.
 *
 * @{
 ****************************************************************************************
 */

        #undef HAS_I2C_EEPROM_STORAGE

        #if (HAS_MULTI_BOND)
                #error "This keyboard setup does not support EEPROM!"
        #endif

        // Switch required by the compiler
        #undef INIT_EEPROM_PINS
        #define INIT_EEPROM_PINS	(0)

        // Pin definition (irrelevant to whether LED pins are used or not since the HW setup is fixed!)
        #define KBD_GREEN_LED_PORT	GPIO_PORT_0
        #define KBD_GREEN_LED_PIN	GPIO_PIN_5

        #define KBD_RED_LED_PORT	GPIO_PORT_0
        #define KBD_RED_LED_PIN		GPIO_PIN_4

        #undef INIT_LED_PINS
        #define INIT_LED_PINS		(1)
enum led_state {
        LED_STATE_ON    = true,         // LEDs are active high
        LED_STATE_OFF   = false
};

__INLINE void declare_led_pins()
{
        RESERVE_GPIO(GREEN_LED,         GPIO_PORT_0, GPIO_PIN_5, PID_GPIO);
        RESERVE_GPIO(RED_LED,           GPIO_PORT_0, GPIO_PIN_4, PID_GPIO);
}

/**
 * \brief Reserve GPIO pins for keyboard usage
 */
__INLINE void declare_keyboard_gpios()
{
        RESERVE_GPIO(INPUT_COL_0,       GPIO_PORT_1, GPIO_PIN_0, PID_GPIO);
        RESERVE_GPIO(INPUT_COL_1,       GPIO_PORT_1, GPIO_PIN_1, PID_GPIO);
        RESERVE_GPIO(INPUT_COL_2,       GPIO_PORT_0, GPIO_PIN_6, PID_GPIO);
        RESERVE_GPIO(INPUT_COL_3,       GPIO_PORT_0, GPIO_PIN_0, PID_GPIO);
        RESERVE_GPIO(INPUT_COL_4,       GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);
        RESERVE_GPIO(OUTPUT_ROW_0,      GPIO_PORT_0, GPIO_PIN_3, PID_GPIO);
        declare_led_pins();
}

/// @} APP

#endif //_APP_KBD_KEY_MATRIX_SETUP_11_H_
