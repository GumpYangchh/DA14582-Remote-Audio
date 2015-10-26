/**
 ****************************************************************************************
 *
 * @file app_kbd_proj_task.c
 *
 * @brief HID Keyboard handlers.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
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

#include "rwip_config.h"               // SW configuration

#if (BLE_APP_PRESENT)

#include "app_task.h"                  // Application Task API
#include "app_kbd.h"
#include "app_kbd_hid_sensor.h"


#define REPORT_MAP_LEN sizeof(report_map)

// Report Descriptor == Report Map (HID1_11.pdf section E.6)
KBD_TYPE_QUALIFIER uint8 report_map[] KBD_ARRAY_ATTRIBUTE =
	{
		HID_USAGE_PAGE    (HID_USAGE_PAGE_GENERIC_DESKTOP), 
		HID_USAGE         (HID_GEN_DESKTOP_USAGE_KEYBOARD), 
		HID_COLLECTION    (HID_APPLICATION),                
        HID_REPORT_ID     (0x01),                           
		HID_USAGE_PAGE    (HID_USAGE_PAGE_KEY_CODES),   
		HID_USAGE_MIN_8   (0xE0),                       
		HID_USAGE_MAX_8   (0xE7),                       
		HID_LOGICAL_MIN_8 (0x00),                       
		HID_LOGICAL_MAX_8 (0x01),                       
		HID_REPORT_SIZE   (0x01),                       
		HID_REPORT_COUNT  (0x08),                       
		HID_INPUT         (HID_DATA_BIT | HID_VAR_BIT | HID_ABS_BIT), //  Input: (Data, Variable, Absolute) ; Modifier byte
		HID_REPORT_COUNT  (0x01),                       
		HID_REPORT_SIZE   (0x08),                       
		HID_INPUT         (HID_CONST_BIT),              //  Input: (Constant) ; Reserved byte
        #if 1		//LED DEFINITION - Kept for compatibility with keyboard reference design 
		HID_REPORT_COUNT  (0x05),                      
		HID_REPORT_SIZE   (0x01),                      
		HID_USAGE_PAGE    (HID_USAGE_PAGE_LEDS),       
		HID_USAGE_MIN_8   (0x01),                      
		HID_USAGE_MAX_8   (0x05),                      
		HID_OUTPUT        (HID_DATA_BIT | HID_VAR_BIT | HID_ABS_BIT), //  Output: (Data, Variable, Absolute) ; LED report
		HID_REPORT_COUNT  (0x01),                       
		HID_REPORT_SIZE   (0x03),                       
		HID_OUTPUT        (HID_CONST_BIT),                       //  Output: (Constant); LED report padding
#endif              
		HID_REPORT_COUNT  (0x06),                       
		HID_REPORT_SIZE   (0x08),                       
		HID_LOGICAL_MIN_8 (0x00),                       
		HID_LOGICAL_MAX_8 (0x65),                       
		HID_USAGE_PAGE    (HID_USAGE_PAGE_KEY_CODES),   
		HID_USAGE_MIN_8   (0x00),                       
		HID_USAGE_MAX_8   (0x65),                       
		HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT), //  Input: (Data, Array) ; Key arrays (6 bytes)
		HID_END_COLLECTION,                             
        HID_USAGE_PAGE    (HID_USAGE_PAGE_CONSUMER),         
        HID_USAGE         (HID_CONSUMER_USAGE_CONSUMER_CONTROL), 
        HID_COLLECTION    (HID_APPLICATION),    
        HID_REPORT_ID     (0x03),               
        HID_LOGICAL_MIN_8 (0x00),               
        HID_LOGICAL_MAX_8 (0x01),               
        HID_REPORT_SIZE   (0x01),               
        HID_REPORT_COUNT  (0x08),               
        HID_USAGE         (0xB5),               //  Usage (Scan Next Track)
        HID_USAGE         (0xB6),               //  Usage (Scan Previous Track)
        HID_USAGE         (0xB7),               //  Usage (Stop)
        HID_USAGE         (0xB8),               //  Usage (Eject)
        HID_USAGE         (0xCD),               //  Usage (Play/Pause)
        HID_USAGE         (0xE2),               //  Usage (Mute)
        HID_USAGE         (0xE9),               //  Usage (Volume Increment)
        HID_USAGE         (0xEA),               //  Usage (Volume Decrement)
        HID_INPUT         (HID_DATA_BIT | HID_VAR_BIT | HID_ABS_BIT |
                           HID_NWRP_BIT | HID_LIN_BIT | HID_PREF_BIT |
                           HID_NNUL_BIT),                               // Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        HID_USAGE16       (0x83, 0x01),   //  Usage (AL Consumer Control Configuration)
        HID_USAGE16       (0x8A, 0x01),   //  Usage (AL Email Reader)
        HID_USAGE16       (0x92, 0x01),   //  Usage (AL Calculator)
        HID_USAGE16       (0x94, 0x01),   //  Usage (AL Local Machine Browser)
        HID_USAGE16       (0x21, 0x02),   //  Usage (AC Search)
        HID_USAGE_MIN_16  (0x23, 0x02),   //  Usage Minimum (AC Home)
        HID_USAGE_MAX_16  (0x25, 0x02),   //  Usage Maximum (AC Forward)
        HID_INPUT         (HID_DATA_BIT | HID_VAR_BIT | HID_ABS_BIT |
                           HID_NWRP_BIT | HID_LIN_BIT | HID_PREF_BIT |
                           HID_NNUL_BIT),                               // Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        HID_USAGE16       (0x26, 0x02),   //  Usage (AC Stop)
        HID_USAGE16       (0x27, 0x02),   //  Usage (AC Refresh)
        HID_USAGE16       (0x2A, 0x02),   //  Usage (AC Bookmarks)
        HID_USAGE         (0x40),         //  Usage    (Menu)          
        HID_USAGE         (0x30),         //  Usage    (Power)         
        HID_USAGE         (0x9A),         //  Usage (Media select home
        HID_USAGE         (0x46),         //  Usage (Menu escape)      
        HID_REPORT_COUNT  (0x07),         
        HID_INPUT         (HID_DATA_BIT | HID_VAR_BIT | HID_ABS_BIT |
                           HID_NWRP_BIT | HID_LIN_BIT | HID_PREF_BIT |
                           HID_NNUL_BIT),                               // Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        HID_REPORT_COUNT  (0x01),         
        HID_INPUT         (HID_CONST_BIT | HID_ARY_BIT | HID_ABS_BIT),         //  Input (Cnst,Ary,Abs)      
        HID_END_COLLECTION,              
////////////////////////////////////////////////////////////////////////////////////////////////

#if (HAS_BMI055 || HAS_AUDIO)
        HID_USAGE_PAGE_VENDOR_DEFINED,               
        HID_USAGE         (0x01),                    
        HID_COLLECTION    (HID_LOGICAL),             
                               
        HID_REPORT_ID     (0x04),                    
        HID_USAGE_PAGE_VENDOR_DEFINED,               
        HID_USAGE_MIN_8   (0x00),                    
        HID_USAGE_MAX_8   (0x00),                    
        HID_REPORT_COUNT  (0x14),                    
        HID_REPORT_SIZE   (0x08),                    
        HID_LOGICAL_MIN_8 (0x00),                    
        HID_LOGICAL_MAX_16(0xff, 0x00),              
        HID_OUTPUT        (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT |
                           HID_NPREF_BIT),                              // OUTPUT (Data,Ary,Abs,NPrf)
       HID_REPORT_ID     (0x05),                   
        HID_USAGE_PAGE_VENDOR_DEFINED,              
        HID_USAGE_MIN_8   (0x00),                   
        HID_USAGE_MAX_8   (0x00),                   
        HID_REPORT_COUNT  (0x14),                   
        HID_REPORT_SIZE   (0x08),                   
        HID_LOGICAL_MIN_8 (0x00),                   
        HID_LOGICAL_MAX_16(0xff, 0x00),             
        HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT |
                           HID_NPREF_BIT),                              // INPUT (Data,Ary,Abs,NPrf)
        HID_REPORT_ID     (0x06),                    
        HID_USAGE_PAGE_VENDOR_DEFINED,               
        HID_USAGE_MIN_8   (0x00),                    
        HID_USAGE_MAX_8   (0x00),                    
        HID_REPORT_COUNT  (0x14),                    
        HID_REPORT_SIZE   (0x08),                    
        HID_LOGICAL_MIN_8 (0x00),                    
        HID_LOGICAL_MAX_16(0xff, 0x00),              
        HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT |
                           HID_NPREF_BIT),                              // INPUT (Data,Ary,Abs,NPrf)
        HID_REPORT_ID     (0x07),                   
        HID_USAGE_PAGE_VENDOR_DEFINED,              
        HID_USAGE_MIN_8   (0x00),                   
        HID_USAGE_MAX_8   (0x00),                   
        HID_REPORT_SIZE   (0x08),                   
        HID_REPORT_COUNT  (0x14),                   
        HID_LOGICAL_MIN_8 (0x00),                   
        HID_LOGICAL_MAX_16(0xff, 0x00),             
        HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT | HID_NPREF_BIT), //   INPUT (Data,Ary,Abs,NPrf)

        HID_REPORT_ID     (0x08),                    
        HID_USAGE_PAGE_VENDOR_DEFINED,               
        HID_USAGE_MIN_8   (0x00),                    
        HID_USAGE_MAX_8   (0x00),                    
        HID_REPORT_COUNT  (0x14),                    
        HID_REPORT_SIZE   (0x08),                    
        HID_LOGICAL_MIN_8 (0x00),                    
        HID_LOGICAL_MAX_16(0xff, 0x00),              
        HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT |
                           HID_NPREF_BIT),                              // INPUT (Data,Ary,Abs,NPrf)
#endif

#if (HAS_BMI055)
        HID_REPORT_ID     (0x09),                    
        HID_USAGE_PAGE_VENDOR_DEFINED,               
        HID_USAGE_MIN_8   (0x00),                    
        HID_USAGE_MAX_8   (0x00),                    
        HID_REPORT_COUNT  (0x14),                    
        HID_REPORT_SIZE   (0x08),                    
        HID_LOGICAL_MIN_8 (0x00),                    
        HID_LOGICAL_MAX_16(0xff, 0x00),              
        HID_INPUT         (HID_DATA_BIT | HID_ARY_BIT | HID_ABS_BIT |
                           HID_NPREF_BIT),                              // INPUT (Data,Ary,Abs,NPrf)
#endif        
        HID_END_COLLECTION                          
	};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

int keyboard_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (param->status != PRF_ERR_OK) {
        ASSERT_WARNING(0);
    }

	struct hogpd_set_report_map_req *req = KE_MSG_ALLOC_DYN(HOGPD_SET_REPORT_MAP_REQ,
                                                                TASK_HOGPD, TASK_APP,
                                                                hogpd_set_report_map_req,
                                                                REPORT_MAP_LEN);
	req->report_map_len = REPORT_MAP_LEN;
	req->hids_nb = 0;
	memcpy(&(req->report_map[0]), report_map, REPORT_MAP_LEN);
	ke_msg_send(req);
	
    // Inform the Application Manager
    struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                       TASK_APP, TASK_APP,
                                                       app_module_init_cmp_evt);

    cfm->status = param->status;
    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}


int keyboard_ntf_sent_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ntf_sent_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //Clear pending ack's for param->report_nb == 0 (normal key report) and == 2 (ext. key report)
    if (param->status == PRF_ERR_OK) {
        if (param->hids_nb == 0) {
            if (param->report_nb == 0) {
            } 
            else if (param->report_nb == 2) {
            }
        }
    }
    return (KE_MSG_CONSUMED);
}


int keyboard_proto_mode_ind_handler(ke_msg_id_t const msgid,
                                      struct hogpd_proto_mode_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (HAS_HOGPD_BOOT_PROTO) {
        kbd_proto_mode = param->proto_mode;
        app_multi_bond_store_ccc(CHAR_PROTOCOL_MODE_POS, 0, param->proto_mode);
    } else if (param->proto_mode == HOGP_BOOT_PROTOCOL_MODE) {
        ASSERT_WARNING(0);
    }
    return (KE_MSG_CONSUMED);
}


int keyboard_ntf_config_ind_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ntf_cfg_ind const *ind,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    switch(ind->cfg_code) {
    case HOGPD_BOOT_KB_IN_REPORT_CFG:
        app_multi_bond_store_ccc(CHAR_BOOT_KB_IN_REPORT_POS, 0, ind->ntf_en);
        break;
    case HOGPD_REPORT_CFG:
        app_multi_bond_store_ccc(CHAR_REPORT_POS, ind->report_nb ? 1 : 0, ind->ntf_en);
        break;
    default:
        break;
    }
    return (KE_MSG_CONSUMED);
}


int keyboard_ctnl_pt_ind_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ctnl_pt_ind const *ind,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    app_multi_bond_store_ccc(CHAR_HID_CTNL_PT_POS, 0, ind->hid_ctnl_pt);
    return (KE_MSG_CONSUMED);
}


int keyboard_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_CONNECTED) {
        // Go to the idle state
        ke_state_set(dest_id, APP_CONNECTABLE);
    }

    return (KE_MSG_CONSUMED);
}

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
