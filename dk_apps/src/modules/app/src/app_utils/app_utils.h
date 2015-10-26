#ifndef _APP_UTILS_
#define _APP_UTILS_

#include "ke_timer.h"

/**
 ****************************************************************************************
 *
 * @file app_utils.h
 *
 * @brief Helper utils
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer 
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
 * @brief   Set a timer that supports timeout greater than 5 min
 *          Called when a timeout of more than 5 min (that stack supports) is needed.
 *          Only one such timer (only APP_CON_FSM_TIMER) can be on at a time!
 *          
 * @param[in] timer_id 
 * @param[in] task_id
 * @param[in] delay
 *
 * @return  void
 *
 * #remarks if delay is <= KE_TIMER_DELAY_MAX then the timer is "resumed". Thus, if
 *          extended_timer_cnt != 0, it continues counting 
 *          (extended_timer_cnt * KE_TIMER_DELAY_MAX) + delay
 *
 *****************************************************************************************
 */                                    
void app_extended_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task_id, uint32_t delay);

/**
 ****************************************************************************************
 * @brief   Clears a timer that was set with app_extended_timer_set()
 *          Called when a timeout of more than 5 min (that stack supports) was set.
 *          Do not directly use ke_timer_clear on such a timer!
 *          
 * @param[in] timer_id 
 * @param[in] task_id
 *
 * @return  void
 *
 *****************************************************************************************
 */
void app_extended_timer_clear(ke_msg_id_t const timer_id, ke_task_id_t const task_id);
 
 /**
 ****************************************************************************************
 * @brief   Process a timer that was set with app_extended_timer_set()
 *          Called when the kernel timer timer_id expires
 *          
 * @param[in] timer_id 
 * @param[in] task_id
 *
 * @return  true if the exteneded timer is still running
 *
 *****************************************************************************************
 */
bool app_extended_timer_process(ke_msg_id_t const timer_id, ke_task_id_t const task_id);

void app_delay(int delay);

#endif // _APP_UTILS_
