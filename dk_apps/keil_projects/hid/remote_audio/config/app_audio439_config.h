 /**
 ****************************************************************************************
 *
 * @file app_audio439_config.h
 *
 * @brief  Audio439 application configuration header file
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
 
#ifndef _APP_AUDIO439_CONFIG_H
#define _APP_AUDIO439_CONFIG_H

#ifdef CFG_APP_AUDIO

#define CFG_SPI_439

/*************************************************************************************
 * Define CFG_SPI_439_BLOCK_BASED to use to allow reading blocks of 40 samples       *
 * from DA14439. If not defined samples are read one at a time                       *
 *************************************************************************************/
#define CFG_SPI_439_BLOCK_BASED


/*************************************************************************************
 * Define CFG_AUDIO439_ADAPTIVE_RATE to enable dynamic                               *
 * sampling rate and ADPCM configuration                                             *
 *************************************************************************************/
#undef CFG_AUDIO439_ADAPTIVE_RATE

/*************************************************************************************
 * Define CFG_AUDIO439_IMA_ADPCM to use IMA ADPCM encoding.                          *
 * If not defined aLaw encoding will be used                                        *
 *************************************************************************************/
#define CFG_AUDIO439_IMA_ADPCM

/*************************************************************************************
 * 0: 64 Kbit/s = ima 4Bps, 16 Khz.                                                  *
 * 1: 48 Kbit/s = ima 3Bps, 16 Khz.                                                  *
 * 2: 32 Kbit/s = ima 4Bps, 8 Khz (downsample).                                      *
 * 3: 24 Kbit/s = ima 3Bps, 8 Khz (downsample).                                      *
 * do not use enum. Enums are not recognized by the preprocessor                     *
 *************************************************************************************/
#define IMA_DEFAULT_MODE 0      


/*************************************************************************************
 * Define HAS_AUDIO_MUTE to use a GPIO pin to control DA14439 power supply           *
 * AUDIO_MUTE_PORT and AUDIO_MUTE_PIN, AUDIO_MUTE_POLARITY must be defined           *
 *************************************************************************************/
#define HAS_AUDIO_MUTE                 


/*************************************************************************************
 * Define HAS_AUDIO_VDDIO_CONTROL to use a separate GPIO pin to control DA14439 	 *
 * VDDIO power   																	 *
 * AUDIO_VDDIO_CONTROL_PORT, AUDIO_VDDIO_CONTROL_PIN, AUDIO_VDDIO_CONTROL_POLARITY   *
 *  must be defined                                                                  *
 *************************************************************************************/
// Defined below per design #define HAS_AUDIO_VDDIO_CONTROL


/*************************************************************************************
 * Define HAS_AUDIO_SOFT_START to use a pulse waveform                               *
 * in order to soft start VDD                                                        *
 *************************************************************************************/
#undef HAS_AUDIO_SOFT_START


/*************************************************************************************
 * If a GPIO pin is used to control DA14439 power VDD_TURN_ON_DELAY defines          *
 * the delay between the activation of the pin until VDD power is stable             *
 *************************************************************************************/
#define VDD_TURN_ON_DELAY   1000    

/*************************************************************************************
 * Define the gain of the SC14439 microphone amplifier. 
 * Refer to SC14439 datasheet for additional information                             *
 *************************************************************************************/
// Defined below per design #define AUDIO_MIC_AMP_GAIN 0x1041

#if (DA14580_RCU)
    #define AUDIO_MIC_AMP_GAIN 0x1081
#elif (DA14582_RCU)
    #define AUDIO_MIC_AMP_GAIN 0x1041
#endif

/*************************************************************************************
 * pin configuration                                                                 *
 *************************************************************************************/

#define AUDIO_CLK_PORT GPIO_PORT_0 
#define AUDIO_CLK_PIN  GPIO_PIN_5 

#if (DA14580_RCU)
    #define AUDIO_SPI_EN_PORT   GPIO_PORT_0
    #define AUDIO_SPI_EN_PIN    GPIO_PIN_3
    #define AUDIO_SPI_CLK_PORT  GPIO_PORT_0
    #define AUDIO_SPI_CLK_PIN   GPIO_PIN_0
    #define AUDIO_SPI_DO_PORT   GPIO_PORT_0
    #define AUDIO_SPI_DO_PIN    GPIO_PIN_1
    #define AUDIO_SPI_DI_PORT   GPIO_PORT_0
    #define AUDIO_SPI_DI_PIN    GPIO_PIN_2    

    #define AUDIO_MUTE_PORT      GPIO_PORT_2
    #define AUDIO_MUTE_PIN       GPIO_PIN_2
    #define AUDIO_MUTE_POLARITY  GPIO_ACTIVE_LOW

#elif (DA14582_RCU)
    #define AUDIO_SPI_EN_PORT   GPIO_PORT_2
    #define AUDIO_SPI_EN_PIN    GPIO_PIN_1
    #define AUDIO_SPI_CLK_PORT  GPIO_PORT_3
    #define AUDIO_SPI_CLK_PIN   GPIO_PIN_1
    #define AUDIO_SPI_DO_PORT   GPIO_PORT_0
    #define AUDIO_SPI_DO_PIN    GPIO_PIN_4
    #define AUDIO_SPI_DI_PORT   GPIO_PORT_3
    #define AUDIO_SPI_DI_PIN    GPIO_PIN_2    

    #define HAS_AUDIO_VDDIO_CONTROL

    #define AUDIO_VDDIO_CONTROL_PORT      GPIO_PORT_0
    #define AUDIO_VDDIO_CONTROL_PIN       GPIO_PIN_6
    #define AUDIO_VDDIO_CONTROL_POLARITY  GPIO_ACTIVE_LOW

    #define AUDIO_MUTE_PORT          GPIO_PORT_0
    #define AUDIO_MUTE_PIN           GPIO_PIN_7
    #define AUDIO_MUTE_POLARITY      GPIO_ACTIVE_HIGH

#endif

#if defined(HAS_AUDIO_MUTE) && (!defined(AUDIO_MUTE_PORT) || !defined(AUDIO_MUTE_PIN) || !defined(AUDIO_MUTE_POLARITY))
    #error "AUDIO_MUTE_PORT and AUDIO_MUTE_PIN must be defined"
#endif

#if defined(HAS_AUDIO_VDDIO_CONTROL) && (!defined(AUDIO_VDDIO_CONTROL_PORT) || !defined(AUDIO_VDDIO_CONTROL_PIN) || !defined(AUDIO_VDDIO_CONTROL_POLARITY))
    #error "AUDIO_VDDIO_CONTROL_PORT and AUDIO_VDDIO_CONTROL_PIN must be defined"
#endif


#define GPIO_ACTIVE_HIGH 1
#define GPIO_ACTIVE_LOW  0

#endif  // CFG_APP_AUDIO    
    
#endif	// _APP_AUDIO439_CONFIG_H
