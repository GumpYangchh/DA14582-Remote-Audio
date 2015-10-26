/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals setup and initialization. 
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
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "periph_setup.h"
#include "global_io.h"
#include "gpio.h"
#include "uart.h"

#include "app_kbd.h"
#include "app_kbd_key_matrix.h"
#include "app_kbd_scan_fsm.h"
#include "app_kbd_leds.h"

#if (HAS_AUDIO)
#include "app_audio439.h"
#include "spi_439.h"
#endif

#ifdef HAS_SPI_FLASH_STORAGE
#include "app_flash.h"
#endif

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


#if DEVELOPMENT_DEBUG && !defined(GPIO_DRV_PIN_ALLOC_MON_DISABLED)

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
void GPIO_reservations(void)
{
    /*
     * Globally reserved GPIOs reservation
     */
#ifndef FPGA_USED

    // UART GPIOs
#ifdef PROGRAM_ENABLE_UART
    RESERVE_GPIO( UART1_TX, PRODUCTION_DEBUG_PORT, PRODUCTION_DEBUG_PIN, PID_UART1_TX);
    //RESERVE_GPIO( UART1_RX, UART_RX_GPIO_PORT, UART_RX_GPIO_PIN, PID_UART1_RX);    
#endif // PROGRAM_ENABLE_UART

    /*
     * Application specific GPIOs reservation
     */    
    declare_keyboard_gpios();
    declare_audio439_gpios();
    //DECLARE_I2C_GPIOS;
    DECLARE_SPI_GPIOS;
    
#endif // FPGA_USED
}
#endif // DEVELOPMENT_DEBUG && !GPIO_DRV_PIN_ALLOC_MON_DISABLED


/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and GPIO ports (for debugging) are mapped
 *
 * @return void
 ****************************************************************************************
 */
void set_pad_functions(void)
{
#ifndef FPGA_USED
//------------------------UART _DEBUG    
#ifdef PROGRAM_ENABLE_UART
		GPIO_ConfigurePin( PRODUCTION_DEBUG_PORT, PRODUCTION_DEBUG_PIN, OUTPUT, PID_UART1_TX, false);
    // GPIO_ConfigurePin( UART_RX_PORT, UART_RTX_PIN, INPUT, PID_UART1_RX, false );         // PIN 26 on J6
#endif // PROGRAM_ENABLE_UART

    //INIT_I2C_GPIOS;
    deactivate_spi_flash_gpios();
    
#if (HAS_AUDIO)
    init_audio439_gpios(app_audio439_timer_started);
#endif
    
    if (HAS_KEYBOARD_LEDS) {
        // fix LED leakage
        if ( (green_led_st == LED_OFF) || (green_led_st == BLINK_LED_IS_OFF__TURN_ON) ) {
            GPIO_ConfigurePin(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_OFF);
        } else {
            GPIO_ConfigurePin(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_ON);
        }
        if ( (red_led_st == LED_OFF) || (red_led_st == BLINK_LED_IS_OFF__TURN_ON) ) {
            GPIO_ConfigurePin(KBD_RED_LED_PORT, KBD_RED_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_OFF);
        } else {
            GPIO_ConfigurePin(KBD_RED_LED_PORT, KBD_RED_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_ON);
        }
    }
    else if (INIT_LED_PINS) {
        GPIO_ConfigurePin(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_OFF);
        GPIO_ConfigurePin(KBD_RED_LED_PORT, KBD_RED_LED_PIN, OUTPUT, PID_GPIO, LED_STATE_OFF);
    }

    if (current_scan_state == KEY_SCAN_IDLE) {
		app_kbd_reinit_matrix(); 
    }
#else   //FPGA_USED

    RESERVE_GPIO( UART1_TX, GPIO_PORT_0,  GPIO_PIN_0, PID_UART1_TX);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0,  GPIO_PIN_1, PID_UART1_RX);    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, INPUT, PID_UART1_RX, false );    
    
    RESERVE_GPIO( UART1_TX, GPIO_PORT_2, GPIO_PIN_5, PID_UART1_RTSN);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_2, GPIO_PIN_6, PID_UART1_CTSN);      
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_UART1_RTSN, false );//CD SOS
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_6, INPUT_PULLDOWN, PID_UART1_RTSN/*PID_UART1_CTSN*/, false );//FPGA issue?
		
    //ALLOCATE SPI SIGNALS
    RESERVE_GPIO( SPI_EN,  GPIO_PORT_1, GPIO_PIN_0, PID_SPI_EN);
    RESERVE_GPIO( SPI_CLK, GPIO_PORT_0, GPIO_PIN_4, PID_SPI_CLK);
    RESERVE_GPIO( SPI_DO,  GPIO_PORT_0, GPIO_PIN_6, PID_SPI_DO);	
    RESERVE_GPIO( SPI_DI,  GPIO_PORT_0, GPIO_PIN_7, PID_SPI_DI);

    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_SPI_EN,  true  );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_SPI_DO,  false );	
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_SPI_DI,   false );

//    SetWord16(P24_MODE_REG,0x300);
//    SetWord16(P23_MODE_REG,0x300);
//    SetWord16(P2_RESET_DATA_REG,0x18);
#endif
}


/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down.
 *
 *
 * @return void
 ****************************************************************************************
 */
void periph_init(void)
{
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 

    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 1);
	
    // Initialize UART component
#ifdef PROGRAM_ENABLE_UART
    if (GetBits16(CLK_CTRL_REG, RUNNING_AT_XTAL16M)) {
        SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz       
        
        // mode=3-> no parity, 1 stop bit 8 data length
#ifdef UART_MEGABIT
        uart_init(UART_BAUDRATE_1M, 3);
#else
        uart_init(UART_BAUDRATE_115K2, 3);
#endif // UART_MEGABIT
    }
#endif // PROGRAM_ENABLE_UART

    //FPGA  
#ifdef FPGA_USED    
    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);      // enable  clock
    SetBits16(CLK_PER_REG, SPI_DIV, 1);	        // set divider to 1	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
#endif

	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();

#ifndef FPGA_USED
//	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
#endif    

    //SetWord16(P01_PADPWR_CTRL_REG,0xFF);

    // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
