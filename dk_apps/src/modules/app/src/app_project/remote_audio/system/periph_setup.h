/**
 ****************************************************************************************
 *
 * @file periph_setup.h
 *
 * @brief Peripherals setup header file. 
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
 
 #ifndef PERIPH_SETUP_H_
 #define PERIPH_SETUP_H_
    
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "global_io.h"
#include "arch.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#if BLE_SPOTA_RECEIVER
/****************************************************************************************/ 
/* SPI Flash configuration                                                             */
/****************************************************************************************/
#ifndef SPI_FLASH_SIZE
#define SPI_FLASH_SIZE      0x20000
#endif

#ifndef SPI_FLASH_PAGE_SIZE
#define SPI_FLASH_PAGE_SIZE 0x100
#endif

#endif // BLE_SPOTA_RECEIVER

/****************************************************************************************/ 
/* Wkupct configuration                                                                 */
/****************************************************************************************/ 
#define WKUP_ENABLED

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down.
 *
 *
 * @return void
 ****************************************************************************************
 */ 
void periph_init(void);

/**
 ****************************************************************************************
 * @brief GPIO_reservations - Globally reserved GPIOs
 *        Each application reserves its own GPIOs here.
 *        If there are GPIOs that have to be globally reserved (i.e. UART)
 *        then their reservation MUST be done BEFORE any application reservations.
 *
 * @return void
 ****************************************************************************************
 */

void GPIO_reservations(void);

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and GPIO ports (for debugging) are mapped
 *
 * @return void
 ****************************************************************************************
 */
void set_pad_functions(void);

#endif // PERIPH_SETUP_H_
