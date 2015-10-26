#ifndef _APP_DBG_
    #define _APP_DBG_

/**
 ****************************************************************************************
 *
 * @file app_dbg.h
 *
 * @brief Header file with routines to output debug messages to the UART. 
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
#include "global_io.h"
#include "datasheet.h"
#include "core_cm0.h"

typedef enum
{
    APP_DBG_FAULT_NMI   = 0,
    APP_DBG_FAULT_HARD,
    APP_DBG_FAULT_FLASH,
    APP_DBG_FAULT_NONE  = 0xFF
} app_dbg_fault_t;

#define APP_DBG_HARD_ARGS   8
#define APP_DBG_SCB_REGS    6
#define APP_DBG_TOTAL_REGS  (APP_DBG_HARD_ARGS + 1 + APP_DBG_SCB_REGS)
#define APP_DBG_REGS_SIZE   sizeof(app_dbg_fault_t) + APP_DBG_TOTAL_REGS*sizeof(uint32_t)

extern uint32_t app_dbg_registers[];
    
void app_dbg_write_fault(app_dbg_fault_t fault);
void app_dbg_write_msg(char *msg);
void app_dbg_pack_regs(uint32_t *hardfault_args);
    
#endif //_APP_DBG_
