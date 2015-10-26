/**
 ****************************************************************************************
 *
 * @file app_con_fsm.c
 *
 * @brief Connection FSM.
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration

#if (USE_CONNECTION_FSM)
#include "app_api.h" 
#include "llc_util.h"
#include "nvds.h"

#include "app_con_fsm.h"
#include "app_con_fsm_debug.h"
#include "app_utils.h"
#include "gpio.h"

uint8_t app_adv_data_length                  __attribute__((section("retention_mem_area0"), zero_init));  // Advertising data length
uint8_t app_adv_data[ADV_DATA_LEN-3]         __attribute__((section("retention_mem_area0"), zero_init));  // Advertising data
uint8_t app_scanrsp_data_length              __attribute__((section("retention_mem_area0"), zero_init));  // Scan response data length
uint8_t app_scanrsp_data[SCAN_RSP_DATA_LEN]  __attribute__((section("retention_mem_area0"), zero_init));  // Scan response data

enum main_fsm_states current_fsm_state       __attribute__((section("retention_mem_area0"), zero_init));
enum adv_states current_adv_state            __attribute__((section("retention_mem_area0"), zero_init));
bool is_bonded                               __attribute__((section("retention_mem_area0"), zero_init));
bool nv_rom_is_read                          __attribute__((section("retention_mem_area0"), zero_init));
int adv_timer_remaining                      __attribute__((section("retention_mem_area0"), zero_init));
int conn_timer_remaining                     __attribute__((section("retention_mem_area0"), zero_init));
bool conn_upd_pending                        __attribute__((section("retention_mem_area0"), zero_init));
                                             
bool user_disconnection_req                  __attribute__((section("retention_mem_area0"), zero_init));
bool sync_passcode_entered_evt               __attribute__((section("retention_mem_area0"), zero_init)); // flag to indicate to the high-level FSM that the Passcode has been entered by the user, synchronously to the BLE
bool reset_bonding_request                   __attribute__((section("retention_mem_area0"), zero_init));

extern struct bonding_info_ bond_array[MAX_BOND_PEER];
extern enum multi_bond_host_rejection multi_bond_enabled;

__INLINE void app_con_fsm_call_callback(enum con_fsm_state_update_callback_type type);

ke_task_id_t mitm_src_id, mitm_dest_id;

bool spota_on/* = false*/;

__attribute__((unused)) static const char state_names[8][24] = { "IDLE", "ADVERTISE", "CONNECTION_IN_PROGRESS", "CONNECTED_PAIRING", "CONNECTED",
                        "DISCONNECTED_IDLE", "DISCONNECTED_INIT", "DIRECTED_ADV" };

__attribute__((unused)) static const char events_names[15][17] = { "NO_EVENT", "KEY_PRESS", "TIMER_EXPIRED", "PAIRING_REQ", "CONN_REQ",
                         "CONN_CMP", "DISCONN", "CONN_UPD_RESP", "PASSKEY_ENTERED", "SWITCH_EVT", "NEW_HOST_EVT", "SPOTAR_START_EVT", "SPOTAR_END_EVT",
                         "ADV_TIMER_EXP", "INACT_TIMER_EXP" };

#if (DEVELOPMENT_DEBUG)
#define FSM_LOG_DEPTH 8
                         
struct fsm_log_ {
    enum main_fsm_events evt;
    enum main_fsm_states state;
    uint16_t time;
} fsm_log[FSM_LOG_DEPTH] __attribute__((section("retention_mem_area0"), zero_init));

int fsm_log_ptr __attribute__((section("retention_mem_area0"), zero_init));
#endif


/**
 ****************************************************************************************
 * @brief   Sets the advertising and the scan response data in the GAP Start ADV command
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
static void set_adv_data(struct gapm_start_advertise_cmd *cmd)
{
    if (app_adv_data_length != 0) {
        memcpy(&cmd->info.host.adv_data[0], app_adv_data, app_adv_data_length);
        cmd->info.host.adv_data_len = app_adv_data_length;
    }
    if (app_scanrsp_data_length != 0) {
        memcpy(&cmd->info.host.scan_rsp_data[0], app_scanrsp_data, app_scanrsp_data_length);
        cmd->info.host.scan_rsp_data_len = app_scanrsp_data_length;
    }
}


/**
 ****************************************************************************************
 * @brief Starts undirected advertising
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
static void start_adv_undirected(void)
{
    // Start Adv timer (if not running)
    ke_msg_send_basic(APP_CON_FSM_START_ADV_MSG, TASK_APP, TASK_APP);
    
    // Allocate a message for GAP
    struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_start_advertise_cmd);

    cmd->op.code = GAPM_ADV_UNDIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->channel_map = APP_ADV_CHMAP;

    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    
    switch(current_adv_state) {
    case SLOW_ADV:
        cmd->intv_min = con_fsm_params.slow_bonded_adv_int_min;
        cmd->intv_max = con_fsm_params.slow_bonded_adv_int_max;
        break;
    case UNBONDED_ADV:
        cmd->intv_min = con_fsm_params.normal_adv_int_min;
        cmd->intv_max = con_fsm_params.normal_adv_int_max;
        break;
    case BONDED_ADV:
        cmd->intv_min = con_fsm_params.fast_bonded_adv_int_min;
        cmd->intv_max = con_fsm_params.fast_bonded_adv_int_max;
        break;
    default:
        break;
    }
    
    // The filter policy should be defined prior to the call to this function.
    // An application may wish to advertise using various filter policies, depending
    // on its state i.e. it may use ADV_ALLOW_SCAN_ANY_CON_WLST when it wants to
    // connect to known hosts only or ADV_ALLOW_SCAN_ANY_CON_ANY when it wants to
    // pair to new hosts.
    // So, the following code is placed here as a reference only.
    if (con_fsm_params.has_white_list) {
        if (white_list_written) {
            cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_WLST_CON_WLST;
        } else {
            cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
        }
    } else if (con_fsm_params.has_virtual_white_list) {
        if (white_list_written) {
            virtual_wlist_policy = ADV_ALLOW_SCAN_ANY_CON_WLST;
        } else {
            virtual_wlist_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
        }
        cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    } else {
        cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    }
    
    /*-----------------------------------------------------------------------------------
     * Set the Advertising Data and the Scan Response Data
     *---------------------------------------------------------------------------------*/
    set_adv_data(cmd);
    
    // Send the message
    ke_msg_send(cmd);

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}


/**
 ****************************************************************************************
 * @brief Starts directed advertising
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
static void start_adv_directed(void)
{
    // Allocate a message for GAP
    struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_start_advertise_cmd);

    cmd->op.code = GAPM_ADV_DIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->channel_map = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT_MIN;
    cmd->intv_max = APP_ADV_INT_MAX;
    cmd->info.direct.addr_type = bond_info.env.peer_addr_type;
    memcpy((void *)cmd->info.direct.addr.addr, bond_info.env.peer_addr.addr, BD_ADDR_LEN);
    
    // Send the message
    ke_msg_send(cmd);

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}


/**
 ****************************************************************************************
 * @brief Sends connection update request
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
static void send_connection_upd_req(void)
{    
    ke_state_t app_state = ke_state_get(TASK_APP);
    
	// Modify Conn Params
	if (app_state == APP_SECURITY || app_state == APP_PARAM_UPD || app_state == APP_CONNECTED) {
		struct gapc_param_update_cmd * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD, TASK_GAPC, TASK_APP, gapc_param_update_cmd);

		// Fill in the parameter structure
        req->operation = GAPC_UPDATE_PARAMS;
#ifndef __DA14581__
		req->params.intv_min = con_fsm_params.preferred_conn_interval_min; // N * 1.25ms
		req->params.intv_max = con_fsm_params.preferred_conn_interval_max; // N * 1.25ms
		req->params.latency  = con_fsm_params.preferred_conn_latency;      // Conn Events skipped
		req->params.time_out = con_fsm_params.preferred_conn_timeout;      // N * 10ms
#else
		req->intv_min   = con_fsm_params.preferred_conn_interval_min;     // N * 1.25ms
		req->intv_max   = con_fsm_params.preferred_conn_interval_max;     // N * 1.25ms
		req->latency    = con_fsm_params.preferred_conn_latency;          // Conn Events skipped
		req->time_out   = con_fsm_params.preferred_conn_timeout;          // N * 10ms
#endif        
		dbg_puts(DBG_FSM_LVL, "Send GAP_PARAM_UPDATE_REQ\r\n");
		ke_msg_send(req);
        
        ke_state_set(TASK_APP, APP_PARAM_UPD);
	}
}


/**
 ****************************************************************************************
 * @brief Wakes-up the BLE
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
static void wake_ble_up(void)
{
    app_ble_force_wakeup();
    app_ble_ext_wakeup_off();
}


#include "ke_env.h"
extern uint32_t ke_time(void);

/**
 ****************************************************************************************
 * @brief Removes a kernel timer from the timers list
 *
 * @param[in]   timer_id
 * @param[in]   task_id
 *
 * @return  the time (10ms intervals) that remained for the timer to elapse
 *          (0 if the timer was not removed)
 ****************************************************************************************
 */
static int ke_timer_remove(ke_msg_id_t const timer_id, ke_task_id_t const task_id)
{
    int ret = 0;
    
    struct ke_timer *timer = (struct ke_timer *) ke_env.queue_timer.first;

    /* scan the timer queue to look for a message element with the same id and destination*/
    while (timer != NULL) {
        if ((timer->id == timer_id) && (timer->task == task_id) ) {
            /* Element has been found */
            break;
        }

        /* Check  next timer */
        timer = timer->next;
    }

    /* If the element has been found */
    if (timer != NULL) {
        ret = timer->time - ke_time();
        if (ret < 0) {
            ret += (BLE_GROSSTARGET_MASK+1);
        }
        ke_timer_clear(timer_id, task_id);
    }
    return ret;
}


/**
 ****************************************************************************************
 * @brief Updates the main FSM with an event
 *
 * @param[in]   evt   The main FSM event
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_state_update(enum main_fsm_events evt)
{
    if ( (KEY_PRESS_EVT == evt) && (current_fsm_state > CONNECTED_ST) ) {    // only for IDLE_ST and CONNECTED_ST
        return;
    }
    
#if (DEVELOPMENT_DEBUG)   
    fsm_log[fsm_log_ptr].state = current_fsm_state;
    fsm_log[fsm_log_ptr].evt = evt;
    fsm_log[fsm_log_ptr].time = ke_time() & BLE_GROSSTARGET_MASK;
    fsm_log_ptr++;
    if (fsm_log_ptr == FSM_LOG_DEPTH) {
        fsm_log_ptr = 0;
    }
#endif    
    
    dbg_printf(DBG_FSM_LVL, "\r\n**> %s\r\n", state_names[current_fsm_state]);
    dbg_printf(DBG_FSM_LVL, "--> %s\r\n", events_names[evt]);
    
    //Common code in all states
    if (SPOTAR_END_EVT == evt) {
        spota_on = false;
        // Re-configure NV memory pins since they've been reset to GPIOs
#if (INIT_STORAGE_PINS)
#ifdef HAS_SPI_FLASH_STORAGE
        GPIO_SetPinFunction( FLASH_SPI_CS_PORT,  FLASH_SPI_CS_PIN,  INPUT_PULLUP,   PID_SPI_EN);
        GPIO_SetPinFunction( FLASH_SPI_CLK_PORT, FLASH_SPI_CLK_PIN, INPUT_PULLDOWN, PID_SPI_CLK);
        GPIO_SetPinFunction( FLASH_SPI_DO_PORT,  FLASH_SPI_DO_PIN,  INPUT_PULLDOWN, PID_SPI_DO);
        GPIO_SetPinFunction( FLASH_SPI_DI_PORT,  FLASH_SPI_DI_PIN,  INPUT_PULLDOWN, PID_SPI_DI);
#endif
        
#ifdef HAS_I2C_EEPROM_STORAGE
        GPIO_SetPinFunction(I2C_SCL_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL);
        GPIO_SetPinFunction(I2C_SDA_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA);
#endif       
#endif
        
        if (current_fsm_state != CONNECTED_ST) {
            return; // extra functionality required only when connected!
        }
    }

    switch(current_fsm_state) {
    case IDLE_ST:
        if (con_fsm_params.has_nv_rom) {
            if (nv_rom_is_read == false) {
                is_bonded = false;
            }
            if ( (is_bonded == false) && (nv_rom_is_read == false) ) {
                app_alt_pair_read_status();
                reset_active_peer_pos();
                is_bonded = app_alt_pair_load_last_used();
                
                if (is_bonded) {
                    // has the last connected host public /*or static random address*/?
                    if (   (ADDR_PUBLIC == bond_info.env.peer_addr_type)
                        /*|| ( (bond_info.env.peer_addr_type == ADDR_RAND) && ((bond_info.env.peer_addr.addr[5] & SMPM_ADDR_TYPE_STATIC) == SMPM_ADDR_TYPE_STATIC) )*/ ) {
                        // do Directed advertising to this host if connection is lost
                    } else {
                        is_bonded = false;      // will be set to true if Privacy is enabled and the Reconnection address is written
                    }
                }
                nv_rom_is_read = true;
            }
        } else {
            bond_info.info = BOND_INFO_DEFAULT_VAL;
            updatedb_from_bonding_info(&bond_info);
            if (con_fsm_params.has_mitm) {
                bond_info.env.auth = GAP_AUTH_REQ_MITM_BOND;
            } else {
                bond_info.env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
            }
        }
            
        
        if (con_fsm_params.has_inactivity_timeout) {
            conn_timer_remaining = con_fsm_params.kbd_inactivity_timeout / 10;     // reset inactivity timer here
        }
        
        switch(evt) {
        case NO_EVENT:
            if (con_fsm_params.is_normally_connectable) {
                if (is_bonded && (bond_info.env.auth & GAP_AUTH_BOND)) {
                    start_adv_directed();
                    current_fsm_state = DIRECTED_ADV_ST;
                } else {
                    current_adv_state = SLOW_ADV;
                    current_fsm_state = ADVERTISE_ST;
                    start_adv_undirected();
                }
                wake_ble_up();
                if (con_fsm_params.has_deepsleep) {
                    app_set_extended_sleep();
                }
            }
            else if (con_fsm_params.has_deepsleep) {
                app_set_deep_sleep();
                app_ble_ext_wakeup_on();
                app_con_fsm_call_callback(PREPARE_DEEPSLEEP);
            } else {
                app_ble_ext_wakeup_on();
                app_con_fsm_call_callback(PREPARE_DEEPSLEEP);
            }
            break;
        case KEY_PRESS_EVT:
            if (is_bonded && (bond_info.env.auth & GAP_AUTH_BOND)) {
                current_fsm_state = DIRECTED_ADV_ST;
                start_adv_directed();
            } else {
                adv_timer_remaining = con_fsm_params.unbonded_discoverable_timeout / 10;
                current_adv_state = UNBONDED_ADV;
                current_fsm_state = ADVERTISE_ST;
                start_adv_undirected();
            }
            
            wake_ble_up();
            if (con_fsm_params.has_deepsleep) {
                app_set_extended_sleep();
            }
            break;
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    case ADVERTISE_ST:
        switch(evt) {
        case ADV_TIMER_EXP_EVT:
            adv_timer_remaining = 0;
            app_adv_stop();
            break;
        case TIMER_EXPIRED_EVT:
            if (con_fsm_params.is_normally_connectable) {
                current_adv_state = SLOW_ADV;
                start_adv_undirected();
            } else {
                app_con_fsm_call_callback(REINITIALIZE);

                // timer is cleared automatically
                if (con_fsm_params.has_deepsleep) {
                    app_set_deep_sleep();
                }
                app_ble_ext_wakeup_on();
                app_con_fsm_call_callback(PREPARE_DEEPSLEEP);
                current_fsm_state = IDLE_ST;
            }
            break;
        case CONN_REQ_EVT:
            if (adv_timer_remaining) {
                adv_timer_remaining = ke_timer_remove(APP_CON_FSM_ADV_TIMER, TASK_APP);
            }
            spota_on = false;
            current_fsm_state = CONNECTION_IN_PROGRESS_ST;
            break;
            
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    case CONNECTION_IN_PROGRESS_ST:
        if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) )  {
            dbg_puts(DBG_FSM_LVL, "  (-) ENC timer\r\n");
            ke_timer_clear(APP_CON_FSM_ENC_TIMER, TASK_APP);        // Timer expire results in connection drop and is handled below.
        }
        
        switch(evt) {
        case PAIRING_REQ_EVT:
            if (con_fsm_params.has_passcode_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (+) passcode timer\r\n");
                app_timer_set(APP_CON_FSM_TIMER, TASK_APP, (con_fsm_params.kbd_passcode_timeout / 10));
            }
            // entering "passcode" mode
            app_con_fsm_call_callback(REINITIALIZE);
            
            current_fsm_state = CONNECTED_PAIRING_ST;
            break;
            
        case NEW_HOST_EVT:
            app_con_fsm_call_callback(REINITIALIZE);
            break;
        
        case CONN_CMP_EVT:
            if (con_fsm_params.use_pref_conn_params) {
                // Timer for sending the CONN_PARAM_UPDATE is set by the caller
                dbg_puts(DBG_FSM_LVL, "  (+) update params timer\r\n");
                conn_upd_pending = true;
            }
            
#ifdef EXTENDED_TIMERS_ON
            dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
            if (conn_timer_remaining == 0) {
                conn_timer_remaining++; // in case it's zero, just go to CONNECTED_ST in order to reach IDLE_ST
            }
            app_extended_timer_set(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP, conn_timer_remaining);
#endif            
            if (con_fsm_params.has_white_list || con_fsm_params.has_virtual_white_list) {
                add_host_in_white_list(app_env.peer_addr_type, &app_env.peer_addr, app_alt_peer_get_active_index());
            }
            
            // has the host public /* or static random address */?
            if (   (ADDR_PUBLIC == app_env.peer_addr_type)
                /*|| ( (app_env.peer_addr_type == ADDR_RAND) && ((app_env.peer_addr.addr[5] & SMPM_ADDR_TYPE_STATIC) == SMPM_ADDR_TYPE_STATIC) )*/ ) {
                is_bonded = true;       // do Directed advertising to this host if connection is lost
            } else {
                is_bonded = false;      // will be set to true if Privacy is enabled and the Reconnection address is written
            }
            current_fsm_state = CONNECTED_ST;
            break;
        case DISCONN_EVT:
            // Advertising settings remain unchanged!
            current_fsm_state = ADVERTISE_ST;
            start_adv_undirected();
            break;
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
        
        
    case CONNECTED_PAIRING_ST:
        if (con_fsm_params.has_passcode_timeout) {
            dbg_puts(DBG_FSM_LVL, "  (-) passcode timer\r\n");
            ke_timer_clear(APP_CON_FSM_TIMER, TASK_APP);
        }
        
        switch(evt) {
        case PASSKEY_ENTERED:
            if (con_fsm_params.has_passcode_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (+) passcode timer\r\n");
                app_timer_set(APP_CON_FSM_TIMER, TASK_APP, (con_fsm_params.kbd_passcode_timeout / 10));
            }
            break;
        case CONN_CMP_EVT:
            if (con_fsm_params.use_pref_conn_params) {
                // Timer for sending the CONN_PARAM_UPDATE is set by the caller
                dbg_puts(DBG_FSM_LVL, "  (+) update params timer\r\n");
                conn_upd_pending = true;
            }
            
            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
                if (conn_timer_remaining == 0) {
                    conn_timer_remaining++; // in case it's zero, just go to CONNECTED_ST in order to reach IDLE_ST
                }
                app_extended_timer_set(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP, conn_timer_remaining);
            }
            
            if (con_fsm_params.has_white_list || con_fsm_params.has_virtual_white_list) {
                add_host_in_white_list(app_env.peer_addr_type, &app_env.peer_addr, app_alt_peer_get_active_index());
            }
            
            // has the host public /* or static random address */?
            if (   (ADDR_PUBLIC == app_env.peer_addr_type) 
                /*|| ( (app_env.peer_addr_type == ADDR_RAND) && ((app_env.peer_addr.addr[5] & SMPM_ADDR_TYPE_STATIC) == SMPM_ADDR_TYPE_STATIC) )*/ ) {
                is_bonded = true;       // do Directed advertising to this host if connection is lost
            } else {
                is_bonded = false;      // will be set to true if Privacy is enabled and the Reconnection address is written
            }       
            current_fsm_state = CONNECTED_ST;
            break;
        case DISCONN_EVT:
            // Advertising settings remain unchanged!
            current_fsm_state = ADVERTISE_ST;
            start_adv_undirected();
            break;
        case TIMER_EXPIRED_EVT:
            if (con_fsm_params.has_passcode_timeout) {
                app_con_fsm_disconnect();
                
                if (__builtin_popcount(multi_bond_status) > 1) {
                    current_adv_state = BONDED_ADV;
                } else {
                    current_adv_state = UNBONDED_ADV;
                }
                current_fsm_state = DISCONNECTED_INIT_ST;
            } else {
                ASSERT_WARNING(0);
            }
            break;
            
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
        
        
    case CONNECTED_ST:
        switch(evt) {
        case PAIRING_REQ_EVT:
            if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) ) {
                app_con_fsm_call_callback(INDICATE_CONNECTION_IN_PROGRESS);
            }
                
            if (con_fsm_params.use_pref_conn_params) {
                // Clear update connection params timer or it may hit while we are in CONNECTED_PAIRING_ST...
                ke_timer_clear(APP_CON_FSM_TIMER, TASK_APP);
                dbg_puts(DBG_CONN_LVL, "(-) params update timer\r\n");
            }
                
            if (con_fsm_params.has_passcode_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (+) passcode timer\r\n");
                app_timer_set(APP_CON_FSM_TIMER, TASK_APP, (con_fsm_params.kbd_passcode_timeout / 10));
            }
            // entering "passcode" mode
            app_con_fsm_call_callback(REINITIALIZE);

            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (*) inactivity timer\r\n");
                conn_timer_remaining = ke_timer_remove(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP);
                // in case of EXTENDED_TIMERS, extended_timer_cnt holds its value and can be used 
                // when the timer is reset later...
            }
            current_fsm_state = CONNECTED_PAIRING_ST;
            break;
        case SPOTAR_START_EVT:
            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (*) inactivity timer\r\n");
                conn_timer_remaining = ke_timer_remove(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP);
                // in case of EXTENDED_TIMERS, extended_timer_cnt holds its value and can be used 
                // when the timer is reset later...
            }
            spota_on = true;
            break;
        case SPOTAR_END_EVT:
            // Part of the handling of this event is done by the "common code" before the switch statement
            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
                app_extended_timer_set(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP, conn_timer_remaining);
            }
            break;
        case KEY_PRESS_EVT:
            if (con_fsm_params.has_inactivity_timeout) {
                if (spota_on == false) {
                    dbg_puts(DBG_FSM_LVL, "  (!) inactivity timer\r\n");
                    conn_timer_remaining = con_fsm_params.kbd_inactivity_timeout / 10;
                    app_extended_timer_set(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP, conn_timer_remaining);
                }
            }
            break;
        case CONN_CMP_EVT:  // PARAM_UPDATE was completed!
            break;
        case SWITCH_EVT:
            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (*) inactivity timer\r\n");
                conn_timer_remaining = ke_timer_remove(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP);
                // in case of EXTENDED_TIMERS, extended_timer_cnt holds its value and can be used 
                // when the timer is reset later...
            }
            
            if (con_fsm_params.use_pref_conn_params) {
                // Clear update connection params timer
                ke_timer_clear(APP_CON_FSM_TIMER, TASK_APP);
                dbg_puts(DBG_CONN_LVL, "(-) params update timer\r\n");
            }
            
            if (__builtin_popcount(multi_bond_status) > 1) {
                adv_timer_remaining = (con_fsm_params.bonded_discoverable_timeout / 10) + (multi_bond_enabled==MULTI_BOND_REJECT_NONE ? 0 : con_fsm_params.alt_pair_disconn_time);
                current_adv_state = BONDED_ADV;
            } else {
                adv_timer_remaining = (con_fsm_params.unbonded_discoverable_timeout / 10) + (multi_bond_enabled==MULTI_BOND_REJECT_NONE ? 0 : con_fsm_params.alt_pair_disconn_time);
                current_adv_state = UNBONDED_ADV;
            }
            current_fsm_state = DISCONNECTED_INIT_ST;
            break;
        case DISCONN_EVT:
            if (con_fsm_params.has_inactivity_timeout) {
                dbg_puts(DBG_FSM_LVL, "  (*) inactivity timer\r\n");
                conn_timer_remaining = ke_timer_remove(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP);
                // in case of EXTENDED_TIMERS, extended_timer_cnt holds its value and can be used 
                // when the timer is reset later...
            }

            if (con_fsm_params.use_pref_conn_params) {
                // Clear update connection params timer
                ke_timer_clear(APP_CON_FSM_TIMER, TASK_APP);
                dbg_puts(DBG_CONN_LVL, "(-) params update timer\r\n");
            }
                        
            if (is_bonded && (bond_info.env.auth & GAP_AUTH_BOND)) {
                current_fsm_state = DIRECTED_ADV_ST;
                start_adv_directed();
            } else {
                adv_timer_remaining = con_fsm_params.bonded_discoverable_timeout / 10;
                current_adv_state = BONDED_ADV;
                current_fsm_state = ADVERTISE_ST;
                start_adv_undirected();
            }
            break;
            
        case TIMER_EXPIRED_EVT:
            if (con_fsm_params.use_pref_conn_params) {
                send_connection_upd_req();
                conn_upd_pending = false;
            } else {
                ASSERT_WARNING(0);
            }
            break;
        case INACT_TIMER_EXP_EVT:
            if (con_fsm_params.has_inactivity_timeout) {
                app_con_fsm_disconnect();
                current_fsm_state = DISCONNECTED_IDLE_ST;
            } else {
                ASSERT_WARNING(0);
            }
            break;
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    case DISCONNECTED_IDLE_ST:
        switch(evt) {
        case DISCONN_EVT:
            if (con_fsm_params.has_inactivity_timeout)  {
                conn_timer_remaining = con_fsm_params.kbd_inactivity_timeout / 10;     // reset inactivity timer here
            }
        
            if (con_fsm_params.is_normally_connectable) {
                current_adv_state = SLOW_ADV;
                current_fsm_state = ADVERTISE_ST;
                start_adv_undirected();
            } else {
                if (con_fsm_params.has_deepsleep) {
                    app_set_deep_sleep();
                }
                app_ble_ext_wakeup_on();
                app_con_fsm_call_callback(PREPARE_DEEPSLEEP);
                current_fsm_state = IDLE_ST;
            }
            break;
        case CONN_CMP_EVT:  // PARAM_UPDATE was completed!
            break;
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    case DISCONNECTED_INIT_ST:
        switch(evt) {
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            current_fsm_state = ADVERTISE_ST;
            start_adv_undirected();
            break;

        case CONN_CMP_EVT:  // PARAM_UPDATE was completed!
            break;
        
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    case DIRECTED_ADV_ST:
        switch(evt) {
        case TIMER_EXPIRED_EVT:
            adv_timer_remaining = con_fsm_params.bonded_discoverable_timeout / 10;
            current_adv_state = BONDED_ADV;
            current_fsm_state = ADVERTISE_ST;
            start_adv_undirected();
            break;
            
        case CONN_REQ_EVT:
            // prepare advertising settings in case connection setup fails
            adv_timer_remaining = con_fsm_params.bonded_discoverable_timeout / 10;
            current_adv_state = BONDED_ADV;
            spota_on = false;
            current_fsm_state = CONNECTION_IN_PROGRESS_ST;
            break;
        
        default:
            ASSERT_WARNING(0);
            break;
        }
        break;
    default:
        ASSERT_WARNING(0);
        break;
    }
    
    dbg_printf(DBG_FSM_LVL, "    (N) %s\r\n", state_names[current_fsm_state]);
}

/**
 ****************************************************************************************
 * @brief Requests the reset of the bonding data and the white list entries
 *
 * @param[in] reset: true if bonding data must be reset; false to cancel the request    
 *
 * @return    void
 ****************************************************************************************
 */
void app_con_fsm_request_reset_bonding_data(bool reset)
{
    if (con_fsm_params.has_nv_rom) {
        reset_bonding_request = reset;
    }
}

/**
 ****************************************************************************************
 * @brief Check if a requests to reset the bonding data is pending
 *
 * @param   None    
 *
 * @return  value of reset_bonding_request
 ****************************************************************************************
 */
bool app_con_fsm_bonding_data_reset_pending(void)
{
    return reset_bonding_request;
}

/**
 ****************************************************************************************
 * @brief Resets the bonding data and the white list entries
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_reset_bonding_data(void)
{
    // CLRP clears bonding info unconditionnaly
    if (con_fsm_params.has_white_list || con_fsm_params.has_virtual_white_list) {
        clear_white_list();
    }  
    memset( (uint8_t *)&bond_info.env, 0, sizeof(struct app_sec_env_tag) );
    is_bonded = false;
}

/**
 ****************************************************************************************
 * @brief   Requests update of connection params
 *          After connection and, optionally, pairing is completed, this function 
 *          is called to (optionally) modify the connection parameters.
 *          Must be called from app_param_update_func() 
 *          or directly from app_param_update_start()
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_param_update_func(void)
{
    app_con_fsm_call_callback(INDICATE_CONNECTED);    
    app_con_fsm_state_update(CONN_CMP_EVT);
    
    if (con_fsm_params.use_pref_conn_params) {
        //Set a timer to update connection params
        app_timer_set(APP_CON_FSM_TIMER, TASK_APP, (con_fsm_params.time_to_request_param_upd / 10));
        dbg_puts(DBG_CONN_LVL, "** Set param update timer\r\n");
    }    
}

/**
 ****************************************************************************************
 * @brief     Handles the rejection of connection params from the host
 *            Must be called from app_update_params_rejected_func() 
 *            or directly from gapc_cmp_evt_handler()
 *
 * @param[in] The status of gapc_cmp_evt_handler message
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_update_params_rejected_func(uint8_t status)
{
    if (status != GAP_ERR_REJECTED) {
        ASSERT_INFO(0, param->status, APP_PARAM_UPD);

        // Disconnect
        app_disconnect();
    }
    else {
        // it's application specific what to do when the Param Upd request is rejected
        
        // Go to Connected State
        ke_state_set(TASK_APP, APP_CONNECTED);
        app_con_fsm_state_update(CONN_CMP_EVT);
    } 
}

/**
 ****************************************************************************************
 * @brief   Handles the completion of connection params update
 *          Actions taken after the reception of the host's reply to a connection update 
 *          params we sent.
 *          Must be called from app_update_params_rejected_func() 
 *          or directly from gapc_cmp_evt_handler()
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_update_params_complete_func(void)
{
    app_con_fsm_state_update(CONN_CMP_EVT);
}

/**
 ****************************************************************************************
 * @brief   Configures connection FSM when connection is established.
 *          Must be called from app_connection_func() 
 *          or directly from gapc_connection_req_ind_handler()
 *          
 * @param[in] param  parameters passed from the stack
 *
 * @return  bool  If connection is accepted return true. Otherwise return false.
 ****************************************************************************************
 */
bool app_con_fsm_connection_func(struct gapc_connection_req_ind const *param)
{
    // Check if the received Connection Handle was valid
    if (app_env.conidx != GAP_INVALID_CONIDX) {
        ke_state_set(TASK_APP, APP_CONNECTED);  // Update TASK_APP state (MUST always be done!)

       /*--------------------------------------------------------------
        * ENABLE REQUIRED PROFILES
        *-------------------------------------------------------------*/
        dbg_printf(DBG_ALL, "gap_le_create_conn_req_cmp_evt_handler() (%d, %d, %d, %d)\r\n", 
                (int)param->con_interval,
                (int)param->con_latency,
                (int)param->sup_to,
                (int)param->clk_accuracy
              );

        dbg_printf(DBG_ALL, "Peer addr %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
            param->peer_addr.addr[0], param->peer_addr.addr[1], param->peer_addr.addr[2], 
            param->peer_addr.addr[3], param->peer_addr.addr[4], param->peer_addr.addr[5]);
        
        app_con_fsm_state_update(CONN_REQ_EVT);

        if (con_fsm_params.has_virtual_white_list) {
            // Public and Static Random Addresses are checked here. Resolvable Random Addresses are checked after
            // they are resolved.
            if (   (ADDR_PUBLIC == param->peer_addr_type)
                || ( (param->peer_addr_type == ADDR_RAND) && ((param->peer_addr.addr[5] & SMPM_ADDR_TYPE_STATIC) == SMPM_ADDR_TYPE_STATIC) ) ) {
                if (!lookup_public_in_virtual_white_list(param->peer_addr_type, &param->peer_addr)) {
                    app_disconnect();
                    return false;
                }
            }
        }
        
        if (con_fsm_params.has_multi_bond) {
            if (app_alt_pair_check_peer(param) == false) {
                return false;                            
            }
        }
        // Connection is accepted!

        multi_bond_resolved_peer_pos = 0;
        
#if (BLE_APP_SEC)
        // send connection confirmation
        app_connect_confirm(bond_info.env.auth);
# else
        // send connection confirmation
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
# endif

        if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) ) {
            // set a timer in case pairing/encyption does not follow
            app_timer_set(APP_CON_FSM_ENC_TIMER, TASK_APP, (con_fsm_params.enc_safeguard_timeout / 10));
            dbg_puts(DBG_FSM_LVL, "  (+) Connection FSM ENC timer\r\n");

            app_con_fsm_call_callback(INDICATE_CONNECTION_IN_PROGRESS);
            
            if (con_fsm_params.has_security_request_send) {
                if ( (app_env.peer_addr_type == ADDR_RAND) && ((app_env.peer_addr.addr[BD_ADDR_LEN - 1] & 0xC0) == SMPM_ADDR_TYPE_PRIV_RESOLV) ) {
                    //Resolve address
                    struct gapm_resolv_addr_cmd *cmd = (struct gapm_resolv_addr_cmd *)KE_MSG_ALLOC_DYN(GAPM_RESOLV_ADDR_CMD, 
                                    TASK_GAPM, TASK_APP, gapm_resolv_addr_cmd, 
                                    MAX_BOND_PEER * sizeof(struct gap_sec_key) );
                    
                    cmd->operation = GAPM_RESOLV_ADDR; // GAPM requested operation
                    cmd->nb_key = MAX_BOND_PEER; // Number of provided IRK 
                    cmd->addr = app_env.peer_addr; // Resolvable random address to solve
                    if (con_fsm_params.has_nv_rom) {
                        if (MBOND_LOAD_INFO_AT_INIT == false) {
                            memcpy(cmd->irk, irk_array.irk, MAX_BOND_PEER * sizeof(struct gap_sec_key)); // Array of IRK used for address resolution (MSB -> LSB)
                        }
                        
                        if (MBOND_LOAD_INFO_AT_INIT) {
                            for (int i = 0; i < MAX_BOND_PEER; i++) {
                                cmd->irk[i] = bond_array[i].irk;
                            }
                        }
                    } else {
                        cmd->irk[0] = bond_info.irk; // Only one member in the "array", the "previous" host, if any.
                    }

                    ke_msg_send(cmd);
                } else {
                    app_security_start();
                }
            }
        }
        else {
            // no bonding --> connection setup proceeds directly
            app_param_update_start();
        } 
    } else {
        // No connection has been established. Update state.
        app_con_fsm_state_update(TIMER_EXPIRED_EVT);
        return false;
    }
    
    return true;
}

/**
 ****************************************************************************************
 * @brief   Sets the advertising and the scan response data
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */

static void app_set_adv_data(void)
{
    /*-----------------------------------------------------------------------------
     * Set the Advertising Data
     *-----------------------------------------------------------------------------*/
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &app_adv_data_length,
                &app_adv_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        app_adv_data_length = APP_ADV_DATA_LENGTH;
        memcpy(&app_adv_data[0], APP_ADV_DATA, app_adv_data_length);
    }
        
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &app_scanrsp_data_length,
                &app_scanrsp_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
        if (app_scanrsp_data_length > 0) { 
            memcpy(&app_scanrsp_data[0], APP_SCNRSP_DATA, app_scanrsp_data_length);
        }
    }

    #ifdef APP_DFLT_DEVICE_NAME
    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    int8_t device_name_length = APP_ADV_DATA_MAX_SIZE - app_adv_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0) {
        // Get default Device Name (No name if not enough space)
        int8_t temp_len;
        
        temp_len = (strlen(APP_DFLT_DEVICE_NAME) <= device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
        
        if (temp_len > 0) {
            device_name_length = temp_len;
        }
        // else device_name_length shows the available space in the ADV pkt
        
        memcpy(&app_adv_data[app_adv_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);
        app_adv_data[app_adv_data_length]     = device_name_length + 1;         // Length
        
        if (temp_len > 0)  {
            app_adv_data[app_adv_data_length + 1] = '\x09';                     // Complete Local Name Flag
            app_adv_data_length += (device_name_length + 2);                    // Update Advertising Data Length
        } else {
            app_adv_data[app_adv_data_length + 1] = '\x08';                     // Shortened Local Name Flag
            app_adv_data_length += (device_name_length + 2);                    // Update Advertising Data Length
            device_name_length = 0;                                             // To add the full name in the Scan Response data
        }
    }
        
    if (device_name_length > 0)
        return; // device name has been added

    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Scan Response Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_scanrsp_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0) {
        // Get default Device Name (No name if not enough space)
        device_name_length = (strlen(APP_DFLT_DEVICE_NAME) <= device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
        if (device_name_length > 0) {
            memcpy(&app_scanrsp_data[app_scanrsp_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);

            app_scanrsp_data[app_scanrsp_data_length]     = device_name_length + 1; // Length
            app_scanrsp_data[app_scanrsp_data_length + 1] = '\x09';                 // Device Name Flag
            
            app_scanrsp_data_length += (device_name_length + 2);                    // Update Scan response Data Length
        }
    }
    #endif // APP_DFLT_DEVICE_NAME
}

/**
 ****************************************************************************************
 * @brief   Prepares and sends the reply to the GAPC_PAIRING_REQ msg
 *          Must be called from app_send_pairing_rsp_func() 
 *          or directly from gapc_bond_req_ind_handler()
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    dbg_puts(DBG_CONN_LVL, "    GAPC_PAIRING_REQ ind\r\n");
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    
    // Feature check cannot be perfomed by the application. For example, the Host may have sent the
    // LL_CONNECT_REQ with No MITM and Bond. Although we require MITM and Bond we should reply. It is
    // left to the Host to check whether the requirements can be met and the Pairing procedure should
    // proceed or not.

    cfm->accept = true;
    
    if (current_fsm_state == CONNECTED_ST) {
        // special case: got a PAIRING_REQ when already paired and connected! Do not clear bond_info
        // in this case or else the DB will be corrupted (i.e. the notifications will be disabled
        // while the host had them enabled and won't re-enable them after the 2nd Pairing...).
    } else {
        // Clear bond data of previous connection (if any)
        memset(&bond_info, 0, sizeof(struct bonding_info_));
        bond_info.info = BOND_INFO_DEFAULT_VAL;
        updatedb_from_bonding_info(&bond_info);
        reset_active_peer_pos();
    }
    
    app_con_fsm_state_update(PAIRING_REQ_EVT);
    
    // OOB information
    cfm->data.pairing_feat.oob              = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size         = KEY_LEN;

    if (con_fsm_params.has_mitm) {
        // Authentication requirements
        // We do not return NO_BOND even if an NV memory is not present. The reason is that
        // the Host may drop the Pairing procedure in case it requires bonding (i.e. Windows do that).
        // Of course, since an NV memory is not present we fake we are bonding. We expect this to be used
        // in test or controlled environments only though.
            cfm->data.pairing_feat.auth     = GAP_AUTH_REQ_MITM_BOND;
        // IO capabilities
        cfm->data.pairing_feat.iocap        = GAP_IO_CAP_KB_ONLY;
        //Security requirements
        cfm->data.pairing_feat.sec_req      = GAP_SEC1_AUTH_PAIR_ENC;
    }
    else {
        // Authentication requirements
        // See above about NO_BOND...
            cfm->data.pairing_feat.auth     = GAP_AUTH_REQ_NO_MITM_BOND;
        // IO capabilities
        cfm->data.pairing_feat.iocap        = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;    // Make sure that MITM won't be selected
        //Security requirements
        cfm->data.pairing_feat.sec_req      = GAP_SEC1_NOAUTH_PAIR_ENC;
    }
    
    //Initiator key distribution
    if ( (app_env.peer_addr_type == ADDR_RAND) && ((app_env.peer_addr.addr[BD_ADDR_LEN - 1] & 0xC0) == SMPM_ADDR_TYPE_PRIV_RESOLV) ) {
        cfm->data.pairing_feat.ikey_dist    = GAP_KDIST_IDKEY;
    } else {
        cfm->data.pairing_feat.ikey_dist    = GAP_KDIST_NONE;
    }
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist        = GAP_KDIST_ENCKEY;

    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief   Starts the passcode entry
 *          Must be called from app_mitm_passcode_entry_func() 
 *          or directly from gapc_bond_req_ind_handler()
 *
 * @param[in] src_id  id of the kernel task which caused this to happen
 * @param[in] dest_id id of the kernel task which received the message
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    dbg_puts(DBG_CONN_LVL, "    GAPC_TK_EXCH\r\n");
    
    // store task IDs
    mitm_src_id = src_id;
    mitm_dest_id = dest_id;

    app_con_fsm_call_callback(START_PASSCODE); 
    sync_passcode_entered_evt = true;
}

/**
 ****************************************************************************************
 * @brief   Sends the passcode that the user entered to the Host
 *
 * @param[in] code  The code to report to the Host
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_mitm_passcode_report(uint32_t code)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, mitm_src_id, mitm_dest_id, gapc_bond_cfm);
    
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;

    memset(cfm->data.tk.key, 0, KEY_LEN);

    cfm->data.tk.key[3] = (uint8_t)((code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((code & 0x000000FF) >>  0);

    ke_msg_send(cfm);

    dbg_printf(DBG_CONN_LVL, "Code: %d\n\r", code);   
}

/**
 ****************************************************************************************
 * @brief   Pairing completed - Handles the completion of Pairing
 *          Must be called from app_paired_func() 
 *          or directly from gapc_bond_ind_handler()
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_paired_func(void)
{
    dbg_puts(DBG_CONN_LVL, "    GAPC_PAIRING_SUCCEED\r\n");
    
    // We may reach this point after getting an LL_ENC_REQ from an unbonded host 
    // with EDIV and RAND set to zero. Reject the Host in case of MITM since no Pairing has been perfomed.
    if ( (bond_info.env.auth == GAP_AUTH_NONE) && con_fsm_params.has_mitm) {
        // reset bond_info
        if (con_fsm_params.has_mitm) {
            bond_info.env.auth = GAP_AUTH_REQ_MITM_BOND;
        } else {
            bond_info.env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
        }
        
        // disconnect
        app_con_fsm_disconnect();
        
        return;
    }
    
    if (con_fsm_params.has_multi_bond) {
        multi_bond_enabled = MULTI_BOND_REJECT_NONE;
        ke_timer_clear(APP_CON_FSM_ALT_PAIR_TIMER, TASK_APP);
    }
    
    if (bond_info.env.auth & GAP_AUTH_BOND) {
        app_alt_pair_store_bond_data();
    }
            
    app_param_update_start();
}

/**
 ****************************************************************************************
 * @brief   Check if encryption will be accepted (LL_ENC_REQ received but not when in PAIRING mode)
 *          for this link
 *          Must be called from app_validate_encrypt_req_func() 
 *          or directly from gapc_encrypt_req_ind_handler()
 *
 * @param[in] param  The message from the host
 *
 * @return  true, if everything is ok
 *          false, if the request is rejected
 ****************************************************************************************
 */
bool app_con_fsm_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    if (con_fsm_params.has_nv_rom) {
        if(((bond_info.env.auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(bond_info.env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (bond_info.env.ediv == param->ediv)) {
            if (con_fsm_params.has_multi_bond) {
                // the connecting host is the last host we connected to
                if (multi_bond_enabled==MULTI_BOND_REJECT_LAST || multi_bond_enabled==MULTI_BOND_REJECT_ALL_KNOWN) {
                    return false;
                }
            }
            // if it's not blocked then no NV memory access is required to load keys.
            updatedb_from_bonding_info(&bond_info);
            app_update_usage_count(bond_info.env.nvds_tag & 0xF);
        } else { 
            if (app_alt_pair_load_bond_data(&param->rand_nb, param->ediv) == 1) {   
                if (con_fsm_params.has_multi_bond) {
                    if (multi_bond_enabled==MULTI_BOND_REJECT_ALL_KNOWN) {
                        return false;
                    }
                }
                app_con_fsm_state_update(NEW_HOST_EVT);
            }
        }
    } else {
        if(((bond_info.env.auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(bond_info.env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (bond_info.env.ediv == param->ediv)) {
            // bond_info.info is OK
        } else {
            bond_info.info = BOND_INFO_DEFAULT_VAL;
        }
        updatedb_from_bonding_info(&bond_info);
    }
    
    return true;
}

/**
 ****************************************************************************************
 * @brief   Received LL_START_ENC_RSP : Enryption is completed -> Start normal operation. 
 *          Must be called from app_sec_encrypt_ind_func() 
 *          or directly from gapc_encrypt_ind_handler()
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_sec_encrypt_ind_func(void)
{
    if (con_fsm_params.has_multi_bond) {
        multi_bond_enabled = MULTI_BOND_REJECT_NONE;
       ke_timer_clear(APP_CON_FSM_ALT_PAIR_TIMER, TASK_APP);
    }
        
    if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) ) {
        ke_timer_clear(APP_CON_FSM_ENC_TIMER, TASK_APP);
    }
    app_param_update_start();
    // no need to store anything to the NV memory
}

/**
 ****************************************************************************************
 * @brief   Configures the application when the connection is terminated.
 *          Must be called from app_disconnect_func() 
 *          or directly from gapc_cmp_evt_handler()
 *          
 * @param[in] task_id   id of the kernel task calling this function
 * @param[in] param     parameters passed from the stack
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_disconnect_func()
{
    ke_timer_clear(APP_CON_FSM_TIMER, TASK_APP);
    
    if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) ) {
        ke_timer_clear(APP_CON_FSM_ENC_TIMER, TASK_APP);
    }
    
    if (con_fsm_params.has_multi_bond) {
        if (multi_bond_enabled==MULTI_BOND_REJECT_NONE) {
            app_con_fsm_call_callback(INDICATE_DISCONNECTED);
        } else {
            app_con_fsm_call_callback(INDICATE_OFF);
        }
    } else {
        app_con_fsm_call_callback(INDICATE_DISCONNECTED);
    }
    
    app_con_fsm_state_update(DISCONN_EVT);
    ke_state_set(TASK_APP, APP_CONNECTABLE); // APP_CONNECTABLE means "Idle"

    user_disconnection_req = false; // clear any multi-bond switch   
}

/**
 ****************************************************************************************
 * @brief   Bypasses the normal disconnection procedure, in order to avoid sending the 
 *          LL_TERMINATE_IND. Used only when the con_fsm_params.has_send_ll_terminate_ind is not defined.
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
static void app_fake_disconnect(void)
{
    if (con_fsm_params.has_send_ll_terminate_ind)
    {
        ASSERT_WARNING(0);
    }
    else
    {
        ke_msg_send_basic(APP_CON_FSM_TERMINATE_CONN_MSG, TASK_APP, TASK_APP);
    }
}

__INLINE void app_con_fsm_call_callback(enum con_fsm_state_update_callback_type type) 
{
    if (con_fsm_params.state_update_callback)
        (*con_fsm_params.state_update_callback)(type);
}    

/**
 ****************************************************************************************
 * @brief   Initialize the FSM
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_init(void)
{    
    app_alt_pair_init();        // Initialize Multi-Bonding (if applicable)
    app_set_adv_data();         // Prepare Advertising data
    sync_passcode_entered_evt = false;
    reset_bonding_request = false;
    user_disconnection_req = false;
    current_fsm_state = IDLE_ST;
}

/**
 ****************************************************************************************
 * @brief   Get the privacy flags
 *          
 * @param   None
 *
 * @return  The privacy flags for the gapm_set_dev_config_cmd command
 ****************************************************************************************
 */
uint8_t app_con_fsm_set_privacy(void)
{
    // Privacy settings bit field (0b1 = enabled, 0b0 = disabled)
    //  - [bit 0]: Privacy Support
    //  - [bit 1]: Multiple Bond Support (Peripheral only); If enabled, privacy flag is
    //             read only.
    //  - [bit 2]: Reconnection address visible.
    
    if (con_fsm_params.has_privacy) {
// Set the reconnection address if privacy is set for this fearure
// gapm_set_recon_addr(&my_addr);
        return (GAPM_CFG_VISIBLE_RECON_ADDR_EN | GAPM_CFG_PRIVACY_EN);
    } else {
        return 0;
    }
}

/**
 ****************************************************************************************
 * @brief   Synchronize asynchronously generated requests for transmission 
 *          of messages to OS tasks. Must be called from app_asynch_trm()
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_process_async_tasks(void)
{
    if (reset_bonding_request) {
        app_con_fsm_reset_bonding_data();
        reset_bonding_request = false;
    }

    if (sync_passcode_entered_evt == true) {
        app_con_fsm_state_update(PASSKEY_ENTERED);
        sync_passcode_entered_evt = false;
    }

     if (user_disconnection_req) {
        if (app_alt_pair_disconnect()) {
            if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP)) {
                app_con_fsm_call_callback(INDICATE_DISCONNECTED);
            }
            app_con_fsm_state_update(SWITCH_EVT);
        }
        user_disconnection_req = false;
    }
}

/**
 ****************************************************************************************
 * @brief     Connect to the host the bonding info of which are stored 
 *            in the specified position 
 *          
 * @param[in] entry  The position in the storage memory that contains 
 *                   the host's bonding information
 *
 * @return    true if host is known and switch is successful. 
 *            false if the specified position does not contain valid host information.
 ****************************************************************************************
 */
bool app_con_switch_to_peer(int8_t entry)
{
    ASSERT_ERROR(con_fsm_params.has_virtual_white_list || con_fsm_params.has_white_list);
    
    // If the new host is the current host or there are no valid data for this entry
    if(entry==app_alt_peer_get_active_index() || app_alt_pair_load_entry(entry)==false) {
        return false;
    }
    
    clear_white_list();
    add_host_in_white_list(bond_info.env.peer_addr_type, &(bond_info.env.peer_addr), entry);
    app_con_fsm_request_disconnect(MULTI_BOND_REJECT_NONE);  // do not reject known hosts
    
    return true;
}

/**
 ****************************************************************************************
 * @brief     Store the bonding info of the next host that will be bonded 
 *            to the specified position 
 *          
 * @param[in] entry  The position in the storage memory where the bonding data
 *                   will be stored
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_add_host_to_entry(int8_t entry)
{
    clear_white_list();
    app_alt_pair_force_next_store_entry(entry);

    app_con_fsm_request_disconnect(MULTI_BOND_REJECT_ALL_KNOWN);  // reject all known hosts
}

/**
 ****************************************************************************************
 * @brief   Force disconnection from the connected host
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_disconnect(void)
{
    if (con_fsm_params.has_send_ll_terminate_ind) {
        app_disconnect();
    }
    else {
        app_fake_disconnect();                    
    }
}   

/**
 ****************************************************************************************
 * @brief      Request asynchronously a disconnection from the currently connected host
 *             The request will be synchonized with OS in app_con_fsm_process_async_tasks()
 *          
 * @param[in]  reject_hosts Specify how the hosts that will try to connect will be treated
 *             MULTI_BOND_REJECT_NONE: all hosts will be allowed to connect
 *             MULTI_BOND_REJECT_LAST: Last connected host will be rejected
 *             MULTI_BOND_REJECT_ALL_KNOWN: All known hosts stored in the memory will be rejected
 *
 * @return     void
 ****************************************************************************************
 */
void app_con_fsm_request_disconnect(enum multi_bond_host_rejection reject_hosts)
{
    if (con_fsm_params.has_multi_bond) {
        multi_bond_enabled = reject_hosts;
        user_disconnection_req = true;
    }
}

/**
 ****************************************************************************************
 * @brief   Get the current fsm state
 *
 * @param   None    
 *
 * @return  current fsm state
 ****************************************************************************************
 */
enum main_fsm_states app_con_fsm_get_state(void)
{
    return current_fsm_state;
}

/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events for GAPM_RESOLV_ADDR
 *
 * @param[in] param     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void app_con_fsm_handle_cmp_evt(struct gapm_cmp_evt const *param)
{
    switch(param->operation) {
    case GAPM_RESOLV_ADDR:
        if (con_fsm_params.has_nv_rom) {
            if (param->status == GAP_ERR_NO_ERROR) {
                // found!
                // do nothing, handled in app_con_fsm_gapm_addr_solved_ind_handler()
            }
            else if (param->status != GAP_ERR_NOT_FOUND) {
                ASSERT_WARNING(0);
            }
            else
            {
                // Host address has not been resolved - New Host
                if (con_fsm_params.has_virtual_white_list) {
                    if ((virtual_wlist_policy != ADV_ALLOW_SCAN_ANY_CON_ANY)) {
                        app_disconnect();
                    }
                    else if (con_fsm_params.has_security_request_send) {
                        app_security_start();
                    }
                }
                else if (con_fsm_params.has_security_request_send) {
                    app_security_start();
                }
            }
        }
        break;
    case GAPM_ADD_DEV_IN_WLIST:
        if (con_fsm_params.has_white_list) {
            if (param->status == GAP_ERR_NO_ERROR) {
                white_list_written++;
            }
            // else something went wrong, i.e. the host may 
            // already be in the White List or the White List
            // is empty...
        }
        break;
    case GAPM_RMV_DEV_FRM_WLIST:
        if (con_fsm_params.has_white_list) {
            if (white_list_written == 0) {
                ASSERT_WARNING(0);
            }
            
            if (param->status == GAP_ERR_NO_ERROR) {
                white_list_written--;
            }
            // else something went wrong, i.e. the host may 
            // not be in the White List or the White List
            // is empty...
        }
        break;
    }
}

/**
 ****************************************************************************************
 * @brief   Resets the inactivity timeout
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_reset_inactivity_timeout(void)
{
    if (app_con_fsm_get_state() == CONNECTED_ST) {
        app_con_fsm_state_update(KEY_PRESS_EVT);
    }   
}


/**
 ****************************************************************************************
 * @brief   Starts the advertising proceddure
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_start_advertising(void)
{
    if (app_con_fsm_get_state() == IDLE_ST) {
        app_con_fsm_state_update(KEY_PRESS_EVT);
    }   
}

/**
 ****************************************************************************************
 * @brief   Handles what needs to be done after Undirected advertising finishes
 *          Must be called from app_adv_undirect_complete() 
 *          or directly from gapm_cmp_evt_handler()
 *
 * @param[in]   status
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_adv_undirect_complete(uint8_t status)
{
    if ( (status != GAP_ERR_NO_ERROR) && (status != GAP_ERR_CANCELED) ) {
        ASSERT_ERROR(0); // unexpected error
    }
    
    if (status == GAP_ERR_CANCELED) {
        if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
            app_con_fsm_state_update(TIMER_EXPIRED_EVT);
    }
}


/**
 ****************************************************************************************
 * @brief   Handles what needs to be done after Directed advertising finishes
 *          Must be called from app_adv_direct_complete() 
 *          or directly from gapm_cmp_evt_handler()
 *
 * @param[in]   status
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_adv_direct_complete(uint8_t status)
{
    if ( (status != GAP_ERR_NO_ERROR) && (status != GAP_ERR_TIMEOUT) ) {
        ASSERT_ERROR(0); // unexpected error
    }
    
    if (status == GAP_ERR_TIMEOUT) {
        if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
            app_con_fsm_state_update(TIMER_EXPIRED_EVT);
    }
}

#endif // USE_CONNECTION_FSM

