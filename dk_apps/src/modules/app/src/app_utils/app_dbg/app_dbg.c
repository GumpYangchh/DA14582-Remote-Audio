/**
 ****************************************************************************************
 *
 * @file app_dbg.c
 *
 * @brief Routines to output debug messages to the UART. 
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

#include "app_dbg.h"
#include "app_flash.h"
#include "gpio.h"
#include "uart.h"

#if defined(PRODUCTION_DEBUG_OUTPUT) || (DEBUG_WRITE_TO_SPI) || (DEVELOPMENT_DEBUG)

uint32_t app_dbg_registers[APP_DBG_TOTAL_REGS];

void app_dbg_pack_regs(uint32_t *hardfault_args)
{
    uint32_t    *scb_regs   = (uint32_t*)(SCB_BASE + 0x28),
                *address    = app_dbg_registers;

    for (int i = 0; i < APP_DBG_HARD_ARGS; i++, address++) {
        *address = hardfault_args[i];
    }
    *address = (uint32_t)hardfault_args;
    address++;
    
    for (int i = 0; i < APP_DBG_SCB_REGS; i++, address++) {
        *address = scb_regs[i];
    }
}    

#endif

#if defined(PRODUCTION_DEBUG_OUTPUT) 

static const char app_dbg_names[APP_DBG_TOTAL_REGS][5] =
{
    "r0  ",
    "r1  ",
    "r2  ",
    "r3  ",
    "r12 ",
    "lr  ",
    "pc  ",
    "psr ",
    "sp  ",
    "cfsr",
    "hfsr",
    "dfsr",
    "mmar",
    "bfar",
    "afsr"
};

void app_dbg_write_string(char *string)
{
    uart_write((uint8_t *)string, strlen(string), NULL);
    app_delay(5000);    
}

void app_dbg_write_register(uint32_t value, int index)
{
    char buf[18];
    int i, nibble;
    
    buf[0] = app_dbg_names[index][0];
    buf[1] = app_dbg_names[index][1];
    buf[2] = app_dbg_names[index][2];
    buf[3] = app_dbg_names[index][3];
    buf[4] = '=';
    buf[5] = '0';
    buf[6] = 'x';
    
    for (i = 14; i >= 7; i--) {
        nibble = value & 0xF;
        if (nibble < 10) {
            buf[i] = '0' + nibble;
        } else {
            buf[i] = 'a' + nibble - 10;
        }
        value >>= 4;
    }
    buf[15] = '\r';
    buf[16] = '\n';
    buf[17] = '\0';
    app_dbg_write_string(buf);
}

void app_dbg_write_init(void)
{
    // Switch to XTAL16 clock, if necessary
    if (!GetBits16(CLK_CTRL_REG, RUNNING_AT_XTAL16M)) {
        while (!GetBits16(SYS_STAT_REG, XTAL16_SETTLED));   // this takes some milli-seconds
        SetBits16(CLK_CTRL_REG, SYS_CLK_SEL, 0);            // select XTAL 16MHz
    }
    
    // Configure UART
    GPIO_ConfigurePin(PRODUCTION_DEBUG_PORT, PRODUCTION_DEBUG_PIN, OUTPUT, PID_UART1_TX, false);
    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);                // enable clock - always @16MHz
    uart_init(UART_BAUDRATE_115K2, 3);    
    #if (USE_WDOG)        
    SetWord16(WATCHDOG_REG, 0xC8); // Reset WDOG! 200 * 10.24ms active time for UART to finish printing!
    #endif    
}

void app_dbg_write_fault(app_dbg_fault_t fault)
{
    app_dbg_write_init();
    char *string;
    switch (fault) {
    case APP_DBG_FAULT_NMI:
        string = "\n--- NMI STATUS\r\n";
        break;
    case APP_DBG_FAULT_HARD:
        string = "\n--- HF STATUS\r\n";
        break;
    default:
        string = "\n--- Unknown STATUS\r\n";
    }
    app_dbg_write_string(string);
    
    for (int i = 0; i < APP_DBG_TOTAL_REGS; i++) {
        app_dbg_write_register(app_dbg_registers[i], i);
    }
}

void app_dbg_write_msg(char *msg)
{
    app_dbg_write_init();
    app_dbg_write_string(msg);
}

#endif
