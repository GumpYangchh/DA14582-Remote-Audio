/**
 ****************************************************************************************
 *
 * @file app_flash.h
 *
 * @brief Additinal routines to access the spi flash header file.
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

#ifndef _APP_FLASH_
#define _APP_FLASH_

#include "gpio.h"

#include <string.h>
#include "app_dbg.h"

#define APP_FLASH_BASE      0x3E000
#define APP_FLASH_MSG_SIZE  SPI_FLASH_SECTOR - APP_DBG_REGS_SIZE
#define APP_FLASH_REGS_BASE APP_FLASH_BASE + APP_FLASH_MSG_SIZE

/**
 ****************************************************************************************
 * @brief Initializes the flash pins and the SPI interface.
 *
 * @param[in]   void.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_spi_flash_peripheral_init(void);
    
/**
 ****************************************************************************************
 * @brief Releases the flash pins and the SPI interface.
 *
 * @param[in]   void.
 *
 * @return      void.
 ****************************************************************************************
 */

void app_spi_flash_peripheral_release(void);

/**
 ****************************************************************************************
 * @brief Simplified write data to a single page in flash. Used to write bonding data.
 *
 * @param[in]   data: pointer to the data.
 * @param[in]   address: the address in the flash memory.
 * @param[in]   size: the amount of data to write.
 *
 * @return      Amount of data written.
 ****************************************************************************************
 */
size_t app_spi_flash_write_random_page_data(const void *data, uint32_t address, size_t size);

/**
 ****************************************************************************************
 * @brief Write debug registers to flash.
 *
 * @param[in]   fault: type of fault that occurred.
 *
 * @return      APP_DBG_FAULT_NONE if writing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_write_dbg_regs(app_dbg_fault_t fault);

/**
 ****************************************************************************************
 * @brief Read debug registers from flash.
 *
 * @param[in]   void.
 *
 * @return      type of fault that occurred, or APP_DBG_FAULT_FLASH if reading failed.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_read_dbg_regs(void);

/**
 ****************************************************************************************
 * @brief Erase debug registers from flash.
 *
 * @param[in]   void.
 *
 * @return      APP_DBG_FAULT_NONE if erasing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_erase_dbg_regs(void);

/**
 ****************************************************************************************
 * @brief Write debug message to flash.
 *
 * @param[in]   msg: pointer to the message.
 *
 * @return      APP_DBG_FAULT_NONE if writing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_write_dbg_msg(const char *msg);

/**
 ****************************************************************************************
 * @brief Read debug message from flash.
 *
 * @param[in]   msg: pointer to the message bufer
 *
 * @param[in]   len: message buffer size
 *
 * @return      APP_DBG_FAULT_NONE if reading succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_read_dbg_msg(char *msg, size_t len);

/**
 ****************************************************************************************
 * @brief Erase debug message from flash.
 *
 * @param[in]   void.
 *
 * @return      APP_DBG_FAULT_NONE if erasing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_erase_dbg_msg(void);



__INLINE void declare_flash_spi_power_down_gpios(void)
{
#ifdef HAS_FLASH_SPI_POWER_DOWN
    RESERVE_GPIO( SPI_FLASH_PD, FLASH_SPI_POWER_DOWN_PORT, FLASH_SPI_POWER_DOWN_PIN, PID_GPIO);
#endif
}
        
__INLINE void flash_spi_power_down(void)
{
#ifdef HAS_FLASH_SPI_POWER_DOWN
    GPIO_ConfigurePin( FLASH_SPI_POWER_DOWN_PORT, FLASH_SPI_POWER_DOWN_PIN, OUTPUT, PID_GPIO, true);
#endif
}

__INLINE void flash_spi_power_up(void)
{
#ifdef HAS_FLASH_SPI_POWER_DOWN
    GPIO_ConfigurePin( FLASH_SPI_POWER_DOWN_PORT, FLASH_SPI_POWER_DOWN_PIN, OUTPUT, PID_GPIO, false);
#endif
}
        
__INLINE void declare_spi_gpios(void)
{
    declare_flash_spi_power_down_gpios();
    RESERVE_GPIO( SPI_CLK,  FLASH_SPI_CLK_PORT, FLASH_SPI_CLK_PIN, PID_SPI_CLK); 
    RESERVE_GPIO( SPI_DI,   FLASH_SPI_DI_PORT,  FLASH_SPI_DI_PIN,  PID_SPI_DI ); 
    RESERVE_GPIO( SPI_DO,   FLASH_SPI_DO_PORT,  FLASH_SPI_DO_PIN,  PID_SPI_DO ); 
    RESERVE_GPIO( SPI_EN,   FLASH_SPI_CS_PORT,  FLASH_SPI_CS_PIN,  PID_SPI_EN ); 
}

__INLINE void deactivate_spi_flash_gpios(void)
{
    flash_spi_power_down();
    GPIO_SetPinFunction( FLASH_SPI_CS_PORT,  FLASH_SPI_CS_PIN,  INPUT_PULLUP,   PID_GPIO);
    GPIO_SetPinFunction( FLASH_SPI_CLK_PORT, FLASH_SPI_CLK_PIN, INPUT_PULLDOWN, PID_GPIO);
    GPIO_SetPinFunction( FLASH_SPI_DO_PORT,  FLASH_SPI_DO_PIN,  INPUT_PULLDOWN, PID_GPIO);
    GPIO_SetPinFunction( FLASH_SPI_DI_PORT,  FLASH_SPI_DI_PIN,  INPUT_PULLDOWN, PID_GPIO);
}

__INLINE void activate_spi_flash_gpios(void)
{
    flash_spi_power_up();
    GPIO_ConfigurePin( FLASH_SPI_CS_PORT,  FLASH_SPI_CS_PIN,  OUTPUT, PID_SPI_EN,  true  );
    GPIO_ConfigurePin( FLASH_SPI_CLK_PORT, FLASH_SPI_CLK_PIN, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( FLASH_SPI_DO_PORT,  FLASH_SPI_DO_PIN,  OUTPUT, PID_SPI_DO,  false );
    GPIO_ConfigurePin( FLASH_SPI_DI_PORT,  FLASH_SPI_DI_PIN,  INPUT,  PID_SPI_DI,  false );
}

#endif
