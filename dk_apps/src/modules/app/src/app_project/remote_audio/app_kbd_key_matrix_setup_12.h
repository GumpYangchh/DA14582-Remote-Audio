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

#ifndef _APP_KBD_KEY_MATRIX_SETUP_12_H_
    #define _APP_KBD_KEY_MATRIX_SETUP_12_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup HID
 *
 * @brief HID (Keyboard) Application GPIO reservations for the matrix of setup #12.
 *
 * @{
 ****************************************************************************************
 */

/**
 * \brief Reserve GPIO pins for keyboard usage
 */
__INLINE void declare_keyboard_gpios()
{
    RESERVE_GPIO(INPUT_COL_0,       GPIO_PORT_2, GPIO_PIN_7, PID_GPIO);
    RESERVE_GPIO(INPUT_COL_1,       GPIO_PORT_2, GPIO_PIN_8, PID_GPIO);
    RESERVE_GPIO(INPUT_COL_2,       GPIO_PORT_2, GPIO_PIN_9, PID_GPIO);
    RESERVE_GPIO(INPUT_COL_3,       GPIO_PORT_3, GPIO_PIN_0, PID_GPIO);
    RESERVE_GPIO(INPUT_COL_4,       GPIO_PORT_3, GPIO_PIN_1, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_0,      GPIO_PORT_3, GPIO_PIN_2, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_1,      GPIO_PORT_3, GPIO_PIN_3, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_2,      GPIO_PORT_3, GPIO_PIN_4, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_3,      GPIO_PORT_3, GPIO_PIN_5, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_4,      GPIO_PORT_3, GPIO_PIN_6, PID_GPIO);
    RESERVE_GPIO(OUTPUT_ROW_5,      GPIO_PORT_3, GPIO_PIN_7, PID_GPIO);
}

/// @} APP

#endif //_APP_KBD_KEY_MATRIX_SETUP_12_H_
