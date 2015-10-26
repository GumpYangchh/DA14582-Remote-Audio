 /**
 ****************************************************************************************
 *
 * @file app_flash.c
 *
 * @brief Additinal routines to access the flash.
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "app_flash.h"
#include "spi_flash.h"
#include "app_utils.h"

#ifdef HAS_SPI_FLASH_STORAGE

bool power_up_delay;

/**
 ****************************************************************************************
 * @brief Initializes the flash pins and the SPI interface
 *
 * @return      void
 ****************************************************************************************
 */
SPI_Pad_t spi_FLASH_CS_Pad;
void app_spi_flash_peripheral_init(void)
{
#ifdef HAS_FLASH_SPI_POWER_DOWN
    // power_up_delay is used to delay the first write operation
    // until SPI Flash is ready after power up.
    power_up_delay=true; 
#endif

    //Initialize the SPI interface and power up the SPI flash if needed
    //in case it is shared with another device on different pins.
    activate_spi_flash_gpios();
        
	spi_FLASH_CS_Pad.pin =  FLASH_SPI_CS_PIN;
    spi_FLASH_CS_Pad.port = FLASH_SPI_CS_PORT;
    // Enable SPI & SPI FLASH
    spi_flash_init(SPI_FLASH_SIZE, SPI_FLASH_PAGE);
    spi_init(&spi_FLASH_CS_Pad, SPI_MODE_8BIT, SPI_ROLE_MASTER, SPI_CLK_IDLE_POL_LOW,
             SPI_PHA_MODE_0, SPI_MINT_DISABLE, SPI_XTAL_DIV_14);

    //The Flash is kept in power_down so we need to power it up
    spi_flash_release_from_power_down();
}

/**
 ****************************************************************************************
 * @brief Releases the flash pins and the SPI interface
 *
 * @return      void
 ****************************************************************************************
 */
void app_spi_flash_peripheral_release(void)
{
    spi_flash_power_down();
    
    deactivate_spi_flash_gpios();
     
#ifdef HAS_FLASH_SPI_POWER_DOWN
    GPIO_ConfigurePin(FLASH_SPI_POWER_DOWN_PORT, FLASH_SPI_POWER_DOWN_PIN, OUTPUT,
                      PID_GPIO, true);
#endif
}

/**
 ****************************************************************************************
 * @brief Simplified write data to a single page in flash. Used to write bonding data
 *
 * @param[in]   data: pointer to the data
 * @param[in]   address: the address in the flash memory
 * @param[in]   size: the amount of data to write
 *
 * @return      Amount of data written
 ****************************************************************************************
 */
size_t app_spi_flash_write_random_page_data(const void *data, uint32_t address,
                                            size_t size)
{
#ifdef HAS_FLASH_SPI_POWER_DOWN
    if (power_up_delay) {
// The flash was off so wait until is ready for write operations        
        power_up_delay=false;
        app_delay(15000);
    }
#endif

    uint8_t cpdata[SPI_FLASH_PAGE];
    int start_address = address & ~(SPI_FLASH_PAGE-1);
    
    // Assumption that the address refers to a single PAGE
    ASSERT_ERROR(start_address == ((address+size-1) & ~(SPI_FLASH_PAGE-1)));
    //attempt to write size more than one page

    int stop_address = address  | (SPI_FLASH_PAGE-1);  
    int size_to_write = (address+size < stop_address) ? size : stop_address-address+1;
    
//read the whole page
    if (spi_flash_read_data (cpdata, start_address, SPI_FLASH_PAGE) < SPI_FLASH_PAGE) {
        return 0;
    }
    memcpy (&cpdata[address-start_address], data, size_to_write);
    if (spi_flash_set_write_enable( )!= ERR_OK) {
        return 0;
    }
    
// erase a whole sector. Only the first page of the sector is used
    if(spi_flash_block_erase(start_address& ~(SPI_FLASH_SECTOR-1),SECTOR_ERASE)!= ERR_OK) {
        return 0;
    }
    
    if (spi_flash_page_program(cpdata, start_address, SPI_FLASH_PAGE) != ERR_OK) {
        return 0;
    }

    if (spi_flash_set_write_disable() != ERR_OK) {
        return 0;
    }

    return size_to_write;
}    

    #if (DEBUG_WRITE_TO_SPI)

/**
 ****************************************************************************************
 * @brief Write debug registers to flash.
 *
 * @param[in]   fault: type of fault that occurred.
 *
 * @return      APP_DBG_FAULT_NONE if writing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_write_dbg_regs(app_dbg_fault_t fault)
{
    const size_t    flag_size = sizeof(app_dbg_fault_t),
                    uint32_size = sizeof(uint32_t);
    uint8_t page[APP_DBG_REGS_SIZE],
            *address = page;
    
    memcpy(address, &fault, flag_size);
    address += flag_size;
    
    for (int i=0; i < APP_DBG_TOTAL_REGS; i++) {
        memcpy(address, app_dbg_registers + i, uint32_size);
        address += uint32_size;
    }
    
    app_spi_flash_peripheral_init();
    if (app_spi_flash_write_random_page_data(page, APP_FLASH_REGS_BASE, APP_DBG_REGS_SIZE)
        < APP_DBG_REGS_SIZE) {
        return APP_DBG_FAULT_FLASH;
    }
    app_spi_flash_peripheral_release();
    
    return APP_DBG_FAULT_NONE;
}

/**
 ****************************************************************************************
 * @brief Read debug registers from flash.
 *
 * @param[in]   void.
 *
 * @return      type of fault that occurred, or APP_DBG_FAULT_FLASH if reading failed.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_read_dbg_regs(void)
{
    const size_t flag_size = sizeof(app_dbg_fault_t);
    const size_t uint32_size = sizeof(uint32_t);

    uint8_t page[APP_DBG_REGS_SIZE], *address = page;

    app_spi_flash_peripheral_init();
    if (spi_flash_read_data(page, APP_FLASH_REGS_BASE, APP_DBG_REGS_SIZE)
        < APP_DBG_REGS_SIZE) {
            return APP_DBG_FAULT_FLASH;
    }
    app_spi_flash_peripheral_release();

    app_dbg_fault_t fault;
    memcpy(&fault, address, flag_size);
    address += flag_size;    
    if (fault == APP_DBG_FAULT_NONE) {
        return APP_DBG_FAULT_NONE;
    }
    
    for (int i=0; i < APP_DBG_TOTAL_REGS; i++) {
        memcpy(app_dbg_registers + i, address, uint32_size);
        address += uint32_size;
    }
    return fault;
}

/**
 ****************************************************************************************
 * @brief Erase debug registers from flash.
 *
 * @param[in]   void.
 *
 * @return      APP_DBG_FAULT_NONE if erasing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_erase_dbg_regs(void)
{
    const size_t flag_size = sizeof(app_dbg_fault_t);
    const app_dbg_fault_t fault = APP_DBG_FAULT_NONE;
    
    app_spi_flash_peripheral_init();
    if (app_spi_flash_write_random_page_data(&fault, APP_FLASH_REGS_BASE, flag_size)
        < flag_size) {
        return APP_DBG_FAULT_FLASH;
    }
    app_spi_flash_peripheral_release();
    
    return APP_DBG_FAULT_NONE;
}

/**
 ****************************************************************************************
 * @brief Write debug message to flash.
 *
 * @param[in]   msg: pointer to the message.
 *
 * @return      APP_DBG_FAULT_NONE if writing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_write_dbg_msg(const char *msg)
{
    int msg_len;
    const size_t len_size = sizeof(int);

    app_spi_flash_peripheral_init();
    if (msg == NULL)
        msg_len = -1;
    else {
        size_t msg_size = strlen(msg) + len_size;
        if (msg_size + APP_DBG_REGS_SIZE > SPI_FLASH_SECTOR) {
            msg_size = SPI_FLASH_SECTOR - APP_DBG_REGS_SIZE;
        }
        uint32_t address = APP_FLASH_BASE;
        msg_len = msg_size;

        while (msg_size > SPI_FLASH_PAGE) {
            if (app_spi_flash_write_random_page_data(msg, address, SPI_FLASH_PAGE)
                < SPI_FLASH_PAGE) {
                return APP_DBG_FAULT_FLASH;
            }
            address += SPI_FLASH_PAGE;
            msg += SPI_FLASH_PAGE;
            msg_size -= SPI_FLASH_PAGE;
        }
        if (app_spi_flash_write_random_page_data(msg, address, msg_size) < msg_size) {
            return APP_DBG_FAULT_FLASH;
        }
    }
    if (app_spi_flash_write_random_page_data(&msg_len, APP_FLASH_REGS_BASE-len_size,
                                             len_size) < len_size) {
        return APP_DBG_FAULT_FLASH;        
    }
    app_spi_flash_peripheral_release();
    
    return APP_DBG_FAULT_NONE;
}

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
app_dbg_fault_t app_spi_flash_read_dbg_msg(char *msg, size_t len)
{
    int msg_len;
    const size_t    len_size = sizeof(int),
                    char_size = sizeof(char);

    app_spi_flash_peripheral_init();
    if (spi_flash_read_data((uint8_t*)&msg_len, APP_FLASH_REGS_BASE-len_size, len_size)
        < len_size) {
        return APP_DBG_FAULT_FLASH;
    }
    if (msg_len == -1) { // Flash contains 0xFF
        *msg = '\0';
    } else {
        if (msg_len + char_size > len) {
            msg_len = len - char_size;
        }
        if (spi_flash_read_data((uint8_t*)msg, APP_FLASH_BASE, msg_len) < msg_len) {
            return APP_DBG_FAULT_FLASH;
        }
        msg[msg_len] = '\0';
    }
    app_spi_flash_peripheral_release();
    
    return APP_DBG_FAULT_NONE;
}

/**
 ****************************************************************************************
 * @brief Erase debug message from flash.
 *
 * @param[in]   void.
 *
 * @return      APP_DBG_FAULT_NONE if erasing succeeded, APP_DBG_FAULT_FLASH else.
 ****************************************************************************************
 */
app_dbg_fault_t app_spi_flash_erase_dbg_msg(void)
{
    return app_spi_flash_write_dbg_msg(NULL);
}

    #endif // (DEBUG_WRITE_TO_SPI)

#endif // HAS_SPI_FLASH_STORAGE

/// @} APP
