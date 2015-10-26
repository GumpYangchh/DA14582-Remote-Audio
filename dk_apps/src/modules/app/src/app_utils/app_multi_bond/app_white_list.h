/**
 ****************************************************************************************
 *
 * @file app_white_list.h
 *
 * @brief White List mgmt header file.
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

#ifndef APP_WHITE_LIST_H_
#define APP_WHITE_LIST_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup White List
 *
 * @brief White List management entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

#include "rwble_config.h"
#include "co_bt.h"


/*
 * Type declarations
 ****************************************************************************************
 */

enum wl_entry_status {
    UNUSED = 0,
    USED_ADDR_PUBLIC_or_PRIVATE,
    USED_ADDR_RAND
};

struct virtual_wl_entry {
    enum wl_entry_status status;
    union {
        struct bd_addr addr;
        int8_t entry;
    } info;
};


/*
 * Public variables
 ****************************************************************************************
 */

extern uint8_t white_list_written;
extern enum adv_filter_policy virtual_wlist_policy;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief       Adds a host to the normal or virtual White List.
 *
 * @details     Adds a host to the White List. If the host has Public Address,
 *              it is added to the normal White List. If it has a Resolvable
 *              Random Address, it is added to the virtual White List.
 *
 * @param[in]   peer_addr_type   The BD_ADDR type of the host.
 * @param[in]   peer_addr        The BD_ADDR of the host.
 * @param[in]   bond_info_entry  The index to the storage area for this bonding info.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if no error occurred; white_list_written will be increased by 1 upon successful completion
 *                  <li> false if the host could not be added for some reason
 *              </ul>
 ****************************************************************************************
 */
bool add_host_in_white_list(uint8_t peer_addr_type, struct bd_addr const *peer_addr, uint8_t bond_info_entry);


/**
 ****************************************************************************************
 * @brief       Removes a host to the normal or virtual White List.
 *
 * @details     Removes a host from the White List. If the host has Public Address,
 *              it is removed from the normal White List. If it has a Resolvable
 *              Random Address, it is removed from the virtual White List.
 *
 * @param[in]   peer_addr_type   The BD_ADDR type of the host.
 * @param[in]   peer_addr        The BD_ADDR of the host.
 * @param[in]   bond_info_entry  The index to the storage area for this bonding info.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if no error occurred; white_list_written will be decreased by 1 upon successful completion
 *                  <li> false if the host could not be removed for some reason
 *              </ul>
 *
 ****************************************************************************************
 */
bool remove_host_from_white_list(uint8_t peer_addr_type, struct bd_addr const *peer_addr, uint8_t bond_info_entry);


/**
 ****************************************************************************************
 * @brief       Looks up a host with Resolvable Random address in virtual White List.
 *
 * @details     Looks for a host with Resolvable Random address into the virtual White List.
 *
 * @param       bond_info_entry  The index to the storage area for this bonding info.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if the host has been found or the policy is ADV_ALLOW_SCAN_ANY_CON_ANY
 *                  <li> false if the host wasn't found
 *              </ul>
 *
 * @warning     The caller should only call this function for Resolvable Random addresses.
 *
 ****************************************************************************************
 */
bool lookup_rand_in_virtual_white_list(uint8_t entry);


/**
 ****************************************************************************************
 * @brief       Looks up a host with Public or Static Random address in virtual White List.
 *
 * @details     Looks for a host with Public or Static Random address into the virtual White List.
 *
 * @param       bond_info_entry  The index to the storage area for this bonding info.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if the host has been found or the policy is ADV_ALLOW_SCAN_ANY_CON_ANY
 *                  <li> false if the host wasn't found
 *              </ul>
 *
 * @warning     The caller should only call this function for Public or Static Random addresses.
 *
 ****************************************************************************************
 */
bool lookup_public_in_virtual_white_list(uint8_t peer_addr_type, struct bd_addr const *peer_addr);


/**
 ****************************************************************************************
 * @brief       Clears both the normal and the virtual White Lists.
 *
 * @return      void
 ****************************************************************************************
 */
void clear_white_list(void);

/// @} APP

#endif // APP_WHITE_LIST_H_
