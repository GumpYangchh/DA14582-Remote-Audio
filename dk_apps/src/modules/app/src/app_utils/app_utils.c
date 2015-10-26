 /**
 ****************************************************************************************
 *
 * @file app_utils.c
 *
 * @brief Helper utils
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

#include "app.h"
#include "app_utils.h"

/**
 ****************************************************************************************
 * @brief   Set a timer that supports timeout greater than 5 min
 *          Called when a timeout of more than 5 min (that stack supports) is needed.
 *          Only one such timer can be on at a time!
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
 
int extended_timer_cnt __attribute__((section("retention_mem_area0"), zero_init));
 
void app_extended_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task_id, uint32_t delay)
{
#ifdef EXTENDED_TIMERS_ON
    if (delay <= KE_TIMER_DELAY_MAX) {
        app_timer_set(timer_id, task_id, (uint16_t)delay);
    } else {
        int rem = delay % KE_TIMER_DELAY_MAX;
        
        extended_timer_cnt = delay / KE_TIMER_DELAY_MAX;
        if (rem == 0) {
            rem = KE_TIMER_DELAY_MAX;
            extended_timer_cnt--;
        }
        app_timer_set(timer_id, task_id, rem);
    }
#else
    ASSERT_WARNING(delay <= KE_TIMER_DELAY_MAX); 
    app_timer_set(timer_id, task_id, (uint16_t)delay);
#endif    
}

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
void app_extended_timer_clear(ke_msg_id_t const timer_id, ke_task_id_t const task_id)
{
    
    ke_timer_clear(timer_id, task_id);
    
#ifdef EXTENDED_TIMERS_ON
    extended_timer_cnt = 0;
#endif    
}

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
bool app_extended_timer_process(ke_msg_id_t const timer_id, ke_task_id_t const task_id)
{
#ifdef EXTENDED_TIMERS_ON
    if (extended_timer_cnt) {
        extended_timer_cnt--;
        app_timer_set(timer_id, task_id, KE_TIMER_DELAY_MAX);    
        return true;
    }
#endif    
    return false;
}

void app_delay(int delay)
{
    for (volatile unsigned int i=0; i<delay; i++);  
}

/// @} APP
