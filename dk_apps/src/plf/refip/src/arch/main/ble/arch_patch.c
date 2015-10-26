/**
 ****************************************************************************************
 *
 * @file arch_patch.c
 *
 * @brief ROM code patches.
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
#include "arch.h"
#include <stdlib.h>
#include <stddef.h>     // standard definitions
#include <stdint.h>     // standard integer definition
#include <stdbool.h>    // boolean definition
#include "ke_task.h"

#include "gtl_env.h"
#include "gtl_task.h"


/**
 * @addtogroup PATCHES
 * @{
 */

/*
 * DEFINES
 ****************************************************************************************
 */


#if (LOG_MEM_USAGE)
    #if (!DEVELOPMENT_DEBUG)
        #error "LOG_MEM_USAGE must not be set when building for production (DEVELOPMENT_DEBUG is 0)"
    #else
        #warning "LOG_MEM_USAGE is set! Some patches may be disabled when this option is selected..."
    #endif
#endif


/*
 * FORWARD DECLARATIONS OF EXTERNAL FUNCTIONS
 ****************************************************************************************
 */

#if (LOG_MEM_USAGE)
void *log_ke_malloc(uint32_t size, uint8_t type);
void log_ke_free(void* mem_ptr);
#endif


#ifdef __DA14581__

struct l2cc_att_rd_req;
uint8_t atts_read_resp_patch(uint8_t conidx, struct l2cc_att_rd_req* req);

struct advertising_pdu_params;
struct co_buf_tx_node;
struct lld_evt_tag *lld_adv_start_patch(struct advertising_pdu_params *adv_par,
                                        struct co_buf_tx_node *adv_pdu,
                                        struct co_buf_tx_node *scan_rsp_pdu,
                                        uint8_t adv_pwr);

#if (!BLE_HOST_PRESENT)
int llm_rd_local_supp_feats_cmd_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);
#endif


struct smpc_pairing_cfm;
int PATCHED_581_smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                     struct smpc_pairing_cfm *param,
                                     ke_task_id_t const dest_id, ke_task_id_t const src_id);
                                     
#else /* DA14580 */

extern void smpc_send_pairing_req_ind(uint8_t conidx, uint8_t req_type);

struct gapc_pairing;
extern bool smpc_check_pairing_feat(struct gapc_pairing *pair_feat);

struct l2cc_pdu_recv_ind;
extern int l2cc_pdu_recv_ind_handler(ke_msg_id_t const msgid, struct l2cc_pdu_recv_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id);
struct smpc_pairing_cfm;
extern int smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                    struct smpc_pairing_cfm *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id);
 
struct llcp_con_up_req;
extern void my_llc_con_update_req_ind(uint16_t conhdl, struct llcp_con_up_req const *param);

struct llcp_channel_map_req;
extern void my_llc_ch_map_req_ind (uint16_t conhdl, struct llcp_channel_map_req const *param);

struct gapm_start_advertise_cmd;
extern uint8_t patched_gapm_adv_op_sanity(struct gapm_start_advertise_cmd *adv);

#endif // __DA14581__


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

//The linker removes this if LOG_MEM_USAGE is 0
struct mem_usage_log mem_log[KE_MEM_BLOCK_MAX] __attribute__((section("retention_mem_area0"), zero_init));

#ifdef __DA14581__
uint32_t arch_adv_int __attribute__((section("retention_mem_area0"), zero_init));
#endif


/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#include "gtl.h"
#include "gapm_task.h"

#if ((BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1)  && BLE_HOST_PRESENT )
int gtl_sleep_to_handler(ke_msg_id_t const msgid,
                        void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);
int gtl_polling_to_handler(ke_msg_id_t const msgid,
                        void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);

/// KE TASK element structure
struct ke_task_elem
{
    uint8_t   type;
    struct ke_task_desc * p_desc;
};

/// KE TASK environment structure
struct ke_task_env_tag
{
    uint8_t task_cnt;
    struct ke_task_elem task_list[];
};
extern volatile struct ke_task_env_tag ke_task_env;

#define MAX_GTL_PENDING_PACKETS_ADV (50)
#define MAX_GTL_PENDING_PACKETS     (MAX_GTL_PENDING_PACKETS_ADV + 10)

/**
****************************************************************************************
* @brief Function called to send a message through UART.
*
* @param[in]  msgid   U16 message id from ke_msg.
* @param[in] *param   Pointer to parameters of the message in ke_msg.
* @param[in]  dest_id Destination task id.
* @param[in]  src_id  Source task ID.
*
* @return             Kernel message state, must be KE_MSG_NO_FREE.
*****************************************************************************************
*/
static int my_gtl_msg_send_handler (ke_msg_id_t const msgid,
                          void *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
     //extract the ke_msg pointer from the param passed and push it in GTL queue
    struct ke_msg *msg = ke_param2msg(param);

    // Check if there is no transmission ongoing
    if (ke_state_get(TASK_GTL) != GTL_TX_IDLE)
    {
        if(gtl_env.tx_queue.tx_data_packet > MAX_GTL_PENDING_PACKETS_ADV)
        {
            if(msgid == GAPM_ADV_REPORT_IND || gtl_env.tx_queue.tx_data_packet > MAX_GTL_PENDING_PACKETS)
                return KE_MSG_CONSUMED;
        }
        co_list_push_back(&gtl_env.tx_queue, &(msg->hdr));
    }
    else
    {
        // send the message
        gtl_send_msg(msg);

        // Set GTL task to TX ONGOING state
        ke_state_set(TASK_GTL, GTL_TX_ONGOING);
    }

    //return NO_FREE always since gtl_eif_write handles the freeing
    return KE_MSG_NO_FREE;
}

const struct ke_msg_handler my_gtl_default_state[] =
{
    /** Default handler for GTL TX message, this entry has to be put first as table is
        parsed from end to start by Kernel */
    {KE_MSG_DEFAULT_HANDLER,  (ke_msg_func_t)my_gtl_msg_send_handler},

    #if (DEEP_SLEEP)
    {GTL_SLEEP_TO, (ke_msg_func_t)gtl_sleep_to_handler},
    {GTL_POLLING_TO, (ke_msg_func_t)gtl_polling_to_handler},
    #endif // DEEP_SLEEP
};

const struct ke_state_handler my_gtl_default_handler = KE_STATE_HANDLER(my_gtl_default_state);
const struct ke_task_desc TASK_DESC_GTL = {NULL, &my_gtl_default_handler, gtl_state, GTL_STATE_MAX, GTL_IDX_MAX};

void patch_gtl_task()
{
    uint8_t hdl;
    //struct ke_task_desc * p_task_desc = NULL;
    volatile struct ke_task_elem * curr_list = ke_task_env.task_list;
    uint8_t curr_nb = ke_task_env.task_cnt;

    // Search task handle
    for (hdl = 0; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == TASK_GTL)
        {
            ke_task_env.task_list[hdl].p_desc = (struct ke_task_desc *) &TASK_DESC_GTL;
            return;
        }
    }
}

#endif // #if (BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1)


#ifndef __DA14581__
static bool cmp_abs_time(struct co_list_hdr const * timerA, struct co_list_hdr const * timerB)
{
    uint32_t timeA = ((struct ke_timer*)timerA)->time;
    uint32_t timeB = ((struct ke_timer*)timerB)->time;

    return (((uint32_t)( (timeA - timeB) & 0xFFFF) ) > KE_TIMER_DELAY_MAX);
}

#endif


/*
 * PATCH TABLE DEFINITION
 ****************************************************************************************
 */

static const uint32_t * const patch_table[] = 
{
#ifdef __DA14581__
    
    /* DA14581 */
    
# if (BLE_HOST_PRESENT)
    [0] = (const uint32_t *) atts_read_resp_patch,
# else
    [0] = (const uint32_t *) llm_rd_local_supp_feats_cmd_handler,
# endif
    [1] = (const uint32_t *) lld_adv_start_patch,
# if (LOG_MEM_USAGE)
    [6] = (const uint32_t *) log_ke_malloc,
    [7] = (const uint32_t *) log_ke_free,
# endif

#else
    
    /* DA14580 */
    
    (const uint32_t *) cmp_abs_time,
    (const uint32_t *) l2cc_pdu_recv_ind_handler,
    (const uint32_t *) smpc_send_pairing_req_ind,
    (const uint32_t *) smpc_check_pairing_feat,
    (const uint32_t *) smpc_pairing_cfm_handler,
# if (LOG_MEM_USAGE)
    (const uint32_t *) log_ke_malloc,
    (const uint32_t *) log_ke_free,	
# else    
    (const uint32_t *) my_llc_con_update_req_ind,
    (const uint32_t *) my_llc_ch_map_req_ind,	
# endif        
    (const uint32_t *) patched_gapm_adv_op_sanity,
    
#endif //__DA14581__
};

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief SVC_Handler. Handles h/w patching mechanism IRQ
 *
 * @return void 
 ****************************************************************************************
 */
void SVC_Handler_c(unsigned int * svc_args)
{
    // Stack frame contains:
    // r0, r1, r2, r3, r12, r14, the return address and xPSR
    // - Stacked R0 = svc_args[0]
    // - Stacked R1 = svc_args[1]
    // - Stacked R2 = svc_args[2]
    // - Stacked R3 = svc_args[3]
    // - Stacked R12 = svc_args[4]
    // - Stacked LR = svc_args[5]
    // - Stacked PC = svc_args[6]
    // - Stacked xPSR= svc_args[7]

    unsigned int svc_number;
    
    svc_number = ((char *)svc_args[6])[-2];
    
    if (svc_number < (sizeof patch_table)/4)
        svc_args[6] = (uint32_t)patch_table[svc_number];
    else
        while(1);

    return;
}


/**
 ****************************************************************************************
 * @brief Set and enable h/w Patch functions
 *
 * @return void
 ****************************************************************************************
 */
void patch_func(void)
{

#ifdef __DA14581__
    
    /* DA14580 */
    
    //0x00024b1d lld_adv_start
    SetWord32(PATCH_ADDR0_REG, 0x00024b1C);
    SetWord32(PATCH_DATA0_REG, 0xb5ffdf01); //lld_adv_start svc 1 (+ enabling of interrupts)
    
# if (BLE_HOST_PRESENT)	
    //0x0002a8c5 atts_read_resp
    SetWord32(PATCH_ADDR1_REG, 0x0002a8c4);
    SetWord32(PATCH_DATA1_REG, 0xb5f8df00); //atts_read_resp svc 0 (+ enabling of interrupts)
    
    // replace SMPC_PAIRING_CFM handler in smpc_default_state handler table:
    // smpc_default_state[8].func = &PATCHED_581_smpc_pairing_cfm_handler
    SetWord32(PATCH_ADDR2_REG, 0x00034538 + 8 * 8 + 4);
    SetWord32(PATCH_DATA2_REG, (uint32_t)&PATCHED_581_smpc_pairing_cfm_handler); //originally contained 0x0002d231 = the address of ROM function smpc_pairing_cfm_handler
# else
    //0x000278f9 llm_rd_local_supp_feats_cmd_handler
    SetWord32(PATCH_ADDR1_REG, 0x000278f8);
    SetWord32(PATCH_DATA1_REG, 0x4619df00); //llm_rd_local_supp_feats_cmd_handler svc 0 

    SetWord32(PATCH_ADDR2_REG, 0x000278c0); // llm_rd_local_ver_info_cmd_handler
    SetWord32(PATCH_DATA2_REG, 0x80c121d2); // patch manufacturer ID

    SetWord32(PATCH_ADDR3_REG, 0x0002299c); // llc_version_ind_pdu_send
    SetWord32(PATCH_DATA3_REG, 0x80d020d2); // patch manufacturer ID
# endif

# if (LOG_MEM_USAGE)
    //0x00031fcd  ke_malloc
    SetWord32(PATCH_ADDR6_REG, 0x00031fcc);
    SetWord32(PATCH_DATA6_REG, 0x468edf06); //ke_malloc svc 6

    //0x0003208f  ke_free
    SetWord32(PATCH_ADDR7_REG, 0x0003208c);
    SetWord32(PATCH_DATA7_REG, 0xdf07bdf8); //ke_free svc 7
# endif

#else

    /* DA14580 */
    
    //0x00032795 cmp_abs_time
    SetWord32(PATCH_ADDR0_REG, 0x00032794);
    SetWord32(PATCH_DATA0_REG, 0xdf00b662); //cmp_abs_time svc 0 (+ enabling of interrupts)

    //0x0002a32b l2cc_pdu_recv_ind_handler (atts)
    SetWord32(PATCH_ADDR1_REG, 0x0002a328);
    SetWord32(PATCH_DATA1_REG, 0xdf014770); //l2cc_pdu_recv_ind_handler svc 1

    //0x0002ca1f  smpc_send_pairing_req_ind
    SetWord32(PATCH_ADDR2_REG, 0x0002ca1c);
    SetWord32(PATCH_DATA2_REG, 0xdf02bdf8); //smpc_send_pairing_req_ind svc 2

    //0x0002cb43  smpc_check_pairing_feat
    SetWord32(PATCH_ADDR3_REG, 0x0002cb40);
    SetWord32(PATCH_DATA3_REG, 0xdf03e7f5); //smpc_check_pairing_feat svc 3

    //0x0002d485  smpc_pairing_cfm_handler
    SetWord32(PATCH_ADDR4_REG, 0x0002d484);
    SetWord32(PATCH_DATA4_REG, 0xb089df04); //smpc_pairing_cfm_handler svc 4

# if (LOG_MEM_USAGE)
    //0x00032215  ke_malloc
    SetWord32(PATCH_ADDR5_REG, 0x00032214);
    SetWord32(PATCH_DATA5_REG, 0x468edf05); //ke_malloc svc 5

    //0x000322d7  ke_free
    SetWord32(PATCH_ADDR6_REG, 0x000322d4);
    SetWord32(PATCH_DATA6_REG, 0xdf06bdf8); //ke_free svc 6
# else
    //0x000233bf  llc_con_update_req_ind
    SetWord32(PATCH_ADDR5_REG, 0x000233bc);
    SetWord32(PATCH_DATA5_REG, 0xdf05bdf8); //llc_con_update_req_ind svc 5

    //0x0002341b  llc_ch_map_req_ind
    SetWord32(PATCH_ADDR6_REG, 0x00023418);
    SetWord32(PATCH_DATA6_REG, 0xdf06bdf8); //llc_ch_map_req_ind svc 6
# endif

    //0x00030cef gapm_adv_op_sanity
    SetWord32(PATCH_ADDR7_REG, 0x00030cec);
    SetWord32(PATCH_DATA7_REG, 0xdf07bd70); //gapm_adv_op_sanity svc 7

    NVIC_DisableIRQ(SVCall_IRQn);
    NVIC_SetPriority(SVCall_IRQn, 0);
    NVIC_EnableIRQ(SVCall_IRQn);
#endif //__DA14581__
}



/// @} PATCHES
