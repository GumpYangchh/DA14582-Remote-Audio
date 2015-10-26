/**
 ****************************************************************************************
 *
 * @file nmi_handler.c
 *
 * @brief NMI hundler. 
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

#include "app_dbg.h"
#include "app_flash.h"
#include <stdbool.h>

/*
 * This is the base address in Retention RAM where the stacked information will be copied.
 */
#define STATUS_BASE (0x81850)

/*
 * Usage
 * -----
 *
 * Upon exception, all valuable (and available) information will be copied to the area
 * with base address equal to STATUS_BASE. The user can press the Reset button and
 * restart the debugger. Since the Retention RAM is not reset, the information will be
 * accessible via any Memory window.
 *
 * If SmartSnippets is used and P1[3] is available then the user may uncomment the call
 * to set_pxact_gpio() to have a visual indication of the occurrence of a Hard Fault 
 * exception.
 */
void NMI_HandlerC(uint32_t *hardfault_args)
{
#if (DEVELOPMENT_DEBUG || DEBUG_WRITE_TO_SPI || defined(PRODUCTION_DEBUG_OUTPUT))
    app_dbg_pack_regs(hardfault_args);
#endif
    
#if (DEBUG_WRITE_TO_SPI)
    app_spi_flash_write_dbg_regs(APP_DBG_FAULT_NMI);
#endif

    // Reached this point due to a WDOG timeout    
    SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1);        // turn off radio PD
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain
    SetBits16(CLK_RADIO_REG, BLE_LP_RESET, 1);      // reset the BLE LP timer
    NVIC_ClearPendingIRQ(BLE_WAKEUP_LP_IRQn);       // clear any pending LP IRQs

#if (DEVELOPMENT_DEBUG)
    SetWord16(SET_FREEZE_REG, FRZ_WDOG);            // Stop WDOG
    SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);    // enable debugger to be able to re-attach
    
    memcpy((void*)STATUS_BASE, app_dbg_registers, APP_DBG_TOTAL_REGS*sizeof(uint32_t));
    if ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP)
        __asm("BKPT #0\n");
    else
        while(true);
#else // DEVELOPMENT_DEBUG
    #ifdef PRODUCTION_DEBUG_OUTPUT
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));
    
    app_dbg_write_fault(APP_DBG_FAULT_NMI);
    #endif    
    
    // Remap addres 0x00 to ROM and force execution
    SetWord16(SYS_CTRL_REG, (GetWord16(SYS_CTRL_REG) & ~REMAP_ADR0) | SW_RESET );
#endif // DEVELOPMENT_DEBUG
}
