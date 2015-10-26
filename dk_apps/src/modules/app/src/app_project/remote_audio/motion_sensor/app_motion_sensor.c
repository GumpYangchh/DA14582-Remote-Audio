 /**
 ****************************************************************************************
 *
 * @file app_motion_sensor.c
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

#if HAS_BMI055
#include "app_api.h"
#include "app_motion_sensor.h"
#include "app_stream.h"
#include "i2c_bmi055.h"
#include "app_kbd.h"


static int cnt=0;
static bool device_ready=false;

extern bool user_motion_left_click_pressed;

/**
 ****************************************************************************************
 * @brief Resets the LSB of a 16-bit integer or shifts it right by 4 places
 *
 * @param data  Pointer to the integer
 * @param left  true to reset the LSB, false to shift right by 4 places
 *
 * @return void
 ****************************************************************************************
 */
__INLINE void allign(int16_t *data, bool left)
{
    if (left) {
        *data &= 0xFFFE;
    } else {
        *data >>= 4;
    }
}

/**
 ****************************************************************************************
 * @brief Resets the LSB of a 16-bit integer or shifts it right by 4 places
 *
 * @param data  Pointer to the integer
 * @param left  true to reset the LSB, false to shift right by 4 places
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_read_data(struct s_app_motion_data *data)
{
    char tempdata;
    cnt++;
    data->timestamp=cnt;
    i2c_bmi055_init (0x19, 2, 0);
    i2c_bmi055_read_data((uint8_t *)data->acc_motion,2,0x6);
    //Hillcrest suggest left allignment
    allign(data->acc_motion, true);
    allign(data->acc_motion + 1, true);
    allign(data->acc_motion + 2, true);
    i2c_bmi055_read_data((uint8_t *) &tempdata,8,1);
    data->temperature=(int16_t)tempdata;
    i2c_bmi055_init (0x69, 2, 0);
    i2c_bmi055_read_data((uint8_t *)data->rot_motion,2,0x6);
    data->click_events=user_motion_left_click_pressed;
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_issue_bist(void)
{
    i2c_bmi055_init (0x69,2, 0);
    i2c_bmi055_write_byte(0x3C,1);
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static bool app_motion_test_bist(void)
{
    uint8_t val;
    i2c_bmi055_init (0x69,2, 0);
    val=i2c_bmi055_read_byte(0x3C);
    return val & (1<<4);
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_gyro_config(void)
{
    i2c_bmi055_init (0x69,2, 0);
    i2c_bmi055_write_byte(BMI055_PMU_RANGE,0  ); //range is set to +/-2000
    i2c_bmi055_write_byte(BMI055_PMU_BW,   0x2); //bandwidth 116Hz
    i2c_bmi055_write_byte(BMI055_ACCD_HBW, 0  ); //range 
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_accel_config(void)
{
    i2c_bmi055_init (0x19,2, 0);
    i2c_bmi055_write_byte(BMI055_PMU_RANGE,8  ); //range is set to +/-8g
    i2c_bmi055_write_byte(BMI055_PMU_BW,   0xF); //bandwidth BW 0x8 = 7.81 Hz -  0xF = 1000 HZ
    i2c_bmi055_write_byte(BMI055_ACCD_HBW, 0  ); //range 
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_sleep_bmi(void)
{
    cnt=0;
    device_ready=false;
    i2c_bmi055_suspend_device (0x69, BMI055_DEEP_SUSPEND);
    i2c_bmi055_suspend_device (0x19, BMI055_DEEP_SUSPEND);
    i2c_bmi055_release();
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_wakeup_bmi(void)
{
    i2c_bmi055_reset_device (0x69);
    i2c_bmi055_reset_device (0x19);
    cnt=0;
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_config_bmi(void)
{
    app_motion_gyro_config();
    app_motion_accel_config();
    cnt=0;
    device_ready=true;
}

/**
 ****************************************************************************************
 * @brief .
 *
 * @return void
 ****************************************************************************************
 */
static void app_motion_send_motion_not(void)
{
    struct s_app_motion_data data;
    memset (&data,0,sizeof (struct s_app_motion_data));

    app_motion_read_data (&data);
    if (device_ready) {
        app_stream_send_motionreport(&data);
    }
}

extern char motion_cpt_event;        
char state_bmi_pressed=0, state_bmi_released=3;

void app_motion_state_machine ( int pressed )
{
    if (pressed) {
        if (state_bmi_released == 1) { //active waiting to shutdown
            ke_timer_clear( APP_MOT_DIS_TIMER, TASK_APP);
            state_bmi_released=0;
        }
        state_bmi_released=0; //device is active
     
        switch (state_bmi_pressed) {
        case 0:
            app_motion_wakeup_bmi ();
            state_bmi_pressed=1;
            app_timer_set(APP_MOT_TIMER,TASK_APP,4);
            break;
        case 2:
            app_motion_issue_bist();
            state_bmi_pressed=3;
            app_timer_set(APP_MOT_TIMER,TASK_APP,1);
            break;
        case 4:
            if (app_motion_test_bist()) {
                app_motion_config_bmi();
                state_bmi_pressed=5;
                app_timer_set(APP_MOT_TIMER,TASK_APP,1);
            } else {
                app_motion_wakeup_bmi ();
                state_bmi_pressed=1;
                app_timer_set(APP_MOT_TIMER,TASK_APP,1);
            }
            break;
        case 6:
            if (motion_cpt_event == 1) {
                extern bool conn_upd_pending;
                if (app_kbd_check_conn_status() && !conn_upd_pending) {
                    app_motion_send_motion_not();
                }
                motion_cpt_event=0;
            }
            break;
        case 1:
        case 3:
        case 5:
        default:
            break;
        }
    } else {
        switch (state_bmi_released) {
        case 0:
            app_timer_set(APP_MOT_DIS_TIMER,TASK_APP,200);  //2 sec to get the cursor back
            state_bmi_released=1;
            break;
        case 2:
            app_motion_init_state_machine();
            break;
         case 1:
         default:
            break;                
        }
    }
}

void app_motion_init_state_machine ( void )
{
    state_bmi_pressed=0;
    state_bmi_released=3;
    ke_timer_clear( APP_MOT_TIMER, TASK_APP);
    ke_timer_clear( APP_MOT_DIS_TIMER, TASK_APP);
    app_motion_sleep_bmi ();
}

int app_motion_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (state_bmi_pressed < 6) {
        state_bmi_pressed++;
    }
	return (KE_MSG_CONSUMED);
}

int app_motion_disconnect_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (state_bmi_released < 2) {
        state_bmi_released++;
    }
	return (KE_MSG_CONSUMED);
}

#endif
