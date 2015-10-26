/**
****************************************************************************************
*
* @file app_multi_bond.h
*
* @brief Special (multi) bonding procedure header file.
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

#ifndef APP_MULTI_BOND_H_
#define APP_MULTI_BOND_H_

/*
 ****************************************************************************************
 * USAGE
 *
 * To use this module the following must be properly defined in your project and include the header file in app_multi_bond.c:
 *     ALT_PAIR_DISCONN_TIME : Time to block previous host during a "host-switch" (in 10th of msec) 
 *
 * and the following must be defined as true if used or false if not used:
 * con_fsm_params.has_nv_rom : if there's no storage memory then using this module is useless.
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "app_sec.h"

enum notification_info_pos {
    CHAR_PROTOCOL_MODE_POS,
    CHAR_BATTERY_LEVEL_POS,
    CHAR_BOOT_KB_IN_REPORT_POS,
    CHAR_REPORT_POS,
    CHAR_HID_CTNL_PT_POS,
    CHAR_SERVICE_CHANGED_POS,
    ENUM_NTF_INFO_POS_END
};

/*
 * DEFINES
 ****************************************************************************************
 */

// NV_STORAGE_BASE_ADDR(3:0) are left unused
#define NV_STORAGE_MAGIC_ADDR           (NV_STORAGE_BASE_ADDR + 0x04)
#define NV_STORAGE_STATUS_ADDR          (NV_STORAGE_BASE_ADDR + 0x08)
#define NV_STORAGE_USAGE_ADDR           (NV_STORAGE_BASE_ADDR + 0x09)
#define NV_STORAGE_BOND_DATA_ADDR       (NV_STORAGE_BASE_ADDR + 0x10)   // Up to 7 bonds (0x10 - 0x09 = 7)

#define NV_STORAGE_GUARD_NUMBER         (0xA55A5AA5)
#define NV_STORAGE_MAGIC_NUMBER         (0xDEADBEEF)



#define ATTR_TYPE   0
#define CCC_TYPE    1

struct notification_info_
{
    BYTE position;     // potition in the storage memory
    int uuid;          
    BYTE type;         // type, CCC_TYPE or ATTR_TYPE
    BYTE num_of_atts; 
    att_size_t length;
};

/*
 * Nono-volatile memory Structure
 ****************************************************************************************
 *
 *                            7         6         5         4         3         2         1         0
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 * NV_STORAGE_STATUS_ADDR    |                                    STATUS                                     |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                                                                  ...
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 * NV_STORAGE_USAGE_ADDR     |                                   Usage #0                                    |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                           |                                   Usage #1                                    |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                           |                                      ...                                      |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                           |                               Usage #N (< = 15)                               |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                                                                  ...
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 * NV_STORAGE_BOND_DATA_ADDR |                                Bonding info #0                                |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                           |                                      ...                                      |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                    (+60)  |                                Bonding info #1                                |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                           |                                      ...                                      |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                           |         |         |         |         |         |         |         |         |
 *                   (+120)  |                                Bonding info #2                                |
 *                           |         |         |         |         |         |         |         |         |
 *                           |---------|---------|---------|---------|---------|---------|---------|---------|
 *                                                              ...
 */

/*
 * BONDING INFO
 ****************************************************************************************
 *
 * Byte 0
 * ------
 *      7         6         5         4         3         2         1         0
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 * |         |         |         |         |         |         |         |         |
 * |        SCF        |   CPS   |                  NOS                  |   MODE  |
 * |         |         |         |         |         |         |         |         |
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 *
 * MODE                        : 0 = Boot mode, 1 = Report mode
 * NOS (Notifications' status) : Bit value    : 0 = Notifications are disabled, 1 = Notifications are enabled
 *                               Bit position : 1 = Battery, 2 = Boot Report (Input), 3 = Report #0 (Input), 4 = Report #2 (Input)
 * CPS (Control Point Status)  : Value of the control point characteristic
 * SCF (Service Changed flag)  : 0 = Not notified, Not indicated
 *                               1 = error
 *                               2 = Indicated
 *                               3 = error
 *                               The rangle of the affected handles is in Byte 1. If it's not zero then the characteristic must be updated.
 *
 *
 * Byte 1
 * ------
 * Set after any SW update that results in a modification of the DB (at all entries).
 * Cleared (in a specific entry) after informing the corresponding Host.
 *
 *      7         6         5         4         3         2         1         0
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 * |         |         |         |         |         |         |         |         |
 * |                      Start of affected handle attr range                      |
 * |         |         |         |         |         |         |         |         |
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 *
 *
 *
 * Byte 2
 * ------
 * Set after any SW update that results in a modification of the DB (at all entries).
 * Cleared (in a specific entry) after informing the corresponding Host.
 *
 *      7         6         5         4         3         2         1         0
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 * |         |         |         |         |         |         |         |         |
 * |                       End of affected handle attr range                       |
 * |         |         |         |         |         |         |         |         |
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 *
 *
 * Bytes 3 - 6 : RESERVED
 * ------
 *
 *
 * Byte 7
 * ------
 *      7         6         5         4         3         2         1         0
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 * |         |         |         |         |         |         |         |         |
 * | IRK flg |                          RESERVED                                   |
 * |         |         |         |         |         |         |         |         |
 * |---------|---------|---------|---------|---------|---------|---------|---------|
 *
 *
 * Bytes 8 - 23 : IRK
 * ------------
 *
 *
 * Bytes 24 - 59 : app_sec_env_tag
 * ------
 */
 
#define IRK_FLAG                (1 << 7)

struct usage_array_
{
    uint8_t pos[MAX_BOND_PEER];
};

struct irk_array_
{
    struct gap_sec_key irk[MAX_BOND_PEER];
};

/*
 * VARIABLES
 ****************************************************************************************
 */

enum multi_bond_host_rejection {
    MULTI_BOND_REJECT_NONE, 
    MULTI_BOND_REJECT_LAST, 
    MULTI_BOND_REJECT_ALL_KNOWN
};

extern uint8_t multi_bond_status;
extern uint8_t multi_bond_resolved_peer_pos;

extern struct usage_array_ bond_usage;

extern struct irk_array_ irk_array;

extern const struct notification_info_ notification_info[ENUM_NTF_INFO_POS_END];
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief       Initialize NV memory.
 *
 * @details     Checks the existance of the MAGIC number and the sanity of the usage counters.
 *              If any of the checks fails, it deletes the area of the NV memory which is used
 *              to store the bonding info. 
 *              It initializes bond_usage (usage counters) from the data stored in the NV memory.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_init(void);

bool app_alt_pair_disconnect(void);


/**
 ****************************************************************************************
 * @brief       Handler of the "Host switch" timer.
 *
 * @details     Called when the "switching period" of 60 sec expires.
 *
 * @return      int
 *
 * @retval      KE_MSG_CONSUMED (message handler)
 *
 ****************************************************************************************
 */
int app_alt_pair_timer_handler(void);

/**
 ****************************************************************************************
 * @brief       Filter a Host during a Host switch upon reception of a CONNECT_REQ message.
 *
 * @details     Called when a CONNECT_REQ is received during the "switching period" of 60 sec.
 *              It simply checks if the connecting Host is the same with the last one, the one
 *              being blocked for 60 sec. Since it checks the BD_ADDR only, it won't work in case
 *              of Host with Random addresses. In this case, the filtering is based on EDIV and RAND
 *              and is done in app_alt_pair_load_bond_data().
 *
 * @param[in]   peer_addr       BD_ADDR of the connecting host
 * @param[in]   peer_addr_type  Address Type of the connecting host's address (i.e. Public)
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if Multi-Bond is not supported or the connecting Host is (or may be,
 *                       in case of Hosts with Random addresses) different than the last one
 *                  <li> false if Multi-Bond is supported and the connecting Host is the same with
 *                       the last one
 *              </ul>
 *
 ****************************************************************************************
 */
bool app_alt_pair_check_peer(struct gapc_connection_req_ind const *param);


/**
 ****************************************************************************************
 * @brief       Read Multi-Bond status.
 *
 * @details     Initializes multi_bond_status from the data stored in the NV memory.
 *              multi_bond_status is a a flag which indicates the validity of host data 
 *              in the available slots. A '1' at a bit position indicates that the corresponding
 *              slot has valid host data. This field also informs about the number of the used slots
 *              (and, consequently, the number of bonded hosts).
 *              If selective delete is added then this field must be updated properly.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_read_status(void);

/**
 ****************************************************************************************
 * @brief       Store the bonding information of the connected Host to the NV memory.
 *
 * @details     First, it looks for an entry for this Host in the NV memory. If one is found,
 *              it updates it only if the Host is using a Random address or the write is
 *              forced because of a DB change (bond_info.info).
 *              If no entry is found in the NV memory for this host then it gets one via
 *              get_entry_to_delete() and writes the data there.
 *              The usage counters are updated in both cases.
 *
 * @param[in]   force       if 1 then a forced update of the entry is done
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_store_bond_data(void);


/**
 ****************************************************************************************
 * @brief       Check if bond info must be stored in the NV memory due to a DataBase change.
 *
 * @param[in]   mask        The bitfield mask in the info
 * @param[in]   pos         The offest of the field in the info
 * @param[in]   value       The value of this field in the DataBase.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_update_bond_data(int mask, int pos, int value);


/**
 ****************************************************************************************
 * @brief       Find the keys for the connecting host, if available.
 *
 * @details     Search if there's an entry in the NV memory for the connecting host. It is 
 *              called upon the reception of an LL_ENC_REQ message from the host.
 *
 * @param[in]   rand_nb     The RAND_NB value of the LL_ENC_REQ message.
 * @param[in]   ediv        The EDIV value of the LL_ENC_REQ message.
 *
 * @return      int
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> 0 if there's no entry in the NV memory for this host
 *                  <li> 1 if the new host is different from the last host we connected to
 *                  <li> 2 if the new host is the same with the last host we connected to
 *              </ul>
 *
 ****************************************************************************************
 */
int app_alt_pair_load_bond_data(struct rand_nb *rand_nb, uint16_t ediv);


/**
 ****************************************************************************************
 * @brief       Find the first or next entry in the NV memory with valid keys.
 *
 * @details     If init is true then find the first valid entry starting from the beginning.
 *                    Used when no connection exists and advertising is about to start!
 *              If init is false then find the next valid entry (if any) starting just 
 *              after the last position used. This can be the position of the current bonded 
 *              host or, if directed advertising has started, the position of the last host 
 *              we did directed advertising to. Used when connected and intend to start 
 *              directed advertising to other hosts for whom bonded data exist or while a 
 *              cycle of directed advertising to all known bonded hosts, until a connection 
 *              is established, is being executed.
 *
 * @warning     Active and Next pointers will be reset when init is true.
 *
 * @param[in]   init    Controls which entry to find; first, if true or next, if false.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if a valid entry was found
 *                  <li> false if no valid entry was found. bond_info is reset.
 *              </ul>
 *
 ****************************************************************************************
 */ 
bool app_alt_pair_get_next_bond_data(bool init);


/**
 ****************************************************************************************
 * @brief       Find the last used entry in the NV memory and load keys.
 *
 * @details     Find the last used entry in the NV memory and load the bonding info.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if a valid entry was found
 *                  <li> false if the NV memory is empty. bond_info is left unchanged.
 *              </ul>
 *
 ****************************************************************************************
 */
bool app_alt_pair_load_last_used(void);


/**
 ****************************************************************************************
 * @brief       Load the keys from a specific entry in the NV memory.
 *
 * @details     Gets the bonding info from a specific entry in the NV memory and, if it's valid,
 *              it copies it to the bond_info and updates the DataBase according to it.
 *
 * @param       int     The index to the entry in the NV memory.
 *
 * @return      bool
 *
 * @retval      Status of operation.
 *              <ul>
 *                  <li> true if a valid entry was found
 *                  <li> false if there is no valid info at "entry". bond_info is left unchanged.
 *              </ul>
 *
 ****************************************************************************************
 */
bool app_alt_pair_load_entry(int8_t entry);

bool app_alt_pair_read_entry(int8_t entry, struct bonding_info_ *info);


/**
 ****************************************************************************************
 * @brief       Delete a specific entry in the NV memory.
 *
 * @param       int     The index to the entry in the NV memory to be deleted.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_delete_entry(int8_t entry);


/**
 ****************************************************************************************
 * @brief       Clear ALL BONDING DATA FROM the NV memory.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_alt_pair_clear_all_bond_data(void);

int app_alt_peer_get_active_index(void);


/**
 ****************************************************************************************
 * @brief       Update the DataBase based on the bonding information.
 *
 * @details     Updates the DB with the values of inf. If a modified attribute range
 *              is written in the NV memory then a Service Changed indication (if enabled)
 *              is sent to the Host.
 *
 * @param[in]   inf     The bonding information
 *
 * @return      void
 *
 ****************************************************************************************
 */
void updatedb_from_bonding_info(struct bonding_info_ *inf);


/**
 ****************************************************************************************
 * @brief       Initialize the index to the last (or current) used NV memory entry.
 *
 * @param       void
 *
 * @return      void
 *
 ****************************************************************************************
 */
void reset_active_peer_pos(void);

void app_alt_pair_force_next_store_entry(int8_t entry);
    
/**
 ****************************************************************************************
 * @brief       Update CCC notification status on the bonding information.
 *
 * @param[in]   pos       Position of the CCC in the notification_info array
 * @param[in]   attr_num  Attribute number of the CCC
 * @param[in]   value     value of the attribute
 *
 * @return      void
 *
 ****************************************************************************************
 */    
void app_multi_bond_store_ccc(int pos, int attr_num, int value);


/**
 ****************************************************************************************
 * @brief       Refresh usage counters.
 *
 * @details     Refresh usage counters. idx is the current entry being used.
 *              Thus, it gets the maximum value currently available while all
 *              other entries are decremented by 1.
 *
 * @return      void 
 *
 ****************************************************************************************
 */
void app_update_usage_count(uint8_t entry);
#endif // APP_MULTI_BOND_H_
