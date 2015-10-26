/**
 ****************************************************************************************
 *
 * @file app_con_fsm_config.h
 *
 * @brief Connection FSM configuration header file.
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

#ifndef APP_COM_FSM_CONFIG_H_
#define APP_COM_FSM_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>


/****************************************************************************************
 * Define the device name used in the advertisment data during advertising              *
 ****************************************************************************************/

#undef APP_DFLT_DEVICE_NAME 
#if (DA14580_RCU)
#define APP_DFLT_DEVICE_NAME    "DA14580 M&VRemote"
#elif (DA14582_RCU)
#define APP_DFLT_DEVICE_NAME    "DA14582 M&VRemote"
#endif

/*
 * Advertising data
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x19 - Appearance
 * x80\x01 - Remote control
 * --------------------------------------------------------------------------------------
 * x09 - Length
 * x02 - Incomplete list of 16-bit UUIDs available
 * x12\x18 - Human Interface Device Service UUID
 * x0F\x18 - Battery Service UUID
 * x0A\x18 - Device Information Service UUID
 * --------------------------------------------------------------------------------------
 * x05 - Length
 * x16 - Service Data
 * x0A\x18 - Device Information Service UUID
 * x00\x01 - Service data
 */
 
#if defined(CFG_PRF_SPOTAR)
#define APP_ADV_DATA            "\x03\x19\x80\x01\x09\x02\x12\x18\x0F\x18\x0A\x18\xF5\xFE\x05\x16\x0A\x18\x00\x01"
#define APP_ADV_DATA_LENGTH     (20)
#else
#define APP_ADV_DATA            "\x03\x19\x80\x01\x07\x02\x12\x18\x0F\x18\x0A\x18\x05\x16\x0A\x18\x00\x01"
#define APP_ADV_DATA_LENGTH     (18)
#endif

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
 
#define APP_SCNRSP_DATA         "\x00"
#define APP_SCNRSP_DATA_LENGTH  (0)


/****************************************************************************************
 * Load all bond info into RetRAM at power-up to eliminate subsequent                   *
 * Reads and reduce power consumption                                                   *
 ****************************************************************************************/
#define MBOND_LOAD_INFO_AT_INIT 0                   // 0, do not load - 1, do load

#define MAX_BOND_PEER		  (0x03)		// Maximum number of bonds

#define NV_STORAGE_BOND_SIZE  (256)			// NV_STORAGE_BOND_SIZE = (1 + no_of_bonds + no_of_bonds * 40) rounded up to the next multiple of 32
											// If SPI FLASH memory is used NV_STORAGE_BOND_SIZE must be <=256
											// i.e. for no_of_bonds = 6, 1+6+6*40 = 247 ==> NV_STORAGE_BOND_SIZE = 256

#define BOND_INFO_DEFAULT_VAL   (0xA1)

/****************************************************************************************
 * If Storage (I2C EEPROM or SPI FLASH) pins are used for keyboard                      *
 * reinitialize them when not needed any more                                           *
 ****************************************************************************************/
#define INIT_STORAGE_PINS    0

    
enum con_fsm_state_update_callback_type {
    PREPARE_DEEPSLEEP,
    REINITIALIZE,
    INDICATE_CONNECTED,
    INDICATE_CONNECTION_IN_PROGRESS,
    INDICATE_DISCONNECTED,
    INDICATE_OFF,
    START_PASSCODE
};

typedef void (con_fsm_state_update_callback_t)(enum con_fsm_state_update_callback_type type);
typedef void (con_fsm_attr_update_callback_t)(int pos, int attr_num, int value);

void app_kbd_con_fsm_callback(enum con_fsm_state_update_callback_type type);
void app_kbd_attr_update_callback(int pos, int attr_num, int value);

typedef struct {
    bool has_multi_bond;
    bool has_deepsleep;
    bool has_privacy;
    bool use_pref_conn_params;
    bool is_normally_connectable;
    bool has_passcode_timeout;
    bool has_inactivity_timeout;
    bool has_mitm;
    bool has_white_list;
    bool has_virtual_white_list;
    bool has_security_request_send;
    bool has_send_ll_terminate_ind; 
    bool has_usage_counters;
    bool has_nv_rom;
    uint32_t unbonded_discoverable_timeout;
    uint32_t bonded_discoverable_timeout;  
    uint32_t enc_safeguard_timeout;            
    uint32_t kbd_passcode_timeout;             
    uint32_t kbd_inactivity_timeout;           
    uint16_t time_to_request_param_upd;        
    uint16_t alt_pair_disconn_time;            
    uint16_t normal_adv_int_min;              
    uint16_t normal_adv_int_max;               
    uint16_t fast_bonded_adv_int_min;         
    uint16_t fast_bonded_adv_int_max;          
    uint16_t slow_bonded_adv_int_min;          
    uint16_t slow_bonded_adv_int_max;          
    uint16_t preferred_conn_interval_min;              
    uint16_t preferred_conn_interval_max;              
    uint16_t preferred_conn_latency;                 
    uint16_t preferred_conn_timeout;                   
    con_fsm_state_update_callback_t *state_update_callback; 
    con_fsm_attr_update_callback_t *attr_update_callback;
} con_fsm_params_t;

static const con_fsm_params_t con_fsm_params={
    .has_multi_bond              = true,    //  Bond to multiple hosts 
    .has_deepsleep               = false,   //  Use Deep Sleep when in IDLE_ST
    .has_privacy                 = false,   //  Use Privacy (implementation pending) 
    .use_pref_conn_params        = true,    //  Send a ConnUpdateParam request after connection completion  
    .is_normally_connectable     = false,   //  Set NormallyConnectable mode ON
    .has_passcode_timeout        = true,    //  Enable timeout checking during PassCode entry
    .has_inactivity_timeout      = true,    //  Enable disconnection after a pre-defined inactivity timeout
    .has_mitm                    = false,   //  Use MITM authentication mode   
    .has_nv_rom                  = true,    //  Use Non volatile ROM to store bonding data
        
///****************************************************************************************
// * Use White List when exiting DIRECTED_ADV_ST unsuccessfully.                          *
// * Note 1: if White List is used, then the Device will be able to bond only to 1 Master!*
// *         This can be easily modified according to application's usage scenario.       *
// * Note 2: VIRTUAL_WHITE_LIST_ON must NOT be defined! Only 1 White List can exist at a  *
// *         time!                                                                        *
// ****************************************************************************************/
    .has_white_list              = false,
        
///****************************************************************************************
// * Use a Virtual White List to support also hosts with Resolvable Random Addresses.     *
// * All addresses are filtered by SW and not by HW!                                      *
// * Note 1: if White List is used, then the Device will be able to bond only to 1 Master!*
// *         This can be easily modified according to application's usage scenario.       *
// * Note 2: WHITE_LIST_ON MUST NOT be defined! Only 1 White List can exist at a time!    *
// * Note 3: Bonding and Encryption MUST be enabled (bonding info is stored in a          *
// *         non-volatile storage area).                                                  *
// ****************************************************************************************/
    .has_virtual_white_list      = true,    
        
///****************************************************************************************
// * Send a SECURITY_REQUEST when connecting to a Host. If the Host is using a resolvable *
// * random address then check whether it is a known Host (i.e. its address can be        *
// * resolved). If it is a known Host then do not send the SECURITY_REQUEST message.      *
// ****************************************************************************************/
    .has_security_request_send   = true,
        
///****************************************************************************************
// * Enable sending of LL_TERMINATE_IND when dropping a connection                        *
// * Note: undefining this switch gives the option to silently drop a connection. The     *
// *       Host will notice it as if the device is lost or experienced a problem.         *
// ****************************************************************************************/
    .has_send_ll_terminate_ind   = true,
        
///****************************************************************************************
// * Use usage counters                                                                   *
// * Usage counters are used during the bonding of a new host in order to determine       *
// * the oldest entry in the NV RAM that will be used for storing the bonding data        *
// * of the new host                                                                      *
// ****************************************************************************************/
    .has_usage_counters          = false,
    
/****************************************************************************************
 * Timeouts                                                                             *
 ****************************************************************************************/
// Time in Limited Discoverable mode in ADVERTISE_ST:UNBONDED in msec           (when not is_normally_connectable)
    .unbonded_discoverable_timeout  = 180000,
// Time in Limited Discoverable mode in ADVERTISE_ST:BONDED in msec          
    .bonded_discoverable_timeout    = 180000,
// Time to wait for a PAIRING_REQ or ENC_REQ from the Host when BONDING is enabled in msec
    .enc_safeguard_timeout          = 5000,          
// Time in CONNECTED_NO_PAIR_ST until passcode is entered in msec               (when has_passcode_timeout)
    .kbd_passcode_timeout           = 60000,       
// Idle time in CONNECTED_ST until disconnection is requested in msec           (when has_inactivity_timeout)
    .kbd_inactivity_timeout         = 300000,     
// Time to request update of connection parameters in msec                      (when use_pref_conn_params )
    .time_to_request_param_upd      = 5000,       
// Time to block previous host during a "host-switch" in 10msec
    .alt_pair_disconn_time          = 6000,       
// ADVERTISE_ST:UNBONDED : minimum advertising interval (* 0.625ms)
    .normal_adv_int_min             = 0x30,      
// ADVERTISE_ST:UNBONDED : maximum advertising interval (* 0.625ms)(+ pseudo random advDelay from 0 to 10msec)
    .normal_adv_int_max             = 0x40,     
// ADVERTISE_ST:BONDED : minimum advertising interval   (* 0.625ms)(+ pseudo random advDelay from 0 to 10msec)
    .fast_bonded_adv_int_min        = 0x20,     
// ADVERTISE_ST:BONDED : maximum advertising interval   (* 0.625ms)(+ pseudo random advDelay from 0 to 10msec)
    .fast_bonded_adv_int_max        = 0x20,    
// ADVERTISE_ST:SLOW : minimum advertising interval     (* 0.625ms)(+ pseudo random advDelay from 0 to 10msec) (when not is_normally_connectable)
    .slow_bonded_adv_int_min        = 0x640,     
// ADVERTISE_ST:SLOW : maximum advertising interval     (* 0.625ms)(+ pseudo random advDelay from 0 to 10msec) (when not is_normally_connectable)
    .slow_bonded_adv_int_max        = 0xFA0,     
    
/****************************************************************************************
 * Prefered connection parameters                                                       *
 ****************************************************************************************/
    .preferred_conn_interval_min  = 6,         // N * 1.25ms
    .preferred_conn_interval_max  = 6,         // N * 1.25ms
    .preferred_conn_latency       = 31,        // Connection Events skipped
    .preferred_conn_timeout       = 200,       // N * 10ms        

/****************************************************************************************
 * Callback functions                                                                   *
 ****************************************************************************************/
// The callback function that is called when the connection state has changed to 
// connected, connection in progress, disconnected, off, passcode entry started
    .state_update_callback=app_kbd_con_fsm_callback,
    
// The callback function that is called when bonding data are read from the non-volatile  
// memory so that the service database is updated
    .attr_update_callback=app_kbd_attr_update_callback
};
    
#endif // APP_COM_FSM_CONFIG_H_
