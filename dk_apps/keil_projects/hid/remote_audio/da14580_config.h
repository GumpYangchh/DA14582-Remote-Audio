/**
 ****************************************************************************************
 *
 * @file da14580_config.h
 *
 * @brief RCU compile configuration file.
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

#ifndef DA14580_CONFIG_H_
    #define DA14580_CONFIG_H_

    #include "da14580_stack_config.h"
    #include "config\memory_config.h"
/**
 ******************************************************************************
 * Profiles - Stack configuration section
 ******************************************************************************
 */

    /* FullEmbedded - FullHosted */
    #define CFG_APP  

    /* Profiles */
    #define CFG_PRF_BASS
    #define CFG_PRF_DISS
    #define CFG_PRF_HOGPD
    //#define CFG_PRF_SPOTAR
    #ifdef CFG_PRF_SPOTAR
        #define CFG_SPOTAR_SPI_DISABLE          //SPI support is disabled
        #define CFG_SPOTAR_UPDATE_DISABLE       //SUOTA is not included
        #define CFG_APP_SPOTAR              0
        #define SPOTAR_PATCH_AREA           1   //Placed in the RetRAM when SPOTAR_PATCH_AREA is 0 and in SYSRAM when 1
    #endif

    /* Misc */
    #define CFG_NVDS 
    #define CFG_APP_SEC
    #define CFG_CHNL_ASSESS
    #define CFG_LUT_PATCH
    #define CFG_APP_KEYBOARD 


/* Enables the Memory leak s/w patch */
#define MEM_LEAK_PATCH_ENABLED

/**
 ******************************************************************************
 * Sleep & Watchdog section
 ******************************************************************************
 */
/* Watchdog */
    #define CFG_WDOG 

/* Sleep modes */
//#define CFG_EXT_SLEEP  
#undef CFG_DEEP_SLEEP  

/* Maximum user connections */
#define BLE_CONNECTION_MAX_USER 1

#define USE_POWER_OPTIMIZATIONS

#define CFG_USE_DEFAULT_XTAL16M_TRIM_VALUE_IF_NOT_CALIBRATED

/**
 ******************************************************************************
 * Debug - Log section
 ******************************************************************************
 */
  
    #define DEBUG_WRITE_TO_SPI  0

    /* Build for OTP or JTAG */
    #define DEVELOPMENT_DEBUG   0   //0: code at OTP, 1: code via JTAG

    /* Log HEAP usage */
    #define LOG_MEM_USAGE       0   //0: no logging, 1: logging is active

    /* Application boot from OTP memory - Bootloader copies OTP Header to sysRAM */
    //#define APP_BOOT_FROM_OTP

    /* NVDS structure is padded with 0 - NVDS struture data must be written in OTP in production procedure */
    //#define READ_NVDS_STRUCT_FROM_OTP
    
    /* Debug output in Production mode (DEVELOPMENT_DEBUG == 0) */
//    #define PRODUCTION_DEBUG_OUTPUT
    
    /* UART Tx pin to be used for Debug output in Production mode */
    
    #ifdef PRODUCTION_DEBUG_OUTPUT
        #define PRODUCTION_DEBUG_PORT   GPIO_PORT_0//UART_TX_GPIO_PORT
    	#define PRODUCTION_DEBUG_PIN    GPIO_PIN_4//UART_TX_GPIO_PIN
    #endif

    /* arch_printf() and arch_puts() */
    //#define CFG_PRINTF
    #ifdef CFG_PRINTF
        #define HAS_PRINTF 1
    #else
        #define HAS_PRINTF 0
    #endif


/**
 ******************************************************************************
 * Clock & Boot section
 ******************************************************************************
 */
 // #define CFG_LP_CLK		// Please refer to config\hw_config.h 
 
/**
 ******************************************************************************
 * Use connection FSM for pairing/bonding to one or multiple hosts
 ******************************************************************************
 */
    #define CFG_CONNECTION_FSM

/**
 ******************************************************************************
 * Battery configuration section
 ******************************************************************************
 */
    /* Battery Type (if any) */
    #define USED_BATTERY_TYPE                       BATT_AAA
    #define BATTERY_LEVEL_POLLING_PERIOD            (10000)
    #define BATTERY_ALERT_AT_PERCENTAGE_LEFT        (25)    // Will set custom battery alert level and also enable on/off behaviour with hysteresis
    #define CUSTOM_BATTERY_LEVEL_ALERT_LED_HANDLING


/**
 ******************************************************************************
 * GPIO Drv configuration section
 ******************************************************************************
 */
 
    /* GPIO driver - Pin allocation checks */
    #define GPIO_DRV_PIN_ALLOC_MON_DISABLED 1   //Checks are off

    /* GPIO driver - GPIO_INTx handling */
    #define GPIO_DRV_IRQ_HANDLING_DISABLED      //IRQs are off


/**
 ******************************************************************************
 * Misc settings section
 ******************************************************************************
 */
 
    /* HIDS instances: 1 */
    #define USE_ONE_HIDS_INSTANCE

    /* BASS instances: 1 */
    #define USE_ONE_BAS_INSTANCE
    
 /**
 ******************************************************************************
 * RW data placement configuration section
 ******************************************************************************
 */
// Please define memory configuration in config\memory_config.h

/**
 ******************************************************************************
 * Memory maps section (Scatterfile)
 ******************************************************************************
 */

// Please define memory configuration in config\memory_config.h

#endif // DA14580_CONFIG_H_
