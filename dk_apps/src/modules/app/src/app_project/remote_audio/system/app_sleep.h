/*
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 */

#include "rwip_config.h"

#include "l2cm.h"

#include "arch.h"
#include "app.h"
#include "app_task.h"
#include "app_api.h"

#include "gpio.h"
#include "co_buf.h"
#include "ke_event.h"
#include "ke.h"

#include "rwip.h"

#include "app_kbd_scan_fsm.h"
#include "app_kbd.h"

#if (USE_CONNECTION_FSM)
#include "app_con_fsm.h"
#endif

#include "app_stream.h"
#include "l2cm.h"

#if (HAS_BMI055)
#include "app_motion_sensor.h"

extern bool user_motion_keys_pressed;
extern char state_bmi_pressed, state_bmi_released;
#endif
/*
 ******************************** Locals ***********************************
 */

static void delayed_start_proc(void)
{
    if (HAS_DELAYED_WAKEUP)
    {
        if ( ( (trigger_kbd_delayed_start_st == PRESSED_TRIGGER) || (trigger_kbd_delayed_start_st == RELEASED_TRIGGER) ) 
            && !ble_is_woken_up)
        {
            ASSERT_WARNING(app_con_fsm_get_state() == IDLE_ST);
            
            if (trigger_kbd_delayed_start_st == PRESSED_TRIGGER)
                trigger_kbd_delayed_start_st = PRESSED_TRIGGER_SYNCED;
            else
                trigger_kbd_delayed_start_st = RELEASED_TRIGGER_SYNCED;
            
            // Put a message to the queue for the BLE to handle. The message
            // is handled by the Kbd app and starts or stops the monitor timer.
            struct hid_msg* msg = KE_MSG_ALLOC(APP_HID_MSG, TASK_APP, TASK_APP,
                    hid_msg);

            msg->trigger = trigger_kbd_delayed_start_st;

            ke_msg_send(msg);
            
            // If BLE is sleeping, wake it up!
            app_ble_force_wakeup();
            app_ble_ext_wakeup_off();
            
            ble_is_woken_up = true;
        }
    }
}

#if (HAS_AUDIO)
extern char stop_when_buffer_empty;
void app_audio439_stop (void);
#endif

/*
 ********************************* Hooks ************************************
 */

/**
 ****************************************************************************************
 * @brief HOOK #1 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[in] None
 *
 * @return  true, to force calling of schedule()
 *         false, else
 ****************************************************************************************
 */
static inline bool app_asynch_trm(void)
{
	bool ret = false;

	do {
        // Synchronize with the BLE here! The time window of requesting a packet trm at the upcoming
        // anchor point is from the CSCNT event until the FINEGTIM event. If you pass the FINEGTIM
        // event then the packet will be sent at the next anchor point!
        //
        // Note that this synchronization is only possible in sleep modes!
        
        ble_is_woken_up = false;
        
        
        if (app_con_fsm_get_state() == CONNECTED_ST) {
            app_kbd_prepare_keyreports();
        }
        if (sync_key_press_evt == true) {
            app_con_fsm_reset_inactivity_timeout();
            sync_key_press_evt = false;
        }

        app_con_fsm_process_async_tasks();
        
 #if (HAS_AUDIO)
        if (user_audio_sw_pressed == true) {
            if (app_con_fsm_get_state() == CONNECTED_ST) {
                //app_stream_send_enable(1);
                app_stream_send_enable_not(1);
                stop_when_buffer_empty=0;
            }
            user_audio_sw_pressed = false;
        }

        if (user_audio_sw_released == true) {
            app_audio439_stop();
            stop_when_buffer_empty=1;

            user_audio_sw_released = false;
        }
#endif
        
#if (HAS_BMI055)  
        if (user_motion_keys_pressed == true) {
            if (app_con_fsm_get_state() == CONNECTED_ST)  {
                app_motion_state_machine ( 1 );
            } else {
                user_motion_keys_pressed=false;
                app_motion_state_machine ( 0 );
            }
        } else {
            app_motion_state_machine ( 0 );
        }
#endif       

        if ( !ke_event_get(KE_EVENT_KE_MESSAGE) ) {
            // Since pkt reqs can be silently discarded if no Tx bufs are available, check first!
            if (kbd_trm_list && app_kbd_check_conn_status() && l2cm_get_nb_buffer_available()) {
                
                if (app_kbd_send_key_report()) {
                    // One HID report is removed from the trm list. Check if other HID reports are to be
                    // prepared because of unread data in the keycode_buffer now that the free list is not NULL.
                    app_kbd_prepare_keyreports();
                    
                    ret = true;
                    break;
                }
            }
        }               
	} while(0);

	return ret;
}

/**
 ****************************************************************************************
 * @brief HOOK #2 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[in] None
 *
 * @return  true, to force calling of schedule()
 *         false, else
 ****************************************************************************************
 */
static inline bool app_asynch_proc(void)
{
	bool ret = false;

	do {
		fsm_scan_update();

#if (HAS_BMI055)
        if (((user_motion_keys_pressed == true) && 
            (app_con_fsm_get_state() == CONNECTED_ST) &&
            (state_bmi_pressed==0) && 
            (GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 0)) ||
            (user_motion_keys_pressed == false ) && 
            (state_bmi_released==0) &&
            (GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 0)) {
            // If BLE is sleeping, wake it up!
            ret = app_ble_force_wakeup();
        }
#endif
        if  ( app_kbd_buffer_has_data() && 
            (app_con_fsm_get_state() == CONNECTED_ST) &&
            (GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 0)) {
            // If BLE is sleeping, wake it up!
            ret = app_ble_force_wakeup();
        }
        
        if (HAS_DELAYED_WAKEUP) {
            delayed_start_proc();
        }
        
        
 	} while(0);

	return ret;
}

/**
 ****************************************************************************************
 * @brief HOOK #3 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[in] None
 *
 * @return    void
 ****************************************************************************************
 */
static inline void app_asynch_sleep_proc(void)
{
    if (wkup_hit) { // make sure we do not miss any wake up interrupts!
        fsm_scan_update();
    }

    if (HAS_DELAYED_WAKEUP) {
        delayed_start_proc();
    }
    
    if (HAS_KEYBOARD_MEASURE_EXT_SLP) {
        if ( (user_extended_sleep) && (current_scan_state == KEY_SCAN_IDLE)) {
            // see if we can enter into sleep
            if (!ke_sleep_check()) {
                return;
            }
            if (GetBits16(SYS_STAT_REG, RAD_IS_DOWN)) {
                return; // already sleeping, will wake up eventually. wait until this happens.
            }
            if (rwip_prevent_sleep_get() != 0) {
                return;
            }
            rwip_rf.sleep();
            while ( !(GetWord32(BLE_CNTL2_REG) & RADIO_PWRDN_ALLOW) ) {};
            SetBits16(CLK_RADIO_REG, BLE_ENABLE, 0);
            SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1);        // turn off radio
            
            SCB->SCR |= 1<<2;
            SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM
            SetBits16(SYS_CTRL_REG, OTP_COPY, 0);           // disable OTP copy
            SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);       // activate PAD latches
            SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain
            
            SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 0);

            WFI();
        }
    } 
}


/**
 ****************************************************************************************
 * @brief HOOK #4 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[in] None
 *
 * @return    void
 ****************************************************************************************
 */
static inline void app_sleep_prepare_proc(sleep_mode_t *sleep_mode)
{
    if ( (current_scan_state == KEY_STATUS_UPD) || (current_scan_state == KEY_SCANNING) )  {
        *sleep_mode = mode_idle;                // block power-off
    }
}


/**
 ****************************************************************************************
 * @brief HOOK #5 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[out] sleep_mode (to block power off after going to sleep once, if needed)
 *
 * @return    void
 ****************************************************************************************
 */
static inline void app_sleep_entry_proc(sleep_mode_t *sleep_mode)
{
    if ( *sleep_mode == mode_idle && !HAS_PRINTF)  {
        /*
        * Use a lower clock to preserve power (i.e. 2MHz)
        */
        syscntl_use_lowest_amba_clocks();
    }
}


/**
 ****************************************************************************************
 * @brief HOOK #6 - See documentation (UM-B-006 : User Manual - Sleep mode configuration)
 *
 * @param[in] sleep_mode 
 *
 * @return    void
 ****************************************************************************************
 */
static inline void app_sleep_exit_proc(sleep_mode_t sleep_mode)
{
    /*
     * Restore clock
     */
    syscntl_use_highest_amba_clocks();
}
