/**
 ****************************************************************************************
 *
 * @file app_kbd_config.h
 *
 * @brief Keyboard (HID) Application configuration header file.
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

#ifndef APP_KBD_CONFIG_H_
#define APP_KBD_CONFIG_H_


/****************************************************************************************
 * Continuous key scanning. The chip won't go to sleep in this case.                    *
 ****************************************************************************************/
#undef SCAN_ALWAYS_ACTIVE_ON


/****************************************************************************************
 * Delayed wakeup (requires pressing a key for X ms to start)                           *
 ****************************************************************************************/
#undef DELAYED_WAKEUP_ON


/****************************************************************************************
 * Use BOOT MODE                                                                        *
 ****************************************************************************************/
#undef HOGPD_BOOT_PROTO_ON


/****************************************************************************************
 * Include BATT in HID                                                                  *
 ****************************************************************************************/
#undef BATT_EXTERNAL_REPORT_ON


/****************************************************************************************
 * Use different Fast and Partial scan periods                                          *
 ****************************************************************************************/
#undef ALTERNATIVE_SCAN_TIMES_ON



/****************************************************************************************
 * Use a key combination to put the device permanently in extended sleep                *
 * (i.e. 'Fn'+'Space') for consumption measurement purposes.                            *
 ****************************************************************************************/
#undef KEYBOARD_MEASURE_EXT_SLP_ON


/****************************************************************************************
 * Use Connection status LEDs                                                           *
 ****************************************************************************************/
#undef KEYBOARD_LEDS_ON


/****************************************************************************************
 * Set RemoteWakeup mode ON                                                             *
 ****************************************************************************************/
#undef REMOTE_WAKEUP_ON


/****************************************************************************************
 * Enable reporting of any key events happened while disconnected                        *
 ****************************************************************************************/
#undef REPORT_HISTORY_ON


/****************************************************************************************
 * Enable multi-key combinations                                                        *
 ****************************************************************************************/
// Defined below per design #define MULTI_KEY_COMBINATIONS_ON
// Defined below per design #define MULTI_KEY_NUM_OF_KEYS       2       // maximum 8


/****************************************************************************************
 * FORCE_CONNECT_TO_HOST_ON: Enable force-connect to specific host                      *
 * FORCE_CONNECT_NUM_OF_HOSTS must be defined                                           *
 ****************************************************************************************/
// Defined below per design #define FORCE_CONNECT_TO_HOST_ON
// Defined below per design #define FORCE_CONNECT_NUM_OF_HOSTS  3


/****************************************************************************************
 * Choose keyboard layout                                                               *
 ****************************************************************************************/
// Defined below per design #define MATRIX_SETUP  (12)


#if (DA14580_RCU)
#define MULTI_KEY_COMBINATIONS_ON
#define MULTI_KEY_NUM_OF_KEYS       2       // maximum 8
#define FORCE_CONNECT_TO_HOST_ON
#define FORCE_CONNECT_NUM_OF_HOSTS  3
#define MATRIX_SETUP                (12)

#elif (DA14582_RCU)
#define MULTI_KEY_COMBINATIONS_ON
#define MULTI_KEY_NUM_OF_KEYS       2       // maximum 8
#define FORCE_CONNECT_TO_HOST_ON
#define FORCE_CONNECT_NUM_OF_HOSTS  3
#define MATRIX_SETUP				(16)

#endif


/****************************************************************************************
 * Key scanning and Debouncing parameters                                                                *
 ****************************************************************************************/
 
// HW debouncing time for key press
#define DEBOUNCE_TIME_PRESS                     (0)         // in msec
#define DEBOUNCE_TIME_RELEASE                   (0)         // in msec
#define DEBOUNCE_TIME_DELAYED_PRESS             (20)        // in msec
#define DEBOUNCE_TIME_DELAYED_RELEASE           (30)        // in msec

// SW debouncing times
#define ROW_SCAN_TIME                           (150)       // in usec

#ifdef ALTERNATIVE_SCAN_TIMES
#define FULL_SCAN_IN_MS                         (2)
#else
#define FULL_SCAN_IN_MS                         (3)
#endif
#define FULL_SCAN_TIME                          (FULL_SCAN_IN_MS * 1000)

#ifdef ALTERNATIVE_SCAN_TIMES
# define PARTIAL_SCAN_IN_MS                     (1)
#else
# define PARTIAL_SCAN_IN_MS                     (FULL_SCAN_IN_MS)
#endif
#define PARTIAL_SCAN_TIME                       (PARTIAL_SCAN_IN_MS * 1000)


// In general, debounce counters cannot be applied accurately. The reason is that 
// debouncing is done in SW, based on SysTick interrupt events that are used to 
// trigger the execution of the FSM.
// Thus, in the final application it is suggested to modify the definitions of the two
// counters (below) so that the real debouncing time is applied accurately in all
// cases. First, key presses need to be examined. It should be checked whether the
// debouncing period is correct both after wake-up (when the RC16 is used which is 
// approx. 14.5MHz) and when a 2nd key is pressed (assuming that in this case the BLE 
// core woke up to send the HID report of the 1st key press, which means that the 
// system runs with XTAL16).
// Release debouncing period must also be enforced. In all cases, though, the XTAL16 clk
// is used and there are no special cases.

#define DEBOUNCE_COUNTER_P_IN_MS                (12)
#define DEBOUNCE_COUNTER_R_IN_MS                (24)

#define KEYCODE_BUFFER_SIZE                     (32)	// if set to more than 255, change the type of the rd & wr pointers from 8- to 16-bit

#define DEBOUNCE_BUFFER_SIZE                    (16)

/****************************************************************************************
 * Timeouts                                                                             *
 ****************************************************************************************/

// Time to hold down a key for the system to wake-up (DELAYED_WAKEUP_ON must be set)
#define KBD_DELAY_TIMEOUT                       (0x7D0)     // 2 s

#endif // APP_KBD_CONFIG_H_
