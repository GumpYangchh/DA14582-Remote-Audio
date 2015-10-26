 /**
 ****************************************************************************************
 *
 * @file app_motion_sensor_test.c
 *
 * @brief Motion Sensor application test.
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


#if (HAS_BMI055)
#include "i2c_bmi055.h"
#include "app_motion_sensor.h"
#include "app_console.h"
#include "app_utils.h"

void app_motion_access_i2c_device(int address)
{
    volatile uint8_t rd_data[0x40], *pt;

    arch_puts("Init I2C for ");
    print_hex (address,' ');
    arch_puts("\r\n");
    arch_printf_process();
    
    arch_puts("Read I2C dev \n");    
    i2c_bmi055_init (address,2, 0);
    arch_printf_process();
    
    i2c_bmi055_read_data((uint8_t*)rd_data,0,0x40);
    int i,j;
    pt=rd_data;
    for (j=0;j<4;j++) {
        for (i=0;i<0x10;i++) {
            print_hex( (unsigned char) (*pt),',');
            pt++;
        }
    }
    arch_puts("\r\n");
    arch_printf_process();
}



void app_motion_test_delay ( void )
{
    arch_puts("Waiting.. \r\n");
    arch_printf_process();
    for (volatile unsigned int j=0;j<0xa;j++) {
        app_delay(0x5FFFFF);
        arch_puts(".");
        arch_printf_process();
    }
    arch_puts("\r\n");
}

static int cnt=0;

void app_motion_random_data (struct s_app_motion_data *data)
{
    cnt++;
    data->temperature= 23;
    data->timestamp=cnt;
    data->acc_motion[0]= (cnt &(1<<4))?(cnt&(0x3)+8):(cnt&(0x3)-8);
    data->rot_motion[0]= (cnt &(1<<8))?(cnt&(0x3)+8):(cnt&(0x3)-8);
}

#endif

