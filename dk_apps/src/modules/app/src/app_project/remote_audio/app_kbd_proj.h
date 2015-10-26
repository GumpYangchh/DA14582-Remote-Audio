/**
 ****************************************************************************************
 *
 * @file app_kbd_proj.h
 *
 * @brief HID Keyboard hooks header file.
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

#ifndef APP_KBD_PROJ_H_
#define APP_KBD_PROJ_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_dis_task.h"           // diss message handlers
#include "app_batt_task.h"          // bass message handlers
#include "hogpd_task.h"             // hogpd message IDs
#include "app_kbd_proj_task.h"      // hogpd message handlers
#include "app_kbd_leds.h"           // leds message handlers

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 

/*
 * DEFINES
 ****************************************************************************************
 */
 
 
/*
 * EXPORTED VARIABLES
 ****************************************************************************************
 */
extern struct bonding_info_ bond_info;

extern struct hogpd_env_tag hogpd_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


#if (HAS_AUDIO)
/**
 ****************************************************************************************
 * @brief Handle the Stream On - Off notifications
 *        Called when STREAMONOFF is received
 *
 * @param[in]   msgid: the message structure
 * @param[in]   param: pointer to the hogpd report
 * @param[in]   dest_id: destination task id
 * @param[in]   src_id: source task id
 *
 * @return      KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_hogpd_report_ind_handler(ke_msg_id_t const msgid,
                           struct hogpd_report_info *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
#endif
                                 
/**
 ****************************************************************************************
 * @brief   Handler of the HID Timer - Action depends on the app state
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_hid_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);



/**
 ****************************************************************************************
 * @brief   Handler of a dummy TASK_APP msg sent to trigger the timer
 *
 * @param[in] msgid 
 * @param[in] param
 * @param[in] dest_id
 * @param[in] src_id
 *
 * @return  KE_MSG_CONSUMED
 *
 * @remarks In case of delayed start, a dummy message is sent to the TASK_APP.
 *          This msg is put in the queue when the BLE is woken up. When the
 *          handler is called, it is certain that the BLE is running and 
 *          the timer may start.
 ****************************************************************************************
 */                                    
int app_hid_msg_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handle the Stream On - Off notifications
 *        Called when STREAMONOFF is received
 *
 * @param    none
 *
 * @return   void
 ****************************************************************************************
 */                                    
#if (HAS_AUDIO || HAS_BMI055)
void streamdatad_streamonoff_hogpd (void);
#endif                                    
/// @} APP

#endif // APP_KBD_PROJ_H_
