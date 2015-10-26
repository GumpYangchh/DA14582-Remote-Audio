/**
 ****************************************************************************************
 *
 * @file app_con_fsm_task.h
 *
 * @brief Connection FSM handlers header file.
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

#ifndef APP_CON_FSM_TASK_H_
#define APP_CON_FSM_TASK_H_


// EXTENDED TIMER

#include "gattc_task.h"             // gattc functions and messages
#include "ke_msg.h"                 // kernel message


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

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
                                    ke_task_id_t const src_id);

                                    
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
                                    ke_task_id_t const src_id);

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
                                    ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief   Handler for the message sent by app_fake_disconnect() function.
 *          Used to bypasses the normal disconnection procedure, in order to avoid sending the 
 *          LL_TERMINATE_IND. Used only when the HAS_SEND_LL_TERMINATE_IND is not defined.
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  void
 ****************************************************************************************
 */                                    
int app_con_fsm_disconnect_cmd_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

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
                                    ke_task_id_t const src_id);
                                    
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
                                    ke_task_id_t const src_id);		

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
                                    ke_task_id_t const src_id);                                    

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
                                    ke_task_id_t const src_id);

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
int app_updated_privacy_ind_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

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
                                    ke_task_id_t const src_id);
   
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
                                   ke_task_id_t const src_id);
                                    
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
                                   ke_task_id_t const src_id);
                    
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
void app_spotar_callback(const uint8_t status);                                   
                                   
#endif // APP_CON_FSM_TASK_H_
