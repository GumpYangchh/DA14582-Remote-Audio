/**
 ****************************************************************************************
 *
 * @file app_kbd_scan_fsm.c
 *
 * @brief Keyboard scanning FSM implementation.
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

#include "app_kbd.h"
#include "app_kbd_scan_fsm.h"

enum key_scan_states current_scan_state __attribute__((section("retention_mem_area0"), zero_init));
static int scanning_substate            __attribute__((section("retention_mem_area0"), zero_init));

//bool main_fsm_changed = true;   /* use if the key scanning needs to be disabled */

void fsm_scan_update(void)
{
    switch(current_scan_state) {
    case KEY_SCAN_INACTIVE:
        if (DEVELOPMENT_DEBUG && (systick_hit || wkup_hit)) {
            ASSERT_ERROR(0);
        }

        if (HAS_DELAYED_WAKEUP) {
            app_kbd_enable_delayed_scanning(true);
        } else {
            app_kbd_enable_scanning();
        }
        current_scan_state = KEY_SCAN_IDLE;     // Transition from KEY_SCAN_INACTIVE -> KEY_SCAN_IDLE
        break;
    case KEY_SCAN_IDLE:
        if (DEVELOPMENT_DEBUG && systick_hit) {
            ASSERT_ERROR(0);
        }

        if (wkup_hit) {
        //	dbg_puts(DBG_SCAN_LVL, "KEY_SCAN_IDLE -> Wakeup! -> KEY_SCANNING\r\n");
            scanning_substate = 0;
            GLOBAL_INT_DISABLE();
            current_scan_state = KEY_SCANNING;              // Transition from KEY_SCAN_IDLE -> KEY_SCANNING
            app_kbd_start_scanning();
            wkup_hit = false;                                   
            GLOBAL_INT_RESTORE();
        }
        break;
    case KEY_STATUS_UPD:
        if (DEVELOPMENT_DEBUG && wkup_hit) {
            ASSERT_ERROR(0);
        }

        if (systick_hit) {
            systick_hit = false;
            if (app_kbd_update_status()) {
                scanning_substate = 0;
                current_scan_state = KEY_SCANNING;          // Transition from KEY_STATUS_UPD -> KEY_SCANNING
                // scan once to save time!
                if (app_kbd_scan_matrix(&scanning_substate)) {
                    current_scan_state = KEY_STATUS_UPD;    // Transition from KEY_SCANNING -> KEY_STATUS_UPD
                }
            } else {
                GLOBAL_INT_DISABLE();
                app_kbd_enable_scanning();
                current_scan_state = KEY_SCAN_IDLE;         // Transition from KEY_STATUS_UPD -> KEY_SCAN_IDLE
                GLOBAL_INT_RESTORE();
            }
        }
        break;
    case KEY_SCANNING:
        if (DEVELOPMENT_DEBUG && wkup_hit) {
            ASSERT_ERROR(0);
        }

        if (systick_hit) {
            systick_hit = false;
            if (app_kbd_scan_matrix(&scanning_substate)) {
                current_scan_state = KEY_STATUS_UPD;        // Transition from KEY_SCANNING -> KEY_STATUS_UPD
            } // else the state remains unchanged and next time we will scan the next row
        }
        break;
    default:
            break;
    }
}
