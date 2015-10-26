/**
 ****************************************************************************************
 *
 * @file app_kbd_proj_task.h
 *
 * @brief HID Keyboard handlers header file.
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

#ifndef APP_KBD_PROJ_TASK_H_
#define APP_KBD_PROJ_TASK_H_


// EXTENDED TIMER

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @brief   Sets the HID Report Map value in the DB
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks Called when the DB has been initialized.
 *****************************************************************************************
 */
int keyboard_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      
/**
 ****************************************************************************************
 * @brief   Called when the HID profile is being disabled
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *****************************************************************************************
 */                                    
int keyboard_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief   Called when the HID report has been ACK'ed from the master
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks PRF_ERR_NTF_DISABLED (0x8A) may arrive if we tried to send a Boot report
 *          and the notifications have been disabled for some reason
 *****************************************************************************************
 */                                    
int keyboard_ntf_sent_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ntf_sent_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief   Called when the Protocol Mode char is written by the Host
 *          
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks Protocol Mode has been updated
 *****************************************************************************************
 */                                    
int keyboard_proto_mode_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_proto_mode_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
/**
 ****************************************************************************************
 * @brief   Called when the CCC of a Report is written by the Host
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
int keyboard_ntf_config_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_ntf_cfg_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);


/**
 ****************************************************************************************
 * @brief   Called when the Control Point char is written by the Host
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
int keyboard_ctnl_pt_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_ctnl_pt_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

#endif // APP_KBD_PROJ_TASK_H_
