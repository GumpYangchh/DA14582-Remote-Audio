/**
 ****************************************************************************************
 *
 * @file app_con_fsm.h
 *
 * @brief Connection FSM header file.
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

#ifndef APP_CON_FSM_H_
#define APP_CON_FSM_H_

#include <stdbool.h>
#include "gapc_task.h"              // gapc functions and messages
#include "gapm_task.h"              // gapm functions and messages
#include "app_multi_bond.h"
#include "app_white_list.h"


/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup Connection FSM
 *
 * @brief Connection FSM.
 *
 * @{
 ****************************************************************************************
 */

#if ( HAS_INACTIVITY_TIMEOUT && HAS_NORMALLY_CONNECTABLE )
#warning "It does not make sense to use inactivity timeout when the NormallyConnectable flag is set..."
#endif

enum main_fsm_states {
    IDLE_ST,
    CONNECTED_ST,
    ADVERTISE_ST,
    CONNECTION_IN_PROGRESS_ST,
    CONNECTED_PAIRING_ST,
    DISCONNECTED_IDLE_ST,
    DISCONNECTED_INIT_ST,
    DIRECTED_ADV_ST,
};

enum adv_states {
    SLOW_ADV,
    UNBONDED_ADV,
    BONDED_ADV,
};

enum main_fsm_events {
    NO_EVENT,
    KEY_PRESS_EVT,
    TIMER_EXPIRED_EVT,
    PAIRING_REQ_EVT,
    CONN_REQ_EVT,
    CONN_CMP_EVT,
    DISCONN_EVT,
    CONN_UPD_RESP_EVT,
    PASSKEY_ENTERED,
    SWITCH_EVT,
    NEW_HOST_EVT,
    SPOTAR_START_EVT,
    SPOTAR_END_EVT,
    ADV_TIMER_EXP_EVT,
    INACT_TIMER_EXP_EVT
};
                                   

/**
 ****************************************************************************************
 * @brief Updates the main FSM with an event
 *
 * @param[in]   evt   The main FSM event
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_state_update(enum main_fsm_events);

/**
 ****************************************************************************************
 * @brief Resets the bonding data and the white list entries
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_reset_bonding_data(void);

/**
 ****************************************************************************************
 * @brief   Configures connection FSM when connection is established.
 *          
 * @param[in] param  parameters passed from the stack
 *
 * @return  bool  If connection is accepted return true. Otherwise return false.
 ****************************************************************************************
 */
bool app_con_fsm_connection_func(struct gapc_connection_req_ind const *param);


/**
 ****************************************************************************************
 * @brief   Initialize the FSM
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_init(void);

/**
 ****************************************************************************************
 * @brief   Get the privacy flags
 *          
 * @param   None
 *
 * @return  The privacy flags for the gapm_set_dev_config_cmd command
 ****************************************************************************************
 */
uint8_t app_con_fsm_set_privacy(void);
    
/**
 ****************************************************************************************
 * @brief   Prepares and sends the reply to the GAPC_PAIRING_REQ msg
 *
 * @param[in] param  The message from the host
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_send_pairing_rsp_func(struct gapc_bond_req_ind *param);

/**
 ****************************************************************************************
 * @brief   Get passcode entry mode
 *
 * @param[in] src_id  id of the kernel task which caused this to happen
 * @param[in] dest_id id of the kernel task which received the message
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id);

/**
 ****************************************************************************************
 * @brief   Sends the passcode that the user entered to the Host
 *
 * @param[in] code  The code to report to the Host
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_mitm_passcode_report(uint32_t code);
 
 /**
 ****************************************************************************************
 * @brief   Pairing completed - Handles the completion of Pairing
 *
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_paired_func(void);

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
bool app_con_fsm_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param);
   
/**
 ****************************************************************************************
 * @brief   Received LL_START_ENC_RSP : Enryption is completed -> Start operation. 
 *          After completion of the encryption of the link, it sets the keyboard
 *          to normal report mode
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_sec_encrypt_ind_func(void);

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
void app_con_fsm_disconnect_func(void);


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
bool app_con_switch_to_peer(int8_t entry);


/**
 ****************************************************************************************
 * @brief   Force disconnection from the connected host
 *          
 * @param   None
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_disconnect(void);


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
void app_con_fsm_add_host_to_entry(int8_t entry);

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
void app_con_fsm_param_update_func(void);

/**
 ****************************************************************************************
 * @brief     Handles the rejection of connection params from the host
 *
 * @param[in] The status of gapc_cmp_evt_handler message
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_update_params_rejected_func(uint8_t status);
    
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
void app_con_fsm_update_params_complete_func(void);


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
void app_con_fsm_process_async_tasks(void);


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
void app_con_fsm_request_disconnect(enum multi_bond_host_rejection reject_hosts);

/**
 ****************************************************************************************
 * @brief Requests the reset of the bonding data and the white list entries
 *
 * @param[in] reset: true if bonding data must be reset; false to cancel the request    
 *
 * @return    void
 ****************************************************************************************
 */
void app_con_fsm_request_reset_bonding_data(bool reset);

/**
 ****************************************************************************************
 * @brief Check if a requests to reset the bonding data is pending
 *
 * @param   None    
 *
 * @return  value of reset_bonding_request
 ****************************************************************************************
 */
bool app_con_fsm_bonding_data_reset_pending(void);

/**
 ****************************************************************************************
 * @brief get the current fsm state
 *
 * @param   None    
 *
 * @return  current fsm state
 ****************************************************************************************
 */
enum main_fsm_states app_con_fsm_get_state(void);


/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events
 *
 * @param[in] param     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void app_con_fsm_handle_cmp_evt(struct gapm_cmp_evt const *param);
/**
 ****************************************************************************************
 * @brief   Resets the inactivity timeout
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_reset_inactivity_timeout(void);

/**
 ****************************************************************************************
 * @brief   Starts the advertising proceddure
 *
 * @param   None    
 *
 * @return  void
 ****************************************************************************************
 */
void app_con_fsm_start_advertising(void);

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
void app_con_fsm_adv_undirect_complete(uint8_t status);

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
void app_con_fsm_adv_direct_complete(uint8_t status);
#endif // APP_CON_FSM_H_
