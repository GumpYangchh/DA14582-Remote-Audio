 /**
 ****************************************************************************************
 *
 * @file memory_config.h
 *
 * @brief  Memory configuration file
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
 
#ifndef _MEMORY_CONFIG_H
#define _MEMORY_CONFIG_H

/**
 ******************************************************************************
 * RW data placement configuration section
 ******************************************************************************
 */
 
   /*
     * Set the arrays used by the application to be placed in .constdata section.
     * If not defined, the arrays will be put in the ".data" section which can optionally be
     * compressed and placed elsewhere.
     * WARNING: If not defined then the following options are available:
     *     - uncompress data in RetRAM once at startup
     *     - uncompress data in SysRAM once at startup and use Extended Sleep (done automatically if EXT_SLEEP_SETUP is used)
     *     - uncompress data in SysRAM at startup and after every Deep Sleep wakeup
     */
    #undef ARRAYS_AS_CONST_DATA

    #if defined(ARRAYS_AS_CONST_DATA)
        #define KBD_TYPE_QUALIFIER  const
        #define KBD_ARRAY_ATTRIBUTE  
    #else
        #define KBD_TYPE_QUALIFIER  
        #define KBD_ARRAY_ATTRIBUTE __attribute__((section(".data")))
    #endif

    /* Select where the .data section will be placed and whether it will be compressed */
    #define DATA_UNCOMPRESSED       1
    #define DATA_COMPRESSED_SYSRAM  2       // only in Extended Sleep
    #define DATA_COMPRESSED_RETRAM  3       // re-init will be enforced

    #if defined(ARRAYS_AS_CONST_DATA)
        #define DATA_SECTION_SEL DATA_UNCOMPRESSED
    #else
        #define DATA_SECTION_SEL DATA_COMPRESSED_SYSRAM
    #endif

   /* Memory map */
#if defined(USE_MEMORY_MAP) || defined(DB_HEAP_SZ) || defined(ENV_HEAP_SZ) || defined(MSG_HEAP_SZ) || defined(NON_RET_HEAP_SZ)
#error "Please define memory configuration only in file memory_config.h"
#endif

    #define USE_MEMORY_MAP DEEP_SLEEP_SETUP

    /* Descriptors' reinitilization */
    #if (defined(CFG_DEEP_SLEEP) && (DATA_SECTION_SEL == DATA_COMPRESSED_RETRAM) )
        #define REINIT_DESCRIPT_BUF 1 //0: keep in RetRAM, 1: re-init is required (set to 0 when Extended Sleep is used)
    #else
        #define REINIT_DESCRIPT_BUF 0
    #endif
    
    #define DB_HEAP_SZ      (2048 + 128)
    #define ENV_HEAP_SZ     440

    #define MSG_HEAP_SZ     (1392- 64)
    #define NON_RET_HEAP_SZ 2048
        
    
#endif	// _MEMORY_CONFIG_H
