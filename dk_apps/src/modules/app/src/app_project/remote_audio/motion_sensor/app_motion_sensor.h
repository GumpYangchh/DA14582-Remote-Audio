 /**
 ****************************************************************************************
 *
 * @file app_motion_sensor.h
 *
 * @brief Motion Sensor application.
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */
 
#ifndef APP_MOTION_H_
#define APP_MOTION_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

 #include "ke_msg.h"
 
typedef  struct s_app_motion_data
{  
    int32_t timestamp;
    int16_t temperature;
    int16_t acc_motion[3];
    int16_t rot_motion[3];
    int16_t click_events;
} t_app_motion_data;


/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief .
 *
 * @param pressed
 *
 * @return void
 ****************************************************************************************
 */
void app_motion_state_machine(int pressed);

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
void app_motion_init_state_machine(void);

/**
 ****************************************************************************************
 * @brief .
 *
 * @param msgid     Id of the message received.
 * @param param     Pointer to the parameters of the message.
 * @param dest_id   ID of the receiving task instance (TASK_GAP).
 * @param src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_motion_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief .
 *
 * @param msgid     Id of the message received.
 * @param param     Pointer to the parameters of the message.
 * @param dest_id   ID of the receiving task instance (TASK_GAP).
 * @param src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_motion_disconnect_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
#endif
