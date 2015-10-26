/**
 ****************************************************************************************
 *
 * @file app_kbd_proj.c
 *
 * @brief HID Keyboard hooks.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
 
#include "rwip_config.h"

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "app_api.h"

#include "app_kbd_proj.h"
#include "app_kbd_debug.h"
#include "app_kbd.h"

#include "app_dis.h"
#include "app_batt.h"

#if (HAS_BMI055)
#include "app_motion_sensor.h"
#endif

#if (HAS_AUDIO)
#include "app_audio439.h"
#endif

#if (HAS_AUDIO || HAS_BMI055)
#include "app_stream.h"
#endif

#include "app_utils.h"

    #if (BLE_SPOTA_RECEIVER)    
        #include "app_con_fsm_task.h"
    #endif

#if DEVELOPMENT_DEBUG
#warning "Put manually APP_DISS_PNP_PRODUCT_VERSION in ADV data. Change \x00\x01 (0x0100) to a proper value!"
#endif


extern struct gap_cfg_table_struct gap_timeout_table;

/*
 * LOCAL VARIABLES
 ****************************************************************************************
 */

const struct notification_info_ notification_info[] = {
    {0, ATT_CHAR_PROTOCOL_MODE,      ATTR_TYPE,  1, 1},
    {1, ATT_CHAR_BATTERY_LEVEL,      CCC_TYPE,   1, 2},
    {2, ATT_CHAR_BOOT_KB_IN_REPORT,  CCC_TYPE,   1, 2},
    {3, ATT_CHAR_REPORT,             CCC_TYPE,   2, 2},
    {5, ATT_CHAR_HID_CTNL_PT,        ATTR_TYPE,  1, 1},
    {6, ATT_CHAR_SERVICE_CHANGED,    CCC_TYPE,   1, 2}
}; 

extern int min_vendor_hndl;
extern int max_vendor_hndl;

/**
 ****************************************************************************************
 * @brief     This callback is called when the connection state has changed to connected,
 *            connection in progress, disconnected, off, passcode entry started.  
 *
 * @param[in] type Type of connection FSM callback
 *
 * @return    void
 ****************************************************************************************
 */
void app_kbd_con_fsm_callback(enum con_fsm_state_update_callback_type type)
{
    switch (type) {
    case PREPARE_DEEPSLEEP:
        if (HAS_DELAYED_WAKEUP) {
            app_kbd_enable_delayed_scanning(true);
        }
        break;
    case REINITIALIZE:
        app_kbd_flush_buffer();
        app_kbd_flush_reports();
        break;
    case INDICATE_CONNECTION_IN_PROGRESS:
        if (HAS_KEYBOARD_LEDS) {
            leds_set_connection_in_progress();
        }
        break;
    case INDICATE_DISCONNECTED:
        if (HAS_KEYBOARD_LEDS) {
            leds_set_disconnected();
        }
        break;
    case INDICATE_OFF:
        if (HAS_KEYBOARD_LEDS) {
            leds_init();
        }
        break;
    case INDICATE_CONNECTED:
        if (HAS_KEYBOARD_LEDS) {
            leds_set_connection_established();
        }
        break;
    case START_PASSCODE:
        app_kbd_start_passcode();               // Set to 'Passcode' mode until the connection is established        
        break;
    default:
        break;
    }
}

/**
 ****************************************************************************************
 * @brief      This callback is called when bonding data are read from the non-volatile   
 *             memory so that the service database is updated
 *
 * @param[in]  pos Position of the characteristic in the notification_info
 * @param[in]  attr_num Attribute number of the characteristic
 * @param[in]  value Value of the attribute
 *
 * @return     void
 ****************************************************************************************
 */
void app_kbd_attr_update_callback(int pos, int attr_num, int value)
{
    switch (pos) {
    case CHAR_PROTOCOL_MODE_POS:   
        // Update Protocol mode
        hogpd_env.proto_mode[0] = value ? HOGP_REPORT_PROTOCOL_MODE
                                        : HOGP_BOOT_PROTOCOL_MODE;
        break;
    case CHAR_BATTERY_LEVEL_POS:     
        // Update Battery Level
        if (value) {
            bass_env.features[0] |= BASS_FLAG_NTF_CFG_BIT;
        }
        break;
    case CHAR_BOOT_KB_IN_REPORT_POS:
        // Update Boot report
        if (value) {
            hogpd_env.features[0].svc_features |= HOGPD_REPORT_NTF_CFG_MASK;
        }
        break;
    case CHAR_REPORT_POS:
        switch (attr_num) {
        case 0:
            if (value) {
                hogpd_env.features[0].report_char_cfg[0] |= HOGPD_REPORT_NTF_CFG_MASK;
            }
            break;
        case 1:
            if (value) {
                hogpd_env.features[0].report_char_cfg[2] |= HOGPD_REPORT_NTF_CFG_MASK;
            }
            break;
        }
        break;
        case CHAR_HID_CTNL_PT_POS:      
        case CHAR_SERVICE_CHANGED_POS:  
        default:
            break;
    }
}
    
/**
 ****************************************************************************************
 * @brief       Initialize the application   
 *              Initialize state, GPIOs. Set advertising data. Set sleep mode.
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_init_func(void)
{    
	app_keyboard_init();        // Initialize Keyboard env
    app_dis_init();             // Initialize Device Information Service
    
    app_con_fsm_init();
        
#if (HAS_AUDIO)    
    app_audio439_init();
    app_audio439_configure_ima_mode(IMA_MODE_64KBPS_4_16KHZ);  // used only if CFG_AUDIO439_ADAPTIVE_RATE is used. Otherwise this function does nothing.
#endif
    
#if (HAS_BMI055)
    app_delay(8000);    // wait for BMI to power up
    app_motion_init_state_machine();
#endif
    
#if (BLE_SPOTA_RECEIVER)    
	app_spotar_init(app_spotar_callback);
#endif
        
    app_set_extended_sleep();
//    app_disable_sleep();
}


/**
 ****************************************************************************************
 * @brief       Configures Bluetooth settings
 *
 * @param[in]   task_id - id of the kernel task calling this function
 * @param[in]   cmd - parameters to pass to the stack
 *
 * @return  void
 ****************************************************************************************
 */
void app_configuration_func(ke_task_id_t const task_id, struct gapm_set_dev_config_cmd *cmd)
{
    // set device configuration
    cmd->operation = GAPM_SET_DEV_CONFIG;
    // Device Role
    cmd->role = GAP_PERIPHERAL_SLV;
    // Device Appearance
    cmd->appearance = 384;          // 384: Remote Control
    // Device Appearance write permission requirements for peer device
    cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
    // Device Name write permission requirements for peer device
    cmd->name_write_perm = GAPM_WRITE_DISABLE;
    // Slave preferred Minimum of connection interval
    cmd->con_intv_min = 6;         // 7.5ms (6*1.25ms)
    // Slave preferred Maximum of connection interval
    cmd->con_intv_max = 16;        // 20ms (16*1.25ms)
    // Slave preferred Connection latency
    cmd->con_latency  = 31;
    // Slave preferred Link supervision timeout
    cmd->superv_to    = 200;

    // Device IRK
    memcpy(cmd->irk.key, "0123456789012345", KEY_LEN);
    
    cmd->flags = app_con_fsm_set_privacy();
    
    if (HAS_KEYBOARD_LEDS) {
        // This is a good place to initialize the LEDs
        leds_init();
    }
}


/**
 ****************************************************************************************
 * @brief       Handles what needs to be done after the completion of the configuration phase
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_set_dev_config_complete_func(void)
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init()) {
        app_con_fsm_state_update(NO_EVENT);
//				app_con_fsm_start_advertising();
    }
}


/**
 ****************************************************************************************
 * @brief   Handles what needs to be done after the completion of the initialization
 *          of all modules
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_db_init_complete_func(void)
{
#if (HAS_AUDIO)
    min_vendor_hndl=hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 5] + 1;
    max_vendor_hndl=hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 7] + 1;
#endif
    app_con_fsm_state_update(NO_EVENT);
//    app_con_fsm_start_advertising(); 
    //get the handles of the vector specific data
   
}

/**
 ****************************************************************************************
 * @brief   Handles what needs to be done after Undirected advertising finishes
 *
 * @param[in]   status
 *
 * @return  void
 ****************************************************************************************
 */
void app_adv_undirect_complete(uint8_t status)
{
    app_con_fsm_adv_undirect_complete(status);
}


/**
 ****************************************************************************************
 * @brief   Handles what needs to be done after Directed advertising finishes
 *
 * @param[in]   status
 *
 * @return  void
 ****************************************************************************************
 */
void app_adv_direct_complete(uint8_t status)
{
    app_con_fsm_adv_direct_complete(status);
}

/**
 ****************************************************************************************
 * @brief   Initialize security
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
#define HID_KEY_SIZE (16)
void app_sec_init_func(void)
{
}
#undef HID_KEY_SIZE


/**
 ****************************************************************************************
 * @brief   Prepares and sends the reply to the GAPC_PAIRING_REQ msg
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    app_con_fsm_send_pairing_rsp_func(param);
}


/**
 ****************************************************************************************
 * @brief   N/A for this application
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
}


/**
 ****************************************************************************************
 * @brief   Starts keyboard scanning and sets the keyboard to passcode entry mode
 *
 * @param[in] src_id  id of the kernel task which caused this to happen
 * @param[in] dest_id id of the kernel task which received the message
 *
 * @return  void
 ****************************************************************************************
 */
void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
#if (USE_CONNECTION_FSM)
    app_con_fsm_mitm_passcode_entry_func(src_id, dest_id);
#endif
}


/**
 ****************************************************************************************
 * @brief   N/A for this application
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
}


/**
 ****************************************************************************************
 * @brief   N/A for this application
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
}


/**
 ****************************************************************************************
 * @brief   LTK exchange - Handles the exchange of the LTK with the Host
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    struct app_sec_env_tag *sec_env;
    
#if (USE_CONNECTION_FSM)
    sec_env=&bond_info.env;
#else
    sec_env=&app_sec_env;
#endif
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    
    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = sec_env->key_size;
    cfm->data.ltk.ediv = sec_env->ediv;

    memcpy(&(cfm->data.ltk.randnb), &(sec_env->rand_nb), RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(sec_env->ltk), KEY_LEN);

    ke_msg_send(cfm);
}


/**
 ****************************************************************************************
 * @brief   Pairing completed - Handles the completion of Pairing
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_paired_func(void)
{
    app_con_fsm_paired_func();
    app_param_update_start();
}


/**
 ****************************************************************************************
 * @brief   Check if encryption will be accepted (LL_ENC_REQ received but not when in PAIRING mode)
 *          for this link
 *
 * @param[in] param  The message from the host
 *
 * @return  true, if everything is ok
 *          false, if the request is rejected
 ****************************************************************************************
 */
bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
        // Handle Pairing/Bonding copletion event
    return app_con_fsm_validate_encrypt_req_func(param);
}


/**
 ****************************************************************************************
 * @brief   Received and accepted LL_ENC_REQ 
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_sec_encrypt_complete_func(void)
{
}


/**
 ****************************************************************************************
 * @brief   Received LL_START_ENC_RSP : Enryption is completed -> Start normal keyboard
 *          operation. After completion of the encryption of the link, it sets the keyboard
 *          to normal report mode
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_sec_encrypt_ind_func(void)
{
    app_con_fsm_sec_encrypt_ind_func();  
    app_kbd_start_reporting();          // start sending notifications    
}


/**
 ****************************************************************************************
 * @brief   Configures keyboard application when connection is established.
 *
 * @param[in] param  parameters passed from the stack
 *
 * @return  void
 ****************************************************************************************
 */
void app_connection_func(struct gapc_connection_req_ind const *param)
{
    if (!app_con_fsm_connection_func(param)) {
        return;
    }  
        
// connection confirmation and advertising is handled by app_con_fsm_connection_func        

// Enable DIS for this conhdl
    if (con_fsm_params.has_mitm) {
        app_dis_enable_prf_sec(param->conhdl, PERM(SVC, AUTH));     
    } else {
        if (con_fsm_params.has_nv_rom) {
            app_dis_enable_prf_sec(param->conhdl, PERM(SVC, UNAUTH));
        } else {
            app_dis_enable_prf_sec(param->conhdl, PERM(SVC, ENABLE));
        }            
    }
    
// Enable BATT for this conhdl (with fake levels)
    if (con_fsm_params.has_mitm) {
        app_batt_enable_prf_sec(PERM(SVC, AUTH), 99, HAS_KEYBOARD_LEDS, GPIO_PORT_0, GPIO_PIN_0);
    } else {
        if (con_fsm_params.has_nv_rom) {
            app_batt_enable_prf_sec(PERM(SVC, UNAUTH), 99, HAS_KEYBOARD_LEDS, GPIO_PORT_0, GPIO_PIN_0);
        } else {
            app_batt_enable_prf_sec(PERM(SVC, ENABLE), 99, HAS_KEYBOARD_LEDS, GPIO_PORT_0, GPIO_PIN_0);
        }
    }

    app_batt_poll_start(BATTERY_LEVEL_POLLING_PERIOD/10);  // Start polling
        
    app_keyboard_enable();                  // Enable HOGPD for this conhdl
#if (BLE_APP_STREAM)
    app_stream_enable(); 
#endif
            
// Retrieve the connection info from the parameters (MUST always be done!)
    app_env.conhdl = param->conhdl;
    app_env.peer_addr_type = param->peer_addr_type;
    app_env.peer_addr = param->peer_addr;
                
#if (BLE_SPOTA_RECEIVER)
    app_spotar_enable();
#endif //BLE_SPOTA_RECEIVER    
}

/**
 ****************************************************************************************
 * @brief   Configures the application when the connection is terminated.
 *
 * @param[in] task_id   id of the kernel task calling this function
 * @param[in] param     parameters passed from the stack
 *
 * @return  void
 ****************************************************************************************
 */
void app_disconnect_func(ke_task_id_t const task_id, struct gapc_disconnect_ind const *param)
{
    uint8_t state = ke_state_get(task_id);

    if ((state == APP_CONNECTED) || (state == APP_PARAM_UPD) || (state == APP_SECURITY)) {
#if (HAS_AUDIO)
        app_audio439_stop();
#endif
        app_con_fsm_disconnect_func();
        app_kbd_stop_reporting();
        app_batt_poll_stop();    // stop battery polling
    }
    // There is an extreme case where this message is received twice. This happens when 
    // both the device and the host decide to terminate the connection at the same time.
    // In this case, when the device sends the LL_TERMINATE_IND, it also gets the same
    // message from the Host. This is not an erroneous situation provided that the 
    // device has already cleared its state.
    //    else
    //        ASSERT_ERROR(0);
}

/**
 ****************************************************************************************
 * @brief   Requests update of connection params
 *          After connection and, optionally, pairing is completed, this function 
 *                is called to (optionally) modify the connection parameters.
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_param_update_func(void)
{
    if (!con_fsm_params.has_mitm) {
        app_kbd_start_reporting();          // start sending notifications   
        // Clear all buffers if reporting "old" keys is not wanted
        if (!HAS_REPORT_HISTORY) {
            app_kbd_flush_buffer();
            app_kbd_flush_reports();
        }
        app_kbd_prepare_keyreports();       // prepare any (if required)
    }
    
    app_con_fsm_param_update_func();
}


/**
 ****************************************************************************************
 * @brief   Handles the rejection of connection params from the host
 *
 * @param[in] The status of gapc_cmp_evt_handler message
 *
 * @return  void
 ****************************************************************************************
 */
void app_update_params_rejected_func(uint8_t status)
{
    app_con_fsm_update_params_rejected_func(status);
}


/**
 ****************************************************************************************
 * @brief   Handles the completion of connection params update
 *          Actions taken after the reception of the host's reply to a connection update 
 *          params we sent.
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_update_params_complete_func(void)
{
    app_con_fsm_update_params_complete_func();
}

/**
 ****************************************************************************************
 * @brief   N/A for this application
 *
 * @param[in] cmd   message to GAPM
 *
 * @return  void
 ****************************************************************************************
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
}


/**
 ****************************************************************************************
 * @brief   Initializes the HID server DB 
 *
 * @param[in] cmd   message to GAPM
 *
 * @return  true, if the initialization is done
 *          false, if the initialization fails
 ****************************************************************************************
 */
bool app_db_init_func(void)
{
    // Indicate if more services need to be added in the database
    bool end_db_create = false;
    
    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP) {
        switch (app_env.next_prf_init) {
        case (APP_DIS_TASK):
            app_dis_create_db_send();
            break;
        case (APP_BASS_TASK):
            app_batt_create_db();
            break;
        case (APP_HOGPD_TASK):
            // Add HID Service in the DB
            app_hid_create_db();
            break;
#if (BLE_SPOTA_RECEIVER)
        case (APP_SPOTAR_TASK):
            // Add spotar Service in the DB
            app_spotar_create_db();
            break;
#endif //BLE_SPOTA_RECEIVER
        default:
            ASSERT_ERR(0);
            break;
        }

        // Select following service to add
        app_env.next_prf_init++;
    } else {
        end_db_create = true;
    }

    return end_db_create;
}


int app_hid_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
	dbg_puts(DBG_FSM_LVL, "HID timer exp\r\n");
    	
    // Timer elapsed!
    if (HAS_DELAYED_WAKEUP) {
        if (app_con_fsm_get_state() == IDLE_ST) {
            ASSERT_ERROR(monitor_kbd_delayed_start_st == MONITOR_RELEASE);
            
            GLOBAL_INT_DISABLE();
            // Stop WKUP
            SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, 0); //No more interrupts of this kind
            GLOBAL_INT_RESTORE();
            // Any WKUP int would hit here
            
            trigger_kbd_delayed_start_st = NO_TRIGGER;
            
            wkup_hit = true;
        }
    }
    
	return (KE_MSG_CONSUMED);
}
\

/**
 ****************************************************************************************
 * @brief Handle the Stream On - Off 
 * 
 * Called when STREAMOFF is received, or button pressed/released.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
#if (HAS_AUDIO || HAS_BMI055)
void streamdatad_streamonoff_hogpd (void)
{
    uint16_t len = 0;
	uint8_t* streamdatad_en = NULL;

    uint16 handle = hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 3] + 1;
    attmdb_att_get_value(handle, &len, &(streamdatad_en));

	int on = (streamdatad_en[0]>0);
    int tp = (int)(streamdatad_en[1])&0x00FF;    /* If second byte is zero, consider this the Enable signal. */
    if ((len == 1) || (tp == 0)) {
        if (on) {
            app_stream_start();
            #if (HAS_AUDIO)
            app_audio439_start();
            #endif
                } else {
            app_stream_stop();
            #if (HAS_AUDIO)
            app_audio439_stop();
            #endif
        }
    } else {
#if (HAS_AUDIO)        
        /* Forward the enable packet to the app_audio439 as configuration setting */
    app_audio439_config(streamdatad_en);
#endif        
    }
}

int app_hogpd_report_ind_handler(ke_msg_id_t const msgid,
                           struct hogpd_report_info *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (param->report_nb == 3) {
        streamdatad_streamonoff_hogpd();
    }
    return (KE_MSG_CONSUMED);
}
#endif


int app_hid_msg_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    struct hid_msg* msg = (struct hid_msg*)param;
    
    if (HAS_DELAYED_WAKEUP) {
        dbg_puts(DBG_FSM_LVL, "HID_MSG handler\r\n");
        
        if (msg->trigger == PRESSED_TRIGGER_SYNCED) {
            ASSERT_WARNING(app_con_fsm_get_state() == IDLE_ST);
            
            trigger_kbd_delayed_start_st = NO_TRIGGER;
            app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_DELAY_TIMEOUT / 10));
            
            // Set WKUPCT to monitor key release
            app_kbd_enable_delayed_scanning(false);
        }

        if (msg->trigger == RELEASED_TRIGGER_SYNCED) {
            ASSERT_WARNING(app_con_fsm_get_state() == IDLE_ST);
            
            ke_timer_clear(APP_HID_TIMER, TASK_APP);
            trigger_kbd_delayed_start_st = NO_TRIGGER;
            
            // Put BLE in permanent sleep mode
            app_ble_ext_wakeup_on();
            
            // Set WKUPCT to monitor key press
            app_kbd_enable_delayed_scanning(true);
        }   
    }
    return (KE_MSG_CONSUMED);
}

#endif  //BLE_APP_PRESENT
/// @} APP
