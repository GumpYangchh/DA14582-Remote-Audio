 /**
 ****************************************************************************************
 *
 * @file hw_config.h
 *
 * @brief  Hardware configuration file
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
 
#ifndef _HW_CONFIG_H
#define _HW_CONFIG_H

/*************************************************************************************
 * Extended timers support (of more than 5 min)                                      *
 *************************************************************************************/
#undef EXTENDED_TIMERS_ON

/*************************************************************************************
 * Low power clock selection. Defines whether DA14580 is clocked by XTAL32 or RCX20  *
 * Select between LP_CLK_XTAL32,LP_CLK_RCX20,LP_CLK_FROM_OTP (get from OTP Header)   *
 *************************************************************************************/
#ifdef CFG_LP_CLK 
	#error "Please define CFG_LP_CLK only in hw_config.h!"
#endif
// Defined below per design FG_LP_CLK LP_CLK_XTAL32 


#if (DA14580_RCU || DA14582_RCU)
    #define CFG_LP_CLK  LP_CLK_RCX20
#else
    #define CFG_LP_CLK LP_CLK_XTAL32 
#endif

/*************************************************************************************
 * Define CFG_APP_MOTION to use the motion sensor                                    *
 * BMI055 is connected to the I2C interface. I2C pins must be defined as well        *
 *************************************************************************************/
// Defined below per design #define CFG_APP_MOTION


#if (DA14580_RCU || DA14582_RCU)
    //#define CFG_APP_MOTION
#else
    //#undef CFG_APP_MOTION
#endif


/*************************************************************************************
 * Define CFG_APP_AUDIO to use the audio features                                    *
 * Audio configuration is defined in app_audio439_config.h                           *
 *************************************************************************************/
// Defined below per design #define CFG_APP_AUDIO


#if (DA14580_RCU || DA14582_RCU)
    #define CFG_APP_AUDIO
#else
    #undef CFG_APP_AUDIO
#endif

#if defined(CFG_APP_MOTION) || defined(CFG_APP_AUDIO)
#define CFG_APP_STREAM
#endif

/*************************************************************************************
 * Define HAS_SPI_FLASH_STORAGE if SPI flash is used for storing parameters          *
 * Define SPI_FLASH_IS_2M to set SPI Flash size. 1=2Mbit, 0=1Mbit                    *
 * SPI Flash configuration is defined in app_flash_config.h                          *
 *************************************************************************************/
// Defined below per design #define HAS_SPI_FLASH_STORAGE
// Defined below per design #define SPI_FLASH_IS_2M  1      // Define SPI_FLASH size (0 = 1Mbit, 1 = 2Mbit)

/*************************************************************************************
 * Define HAS_I2C_EEPROM_STORAGE if I2C EEPROM is used for storing parameters        *
 * I2C pins must be defined as well.                                                 *
 *************************************************************************************/
// Defined below per design #define HAS_I2C_EEPROM_STORAGE    
// Defined below per design #define EEPROM_IS_8K          0   // Define EEPROM size (0 = 256 bytes, 1 = 8192 bytes)

/*************************************************************************************
 * Define The base address of the I2C EEPROM or SPI FLASH memory region              *
 * where the data will be stored                                                     *
 *************************************************************************************/
// Defined below per design #define NV_STORAGE_BASE_ADDR 0


    #define UART_TX_PORT       GPIO_PORT_0
    #define UART_TX_PIN        GPIO_PIN_4


#if (DA14580_RCU)
    #define UART_RX_PORT       GPIO_PORT_1
    #define UART_RX_PIN        GPIO_PIN_3

    #define HAS_SPI_FLASH_STORAGE
    #define SPI_FLASH_IS_2M       1   // Define SPI_FLASH size (0 = 1Mbit, 1 = 2Mbit)
    #define NV_STORAGE_BASE_ADDR  (SPI_FLASH_SIZE-SPI_FLASH_SECTOR)
    
    #define I2C_SDA_PORT          GPIO_PORT_2
    #define I2C_SDA_PIN           GPIO_PIN_1
    #define I2C_SCL_PORT          GPIO_PORT_2
    #define I2C_SCL_PIN           GPIO_PIN_0
    
#elif (DA14582_RCU)
    #define UART_RX_PORT       GPIO_PORT_1
    #define UART_RX_PIN        GPIO_PIN_3

    #define HAS_SPI_FLASH_STORAGE
    #define SPI_FLASH_IS_2M       1   // Define SPI_FLASH size (0 = 1Mbit, 1 = 2Mbit)
    #define NV_STORAGE_BASE_ADDR  (SPI_FLASH_SIZE-SPI_FLASH_SECTOR)
    
    #define I2C_SDA_PORT          GPIO_PORT_3
    #define I2C_SDA_PIN           GPIO_PIN_3
    #define I2C_SCL_PORT          GPIO_PORT_2
    #define I2C_SCL_PIN           GPIO_PIN_2

#else
    #define UART_RX_PORT       GPIO_PORT_0
    #define UART_RX_PIN        GPIO_PIN_5

// FIXME fix I2C pins for old kbd_ref designs
// matrix4, 6, 9, 3
    #define HAS_I2C_EEPROM_STORAGE    
    #define EEPROM_IS_8K          0   // Define EEPROM size (0 = 256 bytes, 1 = 8192 bytes)
    #define NV_STORAGE_BASE_ADDR  0

    #define I2C_SDA_PORT	 	  GPIO_PORT_0
    #define I2C_SDA_PIN		 	  GPIO_PIN_6
    #define I2C_SCL_PORT	      GPIO_PORT_0
    #define I2C_SCL_PIN		  	  GPIO_PIN_7

#endif  

#ifdef HAS_I2C_EEPROM_STORAGE

/****************************************************************************************/ 
/* i2c eeprom configuration                                                             */
/****************************************************************************************/ 
#if (EEPROM_IS_8K)
#define I2C_SLAVE_ADDRESS       0x50            // Set slave device address
#define I2C_ADDRESS_MODE        I2C_7BIT_ADDR   // 7-bit addressing
#define I2C_EEPROM_SIZE         8192            // EEPROM size in bytes
#define I2C_EEPROM_PAGE         32              // EEPROM's page size in bytes
#define I2C_SPEED_MODE          I2C_FAST        // fast mode (400 kbits/s)
#define I2C_ADRESS_BYTES_CNT    I2C_2BYTES_ADDR
#else
#define I2C_SLAVE_ADDRESS       0x50            // Set slave device address
#define I2C_ADDRESS_MODE        I2C_7BIT_ADDR   // 7-bit addressing
#define I2C_EEPROM_SIZE         256             // EEPROM size in bytes
#define I2C_EEPROM_PAGE         8               // EEPROM's page size in bytes
#define I2C_SPEED_MODE          I2C_FAST        // fast mode (400 kbits/s)
#define I2C_ADRESS_BYTES_CNT    I2C_1BYTE_ADDR
#endif // EEPROM_IS_8K

#else
// dummy settings so that the compiler does not complain
#define I2C_SLAVE_ADDRESS       0x50            // Set slave device address
#define I2C_ADDRESS_MODE        I2C_7BIT_ADDR   // 7-bit addressing
#define I2C_EEPROM_SIZE         256             // EEPROM size in bytes
#define I2C_EEPROM_PAGE         8               // EEPROM's page size in bytes
#define I2C_SPEED_MODE          I2C_FAST        // fast mode (400 kbits/s)
#define I2C_ADRESS_BYTES_CNT    I2C_1BYTE_ADDR
#endif // HAS_I2C_STORAGE  

#if defined(HAS_I2C_EEPROM_STORAGE) || defined(CFG_APP_MOTION)
#define DECLARE_I2C_GPIOS   \
    {                       \
        RESERVE_GPIO( EEPROM_SDA, I2C_SDA_PORT, I2C_SDA_PIN, PID_I2C_SDA ); 	\
        RESERVE_GPIO( EEPROM_SCL, I2C_SCL_PORT, I2C_SCL_PIN, PID_I2C_SCL ); 	\
    }

#define INIT_I2C_GPIOS   \
    {                       \
        GPIO_SetPinFunction( I2C_SDA_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA ); 	\
        GPIO_SetPinFunction( I2C_SCL_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL ); 	\
    }

#else
#define DECLARE_I2C_GPIOS
#define INIT_I2C_GPIOS
#endif

/// Audio with 439 Application
#if defined(CFG_APP_AUDIO)
#define HAS_AUDIO   1
#else // defined(CFG_APP_AUDIO)
#define HAS_AUDIO   0
#endif // defined(CFG_APP_AUDIO)

/// Audio with 439 Application
#if defined(CFG_APP_MOTION)
#define HAS_BMI055   1
#else // defined(CFG_APP_MOTION)
#define HAS_BMI055   0
#endif // defined(CFG_APP_MOTION)
    
    
#endif	// _HW_CONFIG
