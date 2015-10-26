/**
 ****************************************************************************************
 *
 * @file app_con_fsm_task.c
 *
 * @brief Connection FSM handlers.
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration

#if (USE_CONNECTION_FSM)

#if (BLE_APP_PRESENT)


#include "app_task.h"                  // Application Task API
#include "llc_util.h"

#include "app_con_fsm_task.h"
#include "app_con_fsm_debug.h"

#include "app_utils.h"

extern enum adv_states current_adv_state;
extern int adv_timer_remaining;
extern struct bonding_info_ bond_array[MAX_BOND_PEER];

/**
 ****************************************************************************************
 * @brief   Called when the Service Changed indication has been successfully received by the Host
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 *****************************************************************************************
 */
int app_con_fsm_gattc_svc_chng_cmp_evt_handler(ke_msg_id_t const msgid,
                                         struct gattc_cmp_evt const *ind,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    if(ind->req_type == GATTC_SVC_CHANGED) {
        // Clear bond_info.info so that we do not re-send the Service Changed Indication to this Host again
        bond_info.info &= 0xFF;

        // Update NV memory
        app_alt_pair_store_bond_data();
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief  Handler which checks the resolution procedure of the host's address
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 ****************************************************************************************
 */ 
int app_con_fsm_gapm_addr_solved_ind_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    int i;
    struct gapm_addr_solved_ind *ind = (struct gapm_addr_solved_ind *)param;
    
    // The Host has been found! We don't care about the key and the BD address. 
    // The entry will be located again when EDIV & RAND are provided.
    
    // Since we have the IRK, we can find the real address
    if (MBOND_LOAD_INFO_AT_INIT) {
        for (i = 0; i < MAX_BOND_PEER; i++) {
            if (!memcmp(&bond_array[i].irk.key[0], &ind->irk.key[0], KEY_LEN)) {
                multi_bond_resolved_peer_pos = i+1;
                break;
            }
        }
    } else {
        for (i = 0; i < MAX_BOND_PEER; i++) {
            if (!memcmp(&irk_array.irk[i].key[0], &ind->irk.key[0], KEY_LEN)) {
                break;
            }
        }
        
        if (i != MAX_BOND_PEER) { // found
            multi_bond_resolved_peer_pos = i+1;
        }
    }
    
    if (con_fsm_params.has_virtual_white_list) {
        if (!lookup_rand_in_virtual_white_list(multi_bond_resolved_peer_pos-1)) {
            app_disconnect();
        }
    }
    
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handler of a dummy TASK_APP msg sent to trigger the adv timer
 *        In case of advertising start from IDLE_ST, a dummy message is sent to the TASK_APP. 
 *        This msg is put in the queue when the BLE is woken up. When the handler is called, 
 *        it is certain that the BLE is running and the timer may start.
 *
 * @param[in]   msgid    
 * @param[in]   param
 * @param[in]   dest_id
 * @param[in]   src_id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_con_fsm_start_adv_msg_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Start Adv timer (if not running)
    if (current_adv_state != SLOW_ADV) {
        if (adv_timer_remaining > 0) {
            app_timer_set(APP_CON_FSM_ADV_TIMER, TASK_APP, adv_timer_remaining);
        } else {
            app_con_fsm_state_update(ADV_TIMER_EXP_EVT);
        }
    }
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   Handler for the message sent by app_fake_disconnect() function
 *          Used to bypasses the normal disconnection procedure, in order to avoid sending the 
 *          LL_TERMINATE_IND. Used only when the con_fsm_params.has_send_ll_terminate_ind is not defined
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_con_fsm_disconnect_cmd_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    if (con_fsm_params.has_send_ll_terminate_ind) {
        ASSERT_WARNING(0);
    } else {
        uint8_t status = CO_ERROR_COMMAND_DISALLOWED;

        switch(ke_state_get(dest_id)) {
        case LLC_FREE:
        case LLC_DISC:
        case LLC_STOPPING:
            break;
        case LLCP_WAIT_ACK:
            return (KE_MSG_SAVED);
        default:
            // Termination procedure can be started at any time
            status = CO_ERROR_NO_ERROR;
            // Execute the termination procedure
            llc_util_dicon_procedure(app_env.conhdl, CO_ERROR_REMOTE_USER_TERM_CON); 
            break;
        }

        llc_common_cmd_status_send(LLC_DISCON_STAT_EVT, src_id, dest_id, status, app_env.conhdl);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   Handler of the ADV Timer 
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks If this timer expires then the advertising period is over. 
 *          If a connection is being setup, we will allow the procedure to
 *          complete. If it fails though, we do not return to ADVERTISE_ST
 *          but move on to IDLE_ST.
 ****************************************************************************************
 */
int app_con_fsm_adv_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    dbg_puts(DBG_FSM_LVL, "ADV timer exp\r\n");
    app_con_fsm_state_update(ADV_TIMER_EXP_EVT);    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   Handler of the Enc Timer
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks In case encryption is not activated by the remote host and the connection
 *           is still alive (if it wasn't then the timer would have been cleared),
 *           the handler will drop the connection. This situation appears in certain
 *           cases when pairing fails.
 ****************************************************************************************
 */
int app_con_fsm_enc_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    dbg_puts(DBG_FSM_LVL, "ENC timer exp\r\n");
    
    if ( (con_fsm_params.has_mitm) || (con_fsm_params.has_nv_rom) ) {
        app_con_fsm_disconnect();
    } else {
        ASSERT_WARNING(0);
    }
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   Handler of the Connection FSM Timer - Action depends on the app state
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_con_fsm_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    dbg_puts(DBG_FSM_LVL, "Connection FSM timer exp\r\n");
     
    // Timer elapsed!
    app_con_fsm_state_update(TIMER_EXPIRED_EVT);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   Handler of the Connection FSM Inactivity Timer - Action depends on the app state
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_con_fsm_inactivity_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    dbg_puts(DBG_FSM_LVL, "Connection FSM Inactivity timer exp\r\n");
    
    if (app_extended_timer_process(APP_CON_FSM_INACTIVITY_TIMER, TASK_APP)) {
        return (KE_MSG_CONSUMED);
    }
    // Timer elapsed!
    app_con_fsm_state_update(INACT_TIMER_EXP_EVT);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief  The Privacy flag has been altered by the remote host
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 ****************************************************************************************
 */
int app_con_fsm_updated_privacy_ind_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // 1. If disabled, use the public address in advertising
    // 2. If enabled generate a reconnection address (if 0:0:0:0:0:0) and write it.
    
    if (con_fsm_params.has_privacy) {
        #warning "Full feature implementation (Privacy) is pending..."
        
        struct gapm_updated_privacy_ind *privacy_ind = (struct gapm_updated_privacy_ind *)param;
        if (privacy_ind->privacy) {
            ASSERT_WARNING(0);
        }
    }
    return (KE_MSG_CONSUMED);
}                                       

/**
 ****************************************************************************************
 * @brief  The host updated the reconnection address 
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 ****************************************************************************************
 */
int app_con_fsm_updated_recon_addr_ind_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    if (con_fsm_params.has_privacy) {
        ASSERT_WARNING(0);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief  The Client Char Config of Service Changed has been updated
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 ****************************************************************************************
 */
int app_con_fsm_service_changed_cfg_ind_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gattc_svc_changed_cfg *ind = (struct gattc_svc_changed_cfg *)param;
        
    // ind->ind_cfg holds the value
    app_multi_bond_store_ccc(CHAR_SERVICE_CHANGED_POS, 0, ind->ind_cfg);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief  SPOTAR start/stop callback
 *         Gets called when SPOTAR starts or stops to update the main FSM.
 *
 * @param[in] status  The status of SPOTAR process
 *
 * @return  void
 *
 ****************************************************************************************
 */

#if (BLE_SPOTA_RECEIVER)
void app_spotar_callback(const uint8_t status)
{
    if (status == SPOTAR_START) {
        app_con_fsm_state_update(SPOTAR_START_EVT);
    } else if (status == SPOTAR_END) {
        app_con_fsm_state_update(SPOTAR_END_EVT);
    } else {
        ASSERT_WARNING(0);
    }
}
#endif //BLE_SPOTA_RECEIVER

#endif //(BLE_APP_PRESENT)

#endif //USE_CONNECTION_FSM

/// @} APPTASK
