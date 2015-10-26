/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
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
 * INCLUDES
 ****************************************************************************************
 */
#include "da14580_scatter_config.h"
#include "arch.h"
#include "arch_sleep.h"
#include <stdlib.h>
#include <stddef.h>     // standard definitions
#include <stdint.h>     // standard integer definition
#include <stdbool.h>    // boolean definition
#include "boot.h"       // boot definition
#include "rwip.h"       // BLE initialization
#include "syscntl.h"    // System control initialization
#include "emi.h"        // EMI initialization
#include "intc.h"       // Interrupt initialization
#include "timer.h"      // TIMER initialization
#include "em_map_ble.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "periph_setup.h"

#if PLF_UART
#include "uart.h"       // UART initialization
#endif //PLF_UART

#include "nvds.h"       // NVDS initialization

#if (BLE_EMB_PRESENT)
#include "rf.h"         // RF initialization
#endif // BLE_EMB_PRESENT


#if (BLE_APP_PRESENT)
#include "app.h"       // application functions
#include "app_sleep.h"
#endif // BLE_APP_PRESENT


#if PLF_DEBUG
#include "dbg.h"       // For dbg_warning function
#endif //PLF_DEBUG

#include "global_io.h"

#include "datasheet.h"

#include "em_map_ble_user.h"
#include "em_map_ble.h"

#include "lld_sleep.h"
#include "rwble.h"
#include "rf_580.h"
#include "gpio.h"

#ifdef USE_POWER_OPTIMIZATIONS
#include "lld_evt.h"
#endif

#ifdef CFG_PRINTF
#include "app_console.h"
#endif

#if (HAS_AUDIO)
#include "app_audio439.h"
#endif
#if (BLE_APP_STREAM)
#include "app_stream.h"
#endif
#if (STREAMDATA_QUEUE)
#include "app_stream_queue.h"
#endif

#if (HAS_BMI055)
#include "app_motion_sensor.h"
#endif

// external function declaration 
void patch_gtl_task(void);

#ifndef __DA14581__   
#ifdef MEM_LEAK_PATCH_ENABLED
    void patch_llc_task(void);
#endif
#endif


/**
 * @addtogroup DRIVERS
 * @{
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/// NVDS location in FLASH
#ifndef __DA14581__
#define NVDS_FLASH_ADDRESS          (0x00000340)
#else
#define NVDS_FLASH_ADDRESS          (0x00000350)
#endif

/// NVDS size in RAM
#define NVDS_FLASH_SIZE             (0x00000100)

#if (DEVELOPMENT_DEBUG)
    #warning "==============================================================> DEVELOPMENT_DEBUG is set!"
    #if (DEEP_SLEEP_ENABLED)
        #warning "The device shall not use Deep Sleep mode when DEVELOPMENT_DEBUG = 1!"
        #warning "Extended Sleep mode shall be used instead!"
    #endif
#endif




/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

#ifdef __DA14581__
uint32_t error;              /// Variable storing the reason of platform reset
#endif



extern uint32_t error;              /// Variable storing the reason of platform reset

extern uint32_t last_temp_time;     /// time of last temperature count measurement
extern uint16_t last_temp_count;    /// temperature counter
extern uint32_t lp_clk_sel;

/// Reserve space for Exchange Memory, this section is linked first in the section "exchange_mem_case"
extern volatile uint8 dummy[];
extern uint8_t func_check_mem_flag;
extern struct lld_sleep_env_tag lld_sleep_env;
extern struct arch_sleep_env_tag sleep_env;


volatile uint8 descript[EM_SYSMEM_SIZE] __attribute__((section("BLE_exchange_memory"), zero_init)); //CASE_15_OFFSET
#if ((EM_SYSMEM_START != EXCHANGE_MEMORY_BASE) || (EM_SYSMEM_SIZE > EXCHANGE_MEMORY_SIZE))
#error("Error in Exhange Memory Definition in the scatter file. Please correct da14580_scatter_config.h settings.");
#endif
bool sys_startup_flag __attribute__((section("retention_mem_area0"), zero_init));
#ifndef __DA14581__
#if (BLE_CONNECTION_MAX_USER > 4)
volatile uint8_t cs_table[EM_BLE_CS_COUNT_USER * REG_BLE_EM_CS_SIZE] __attribute__((section("cs_area"), zero_init));
#endif
#else
#if (BLE_CONNECTION_MAX_USER > 1)
volatile uint8_t cs_table[(BLE_CONNECTION_MAX + 2) * REG_BLE_EM_WPB_SIZE * 2] __attribute__((section("cs_area"), zero_init));
#endif
#endif


/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */


void lld_sleep_init_func(void)
{
    // Clear the environment
    memset(&lld_sleep_env, 0, sizeof(lld_sleep_env));

    // Set wakeup_delay
    set_sleep_delay();
    
    // Enable external wake-up by default
    ble_extwkupdsb_setf(0);
}




/**
 ****************************************************************************************
 * @brief otp_prepare()
 *
 * About: Prepare OTP Controller in order to be able to reload SysRAM at the next power-up
 ****************************************************************************************
 */
static __inline void  otp_prepare(uint32 code_size)
{
    // Enable OPTC clock in order to have access
    SetBits16 (CLK_AMBA_REG, OTP_ENABLE, 1);

    // Wait a little bit to start the OTP clock...
    for(uint8 i=0;i<10;i++); //change this later to a defined time  

    SetBits16(SYS_CTRL_REG, OTP_COPY, 1);

    // Copy the size of software from the first word of the retention mem.
    SetWord32 (OTPC_NWORDS_REG, code_size - 1);

    // And close the OPTC clock to save power
    SetBits16 (CLK_AMBA_REG, OTP_ENABLE, 0);
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * MAIN FUNCTION
 ****************************************************************************************
 */
void set_system_clocks(void);

#ifdef USE_POWER_OPTIMIZATIONS
extern bool fine_hit;
#endif

/**
 ****************************************************************************************
 * @brief BLE main function.
 *
 * This function is called right after the booting process has completed.
 ****************************************************************************************
 */
int main_func(void) __attribute__((noreturn));

int main_func(void)
{
    sleep_mode_t sleep_mode; // keep at system RAM. On each while loop it will get a new value. 
    
    sys_startup_flag = true;
 
    /*
     ************************************************************************************
     * Platform initialization
     ************************************************************************************
     */
#if (USE_WDOG)
    SetWord16(WATCHDOG_REG, 0xC8);          // 200 * 10.24ms = ~2sec active time!
    SetWord16(WATCHDOG_CTRL_REG, 0);        // Generate an NMI when counter reaches 0 and a WDOG (SYS) Reset when it reaches -16!
                                            // WDOG can be frozen by SW!
    SetWord16(RESET_FREEZE_REG, FRZ_WDOG);  // Start WDOG
#else
    SetWord16(SET_FREEZE_REG, FRZ_WDOG);
#endif
    
#if defined(CFG_USE_DEFAULT_XTAL16M_TRIM_VALUE_IF_NOT_CALIBRATED)
#define DEFAULT_XTAL16M_TRIM_VALUE (1302)
    // Apply the default XTAL16 trim value if a trim value has not been programmed in OTP
    if ( 0 == GetWord16(CLK_FREQ_TRIM_REG) )
    {
        set_xtal16m_trim_value(DEFAULT_XTAL16M_TRIM_VALUE);
    }
#endif
    
    set_system_clocks();
    GPIO_init();
    periph_init();
    
          
    /* Don't remove next line otherwhise dummy[0] could be optimized away
     * The dummy array is intended to reserve the needed Exch.Memory space in retention memory
     */
    dummy[0] = dummy[0];
    descript[0] = descript[0];

#ifndef __DA14581__        
# if (BLE_CONNECTION_MAX_USER > 4)
    cs_table[0] = cs_table[0];
# endif

#else //DA14581

# if (BLE_CONNECTION_MAX_USER > 1)
    cs_table[0] = cs_table[0];
# endif
#endif //__DA14581__

    // Initialize random process
    srand(1);

    // Initialize NVDS module
    nvds_init((uint8_t *)NVDS_FLASH_ADDRESS, NVDS_FLASH_SIZE);

    //check and read BDADDR from OTP
    nvds_read_bdaddr_from_otp();

#ifdef RADIO_580
    iq_trim_from_otp();
#endif

    /*
     ************************************************************************************
     * BLE initialization
     ************************************************************************************
     */
    init_pwr_and_clk_ble(); 

    // Initialize BLE stack 
    NVIC_ClearPendingIRQ(BLE_SLP_IRQn);     
    NVIC_ClearPendingIRQ(BLE_EVENT_IRQn); 
    NVIC_ClearPendingIRQ(BLE_RF_DIAG_IRQn);
    NVIC_ClearPendingIRQ(BLE_RX_IRQn);
    NVIC_ClearPendingIRQ(BLE_CRYPT_IRQn);
    NVIC_ClearPendingIRQ(BLE_FINETGTIM_IRQn);	
    NVIC_ClearPendingIRQ(BLE_GROSSTGTIM_IRQn);	
    NVIC_ClearPendingIRQ(BLE_WAKEUP_LP_IRQn);     	
    rwip_init(error);
    
#if ((BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1) && BLE_HOST_PRESENT )
    patch_gtl_task();
#endif // #if (BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1)
    
#ifndef __DA14581__    
#ifdef MEM_LEAK_PATCH_ENABLED
    patch_llc_task();
#endif
#endif

    //Enable the BLE core    
    SetBits32(BLE_RWBTLECNTL_REG, RWBLE_EN, 1); 

#if (USE_TRNG)
    // Initialise random number generator seed using random bits acquired from TRNG
    init_rand_seed_from_trng();
#endif
    
#if RW_BLE_SUPPORT && HCIC_ITF
    // If FW initializes due to FW reset, send the message to Host
    if(error != RESET_NO_ERROR)
    {
        rwble_send_message(error);
    }
#endif

    /*
     ************************************************************************************
     * Sleep mode initializations
     ************************************************************************************
     */
#if (EXT_SLEEP_ENABLED)
     app_set_extended_sleep();
#elif (DEEP_SLEEP_ENABLED)
     app_set_deep_sleep();
#else
     app_disable_sleep();
#endif    

    if ( ((lp_clk_sel == LP_CLK_RCX20) && (CFG_LP_CLK == LP_CLK_FROM_OTP)) || (CFG_LP_CLK == LP_CLK_RCX20) )
    {    
        calibrate_rcx20(20);
        read_rcx_freq(20);  
    }
    
    /*
     ************************************************************************************
     * Application initializations
     ************************************************************************************
     */
#if (BLE_APP_PRESENT)    
    {
        app_init();         // Initialize APP
    }
#endif /* #if (BLE_APP_PRESENT) */

    
    lld_sleep_init_func();
    
    /*
     ************************************************************************************
     * XTAL16M trimming settings
     ************************************************************************************
     */
#ifdef USE_POWER_OPTIMIZATIONS
    SetWord16(TRIM_CTRL_REG, 0x00); // ((0x0 + 1) + (0x0 + 1)) * 250usec but settling time is controlled differently
#else    
    SetWord16(TRIM_CTRL_REG, XTAL16_TRIM_DELAY_SETTING); // ((0xA + 1) + (0x2 + 1)) * 250usec settling time
#endif    
    SetBits16(CLK_16M_REG, XTAL16_CUR_SET, 0x5);
    
    // Enable the TX_EN/RX_EN interrupts, depending on the RF mode of operation (PLL-LUT and MGC_KMODALPHA combinations)
    enable_rf_diag_irq(RF_DIAG_IRQ_MODE_RXTX); 

#if BLE_APP_SPOTAR
    //app_spotar_exec_patch();
#endif
	
    /*
     ************************************************************************************
     * Watchdog
     ************************************************************************************
     */
#if (USE_WDOG)
    SetWord16(WATCHDOG_REG, 0xC8);          // 200 * 10.24ms active time for initialization!
    SetWord16(RESET_FREEZE_REG, FRZ_WDOG);  // Start WDOG
#endif

#if (STREAMDATA_QUEUE)
    stream_fifo_init ();
#endif    
    
#if (BLE_APP_STREAM)
    app_stream_init();
#endif

    /*
     ************************************************************************************
     * Main loop
     ************************************************************************************
     */
    while(1)
    {   
		// schedule all pending events
		if(GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 1) { // BLE clock is enabled
			if(GetBits32(BLE_DEEPSLCNTL_REG, DEEP_SLEEP_STAT) == 0 && !(rwip_prevent_sleep_get() & RW_WAKE_UP_ONGOING)) { // BLE is running

                uint8_t ble_evt_end_set = ke_event_get(KE_EVENT_BLE_EVT_END); // BLE event end is set. conditional RF calibration can run.
                
                rwip_schedule();  
   
                if (ble_evt_end_set)
                {
                    uint32_t sleep_duration = 0;
                    
                    if ( ((lp_clk_sel == LP_CLK_RCX20) && (CFG_LP_CLK == LP_CLK_FROM_OTP)) || (CFG_LP_CLK == LP_CLK_RCX20) )
                        read_rcx_freq(20);
                    
                    if (lld_sleep_check(&sleep_duration, 4)) //6 slots -> 3.750 ms
                        conditionally_run_radio_cals(); // check time and temperature to run radio calibrations. 
                }
                
#if (BLE_APP_PRESENT)
				if ( app_asynch_trm() )
					continue; // so that rwip_schedule() is called again
#endif
                
#ifdef CFG_PRINTF
                {
                    arch_printf_process();
                }
#endif                
			}	
		} 
        
#if (BLE_APP_PRESENT)
		// asynchronous events processing
		if (app_asynch_proc())
			continue; // so that rwip_schedule() is called again
#endif

#if (BLE_APP_STREAM)
    #if (HAS_AUDIO)
    app_audio439_encode(8);
    #endif
#endif
#if (STREAMDATA_QUEUE || BLE_APP_STREAM)     
    #if (HAS_AUDIO)        
        if (stream_queue_more_data( ))
            continue;
    #endif
#endif
        
#if (!BLE_APP_PRESENT)
        if (check_gtl_state())
#endif
        {
            GLOBAL_INT_STOP();

#if (BLE_APP_PRESENT)
            app_asynch_sleep_proc();
#endif        

//            // set wake-up delay only for RCX (to cover small frequency shifts due to temerature variation)
//            if (lp_clk_sel == LP_CLK_RCX20)
//                set_sleep_delay();
        
            // if app has turned sleep off, rwip_sleep() will act accordingly
            // time from rwip_sleep() to WFI() must be kept as short as possible!
            sleep_mode = mode_active;
        
            // BLE is sleeping ==> app defines the mode
            if (sleep_mode == mode_sleeping) {
                if (sleep_env.slp_state == ARCH_EXT_SLEEP_ON) {
                    sleep_mode = mode_ext_sleep;
                } else {
                    sleep_mode = mode_deep_sleep;
                }
            }
            
            if (sleep_mode == mode_ext_sleep || sleep_mode == mode_deep_sleep) 
            {
                SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1); // turn off radio
                
                if (jump_table_struct[nb_links_user] > 1)
                {
                    if( (sleep_mode == mode_deep_sleep) && func_check_mem() && test_rxdone() && ke_mem_is_empty(KE_MEM_NON_RETENTION) )
                    {
                        func_check_mem_flag = 2;//true;
                    }
                    else
                        sleep_mode = mode_ext_sleep;
                }
                else
                {
                    if( (sleep_mode == mode_deep_sleep) && ke_mem_is_empty(KE_MEM_NON_RETENTION) )
                    {
                        func_check_mem_flag = 1;//true;
                    }
                    else
                        sleep_mode = mode_ext_sleep;
                }
                
#if (BLE_APP_PRESENT)
                // hook for app specific tasks when preparing sleeping
                app_sleep_prepare_proc(&sleep_mode);
#endif
                
                
                if (sleep_mode == mode_ext_sleep || sleep_mode == mode_deep_sleep)
                {
                    SCB->SCR |= 1<<2; // enable sleepdeep mode bit in System Control Register (SCR[2]=SLEEPDEEP)
                    
                    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);           // activate PAD latches
                    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);           // turn off peripheral power domain
                    if (sleep_mode == mode_ext_sleep)	{
                        SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM
                        SetBits16(SYS_CTRL_REG, OTP_COPY, 0);           // disable OTP copy	  
                    } else { // mode_deep_sleep
#if DEVELOPMENT_DEBUG
                        SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM		
#else
                        SetBits16(SYS_CTRL_REG, RET_SYSRAM, 0);         // turn System RAM off => all data will be lost!
#endif
                        otp_prepare(0x1FC0);                            // this is 0x1FC0 32 bits words, so 0x7F00 bytes 
                    }
                }

#ifdef USE_POWER_OPTIMIZATIONS
                fine_hit = false;
#endif
                
#if (BLE_APP_PRESENT)
                // hook for app specific tasks just before sleeping
                app_sleep_entry_proc(&sleep_mode);
#endif

#if ((EXTERNAL_WAKEUP) && (!BLE_APP_PRESENT)) // external wake up, only in external processor designs
                ext_wakeup_enable(EXTERNAL_WAKEUP_GPIO_PORT, EXTERNAL_WAKEUP_GPIO_PIN, EXTERNAL_WAKEUP_GPIO_POLARITY);
#endif

                if ( (sleep_mode == mode_ext_sleep) || (sleep_mode == mode_deep_sleep) )
                {
                    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 0);      // Set BIAS to '0' if sleep has been decided
                    
#ifdef USE_POWER_OPTIMIZATIONS
                    clk_freq_trim_reg_value = GetWord16(CLK_FREQ_TRIM_REG); // store used trim value
                    
                    SetBits16(CLK_16M_REG, RC16M_ENABLE, 1);                // Enable RC16
                    for (volatile int i = 0; i < 20; i++);

                    SetBits16(CLK_CTRL_REG, SYS_CLK_SEL, 1);                // Switch to RC16
                    while( (GetWord16(CLK_CTRL_REG) & RUNNING_AT_RC16M) == 0 );   

                    // Do not disable XTAL16M! It will be disabled when we sleep...
                    
                    SetWord16(CLK_FREQ_TRIM_REG, 0x0000);                   // Set zero value to CLK_FREQ_TRIM_REG
#endif
                }

                WFI();

#if (BLE_APP_PRESENT)            
                // hook for app specific tasks just after waking up
                app_sleep_exit_proc(sleep_mode);
#endif

#if ((EXTERNAL_WAKEUP) && (!BLE_APP_PRESENT)) // external wake up, only in external processor designs
                // Disable external wakeup interrupt
                ext_wakeup_disable();
#endif

                // reset SCR[2]=SLEEPDEEP bit else the mode=idle WFI will cause a deep sleep 
                // instead of a processor halt
                SCB->SCR &= ~(1<<2);
            }
            else if (sleep_mode == mode_idle) 
            {
#if (!BLE_APP_PRESENT)              
                if (check_gtl_state())
#endif
                {
                    WFI();    
                }
            }
            
            // restore interrupts
            GLOBAL_INT_START();
        }
        
#if (USE_WDOG)        
        SetWord16(WATCHDOG_REG, 0xC8);          // Reset WDOG! 200 * 10.24ms active time for normal mode!
#endif
    }
}

/// @} DRIVERS
