/**
 ****************************************************************************************
 *
 * @file app_sec.c
 *
 * @brief Application Security Entry Point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
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

#include "rwip_config.h"

#if (BLE_APP_SEC)

#if (NVDS_SUPPORT)
#include "nvds.h"           // NVDS API Definition
#endif //(NVDS_SUPPORT)

#include "app_api.h"                // Application task Definition
#include "gapc_task.h"      // GAP Controller Task API Definition
#include "app_sec_task.h"   // Application Security Task API Definition
#include "app_sec.h"        // Application Security API Definition
#include <stdlib.h>

/*
 * DEFINES
 ****************************************************************************************
 */
#define PIN_CODE_MIN   (100000)
#define PIN_CODE_MAX   (999999)
 

// OOB Information
#define OOB_INFORMATION         (GAP_OOB_AUTH_DATA_NOT_PRESENT)

// Authentication Requirements
#define AUTH_REQ                (GAP_AUTH_REQ_NO_MITM_NO_BOND)
// Initiator Key Distribution - Peer Device
#define I_KEYS                  (GAP_KDIST_NONE)
// Responder Key Distribution
#define R_KEYS                  (GAP_KDIST_ENCKEY)

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Security Environment Structure
#if (USE_CONNECTION_FSM)
struct bonding_info_ bond_info     __attribute__((section("retention_mem_area0"),zero_init));        // Bonding info for current host
#else
struct app_sec_env_tag app_sec_env __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
#endif // USE_CONNECTION_FSM

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Generates pin code
 *
 * @return pin code
 ****************************************************************************************
 */
uint32_t app_sec_gen_tk(void)
{
    // Generate a PIN Code (Between PIN_CODE_MIN and PIN_CODE_MAX)
    return (PIN_CODE_MIN + (rand()%(PIN_CODE_MAX - PIN_CODE_MIN + 1)));
}


/**
 ****************************************************************************************
 * @brief Generates Long Term Key in app_sec_env
 *
 * @param[in] key_size     Generated key's size
 *
 * @return void
 ****************************************************************************************
 */
void app_sec_gen_ltk(uint8_t key_size)
{
    struct app_sec_env_tag *sec_env;

#if (USE_CONNECTION_FSM)
    sec_env=&bond_info.env;
#else
    sec_env=&app_sec_env;
#endif

    // Counter
    uint8_t i;
    sec_env->key_size = key_size;

    // Randomly generate the LTK and the Random Number
    for (i = 0; i < RAND_NB_LEN; i++) {
        sec_env->rand_nb.nb[i] = rand()%256;

    }

    // Randomly generate the end of the LTK
    for (i = 0; i < KEY_LEN; i++) {
        sec_env->ltk.key[i] = (((key_size) < (16 - i)) ? 0 : rand()%256);
    }

    // Randomly generate the EDIV
    sec_env->ediv = rand()%65536;
}


/**
 ****************************************************************************************
 * @brief Init security parametres during system initialization process
 *
 * @return void
 ****************************************************************************************
 */
void app_sec_init(void) 
{
    struct app_sec_env_tag *sec_env;

#if (USE_CONNECTION_FSM)
    sec_env=&bond_info.env;
#else
    sec_env=&app_sec_env;
#endif

    // Reset Security Environment
    memset(sec_env, 0, sizeof(*sec_env));
		
	app_sec_init_func();
}


/**
 ****************************************************************************************
 * @brief Start security procedure
 *
 * @return void
 ****************************************************************************************
 */
void app_security_start(void)
{
    // Send security request command
    struct gapc_security_cmd * cmd = KE_MSG_ALLOC(GAPC_SECURITY_CMD,
            KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP, gapc_security_cmd);

    // Security request
    cmd->operation = GAPC_SECURITY_REQ;

#if (USE_CONNECTION_FSM)
    cmd->auth = GAP_AUTH_REQ_NO_MITM_BOND;
#else
    cmd->auth      = ((app_sec_env.auth == 0) ? GAP_AUTH_REQ_NO_MITM_NO_BOND : app_sec_env.auth);
#endif

    // Send the message
    ke_msg_send(cmd);

    // Go to security State
    ke_state_set(TASK_APP, APP_SECURITY);
}
#endif //(BLE_APP_SEC)

/// @} APP
