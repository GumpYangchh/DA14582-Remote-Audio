 /**
 ****************************************************************************************
 *
 * @file app_flash_config.h
 *
 * @brief  SPI Flash configuration file
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
 
#ifndef _APP_FLASH_CONFIG_H
#define _APP_FLASH_CONFIG_H

#ifdef HAS_SPI_FLASH_STORAGE

/*************************************************************************************
 * Define HAS_FLASH_SPI_POWER_DOWN to use GPIO a pin to control SPI Flash power      *
 * FLASH_SPI_POWER_DOWN_PORT and FLASH_SPI_POWER_DOWN_PIN must be defined as well    *
 *************************************************************************************/
// Defined below per design #define HAS_FLASH_SPI_POWER_DOWN
// Defined below per design #define FLASH_SPI_POWER_DOWN_PORT GPIO_PORT_x
// Defined below per design #define FLASH_SPI_POWER_DOWN_PIN  GPIO_PIN_x

/*************************************************************************************
 * pin configuration                                                                 *
 *************************************************************************************/

#if (DA14582_RCU)
    #define FLASH_SPI_CLK_PORT  GPIO_PORT_0
    #define FLASH_SPI_CLK_PIN   GPIO_PIN_3
    #define FLASH_SPI_CS_PORT   GPIO_PORT_0
    #define FLASH_SPI_CS_PIN    GPIO_PIN_1
    #define FLASH_SPI_DI_PORT   GPIO_PORT_0
    #define FLASH_SPI_DI_PIN    GPIO_PIN_0
    #define FLASH_SPI_DO_PORT   GPIO_PORT_3
    #define FLASH_SPI_DO_PIN    GPIO_PIN_0   

#elif (DA14580_RCU)
    #define HAS_FLASH_SPI_POWER_DOWN
    #define FLASH_SPI_POWER_DOWN_PORT GPIO_PORT_1
    #define FLASH_SPI_POWER_DOWN_PIN  GPIO_PIN_2
    
    #define FLASH_SPI_CLK_PORT  GPIO_PORT_2
    #define FLASH_SPI_CLK_PIN    GPIO_PIN_6
    #define FLASH_SPI_CS_PORT   GPIO_PORT_2
    #define FLASH_SPI_CS_PIN     GPIO_PIN_5
    #define FLASH_SPI_DI_PORT   GPIO_PORT_2
    #define FLASH_SPI_DI_PIN     GPIO_PIN_4
    #define FLASH_SPI_DO_PORT   GPIO_PORT_2
    #define FLASH_SPI_DO_PIN     GPIO_PIN_3  

#endif  

#if defined(HAS_FLASH_SPI_POWER_DOWN) && (!defined(FLASH_SPI_POWER_DOWN_PORT) || !defined(FLASH_SPI_POWER_DOWN_PIN))
#error "AUDIO_MUTE_PORT and AUDIO_MUTE_PIN must be defined"
#endif

#if !defined(SPI_FLASH_IS_2M)
#error "SPI Flash size must be defined using by defining SPI_FLASH_IS_2M"
#endif

#if !defined(SPI_FLASH_IS_2M)
#error "SPI Flash size must be defined using by defining SPI_FLASH_IS_2M"
#endif

/****************************************************************************************/ 
/* SPI FLASH configuration                                                              */
/****************************************************************************************/
// SPI Flash options
// SPI flash must be compatible with Winbond W25X10 or W25X20 parts
// SPI Flash sector size is 4KB, page size is 256 bytes

#if SPI_FLASH_IS_2M
#define SPI_FLASH_SIZE      0x40000
#else
#define SPI_FLASH_SIZE      0x20000
#endif

#define SPI_FLASH_PAGE_SIZE 0x100
#define SPI_FLASH_PAGE      SPI_FLASH_PAGE_SIZE   // SPI Flash memory page size in bytes
#define SPI_FLASH_SECTOR    0x1000

#else   // HAS_SPI_FLASH_STORAGE
        
// dummy settings so that the compiler does not complain
#define SPI_FLASH_PAGE_SIZE 0x100
#define SPI_FLASH_PAGE      SPI_FLASH_PAGE_SIZE   // SPI Flash memory page size in bytes
#define SPI_FLASH_SECTOR    0x1000
#define SPI_FLASH_SIZE      0x20000
    
#endif  // HAS_SPI_FLASH_STORAGE
    
#endif	// _APP_FLASH_CONFIG_H
