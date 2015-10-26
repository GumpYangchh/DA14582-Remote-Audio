/**
 ****************************************************************************************
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


#ifndef _APP_KBD_MATRIX_SETUP_16_H_
#define _APP_KBD_MATRIX_SETUP_16_H_

#include <stdint.h>
#include <stddef.h>
#include "app_kbd_macros.h"
#include "da14580_config.h"

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup HID
 *
 * @brief HID (Keyboard) Application matrix of setup #12.
 *
 * @{
 ****************************************************************************************
 */

#if defined(HAS_I2C_EEPROM_STORAGE) || defined(HAS_SPI_FLASH_STORAGE)

#define PAIR                (0xF4F1)
#define CLRP                (0xF4F2)
#else
#define PAIR                (0x0000)
#define CLRP                (0x0000)
#endif


#define KBD_NR_INPUTS       (6)
#define KBD_NR_OUTPUTS      (4)

#define COLUMN_0_PORT		(2)	//port: 3, pin: 5
#define COLUMN_0_PIN		(4)

#define COLUMN_1_PORT		(3)	//port: 2, pin: 5
#define COLUMN_1_PIN		(5)

#define COLUMN_2_PORT		(2)	//port: 2, pin: 5
#define COLUMN_2_PIN		(5)

#define COLUMN_3_PORT		(2)	//port: 2, pin: 6
#define COLUMN_3_PIN		(6)

#define COLUMN_4_PORT		(2)	//port: 2, pin: 0
#define COLUMN_4_PIN		(0)

#define COLUMN_5_PORT		(2)	//port: 2, pin: 9
#define COLUMN_5_PIN		(9)

// used for cycle optimization
#define P0_HAS_INPUT        (0)
#define P1_HAS_INPUT        (0)
#define P2_HAS_INPUT        (1)
#define P3_HAS_INPUT        (1)

#define ROW_0_PORT			(1)	//port: 2, pin: 4
#define ROW_0_PIN			(0)

#define ROW_1_PORT			(3)	//port: 1, pin: 0
#define ROW_1_PIN			(6)

#define ROW_2_PORT			(1)	//port: 3, pin: 6
#define ROW_2_PIN			(2)

#define ROW_3_PORT			(1)	//port: 1, pin: 2
#define ROW_3_PIN			(1)

//#define ROW_4_PORT			(1)	//port: 1, pin: 1
//#define ROW_4_PIN			(1)

//#define ROW_5_PORT			(2)	//port: 2, pin: 8
//#define ROW_5_PIN			(8)


// Masks for the initialization of the KBD controller
#define MASK_P0				(0x4000	| SET_MASK0_FROM_COLUMN(0)                | SET_MASK0_FROM_COLUMN(1)                | SET_MASK0_FROM_COLUMN(2)                | SET_MASK0_FROM_COLUMN(3)                \
									| SET_MASK0_FROM_COLUMN(4)                | SET_MASK0_FROM_COLUMN(5))
uint16_t mask_p0 = MASK_P0;

#define MASK_P12			(0x0000	| SET_MASK12_FROM_COLUMN(0)               | SET_MASK12_FROM_COLUMN(1)               | SET_MASK12_FROM_COLUMN(2)               | SET_MASK12_FROM_COLUMN(3)               \
									| SET_MASK12_FROM_COLUMN(4)               | SET_MASK12_FROM_COLUMN(5))
uint16_t mask_p12 = MASK_P12;

#define MASK_P3				(0x0000	| SET_MASK3_FROM_COLUMN(0)                | SET_MASK3_FROM_COLUMN(1)                | SET_MASK3_FROM_COLUMN(2)                | SET_MASK3_FROM_COLUMN(3)                \
									| SET_MASK3_FROM_COLUMN(4)                | SET_MASK3_FROM_COLUMN(5))
uint16_t mask_p3 = MASK_P3;

// Masks for the initialization of the WKUP controller
#define WKUP_MASK_P0		(	SET_WKUP_MASK_FROM_COLUMN(0, 0)         | SET_WKUP_MASK_FROM_COLUMN(0, 1)         | SET_WKUP_MASK_FROM_COLUMN(0, 2)         | SET_WKUP_MASK_FROM_COLUMN(0, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(0, 4)         | SET_WKUP_MASK_FROM_COLUMN(0, 5))
uint16_t wkup_mask_p0 = WKUP_MASK_P0;

#define WKUP_MASK_P1		(	SET_WKUP_MASK_FROM_COLUMN(1, 0)         | SET_WKUP_MASK_FROM_COLUMN(1, 1)         | SET_WKUP_MASK_FROM_COLUMN(1, 2)         | SET_WKUP_MASK_FROM_COLUMN(1, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(1, 4)         | SET_WKUP_MASK_FROM_COLUMN(1, 5))
uint16_t wkup_mask_p1 = WKUP_MASK_P1;

#define WKUP_MASK_P2		(	SET_WKUP_MASK_FROM_COLUMN(2, 0)         | SET_WKUP_MASK_FROM_COLUMN(2, 1)         | SET_WKUP_MASK_FROM_COLUMN(2, 2)         | SET_WKUP_MASK_FROM_COLUMN(2, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(2, 4)         | SET_WKUP_MASK_FROM_COLUMN(2, 5))
uint16_t wkup_mask_p2 = WKUP_MASK_P2;

#define WKUP_MASK_P3		(	SET_WKUP_MASK_FROM_COLUMN(3, 0)         | SET_WKUP_MASK_FROM_COLUMN(3, 1)         | SET_WKUP_MASK_FROM_COLUMN(3, 2)         | SET_WKUP_MASK_FROM_COLUMN(3, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(3, 4)         | SET_WKUP_MASK_FROM_COLUMN(3, 5))
uint16_t wkup_mask_p3 = WKUP_MASK_P3;

KBD_TYPE_QUALIFIER uint8_t kbd_input_ports[] KBD_ARRAY_ATTRIBUTE =
{
	COL(0),                     // column 0 (P2[7])
	COL(1),                     // column 1
	COL(2),                     // column 2
	COL(3),                     // column 3
	COL(4),                      // column 4
	COL(5)                      // column 4
};

KBD_TYPE_QUALIFIER uint8_t kbd_output_mode_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_OUTPUT_MODE_REG(0),
	SET_OUTPUT_MODE_REG(1),
	SET_OUTPUT_MODE_REG(2),
	SET_OUTPUT_MODE_REG(3),
//	SET_OUTPUT_MODE_REG(4),
//	SET_OUTPUT_MODE_REG(5)
};

KBD_TYPE_QUALIFIER uint8_t kbd_output_reset_data_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_RESET_REG(0),
	SET_RESET_REG(1),
	SET_RESET_REG(2),
	SET_RESET_REG(3),
//	SET_RESET_REG(4),
//	SET_RESET_REG(5)
};

KBD_TYPE_QUALIFIER uint16_t kbd_out_bitmasks[] KBD_ARRAY_ATTRIBUTE =
{
	SET_BITMAP(0),
	SET_BITMAP(1),
	SET_BITMAP(2),
	SET_BITMAP(3),
//	SET_BITMAP(4),
//	SET_BITMAP(5)
};

KBD_TYPE_QUALIFIER uint8_t kbd_input_mode_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_INPUT_MODE_REG(0),
	SET_INPUT_MODE_REG(1),
	SET_INPUT_MODE_REG(2),
	SET_INPUT_MODE_REG(3),
	SET_INPUT_MODE_REG(4),
	SET_INPUT_MODE_REG(5),
};

typedef int kbd_input_ports_check[ (sizeof(kbd_input_ports) / sizeof(uint8_t)) == KBD_NR_INPUTS];                   // on error: the kbd_input_ports[] is not defined properly!
typedef int kbd_output_mode_regs_check[ (sizeof(kbd_output_mode_regs) / sizeof(uint8_t)) == KBD_NR_OUTPUTS];        // on error: the kbd_output_mode_regs[] is not defined properly!
typedef int kbd_output_reset_regs_check[ (sizeof(kbd_output_reset_data_regs) / sizeof(uint8_t)) == KBD_NR_OUTPUTS]; // on error: the kbd_output_reset_data_regs[] is not defined properly!
typedef int kbd_output_bitmasks_check[ (sizeof(kbd_out_bitmasks) / sizeof(uint16_t)) == KBD_NR_OUTPUTS];            // on error: the kbd_out_bitmasks[] is not defined properly!
typedef int kbd_output_input_mode_regs_check[ (sizeof(kbd_input_mode_regs) / sizeof(uint8_t)) == KBD_NR_INPUTS];    // on error: the kbd_input_mode_regs[] is not defined properly!

#ifdef DELAYED_WAKEUP_ON
#define DELAYED_WAKEUP_GPIO_ROW         (0)
#define DELAYED_WAKEUP_GPIO_COLUMN      (2)
#endif

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (1)

// unknown key code - nothing is sent to the other side but the key is examined for ghosting
#define K_CODE              (0xF4FF)


// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xy means special function (x = no of byte in the report, y no of bit set).

KBD_TYPE_QUALIFIER uint16_t kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] KBD_ARRAY_ATTRIBUTE =
{
  {


/*    No Fn key(s) pressed
      0         1         2         3         4
    COL1       COL2      COL3       COL4      COL5
      ------------------------------------------------
    8-SW20     #-NC      #-NC       #-NC        Play-SW25
    5-SW18     4-SW17    Left-SW5   Select-side Ch+-SW11
    Mute-SW2   Ch--SW13  Motion-SW1 <Spk>-SW9   7-SW21
    Down-SW8   Ok-SW6    6-SW19     9-SW23      3-SW16
    Right-SW7  Vol--SW12 Stop-SW26  2-SW15      Up-SW4
    1-SW14     0-SW22    Rew-SW24   Vol+-SW10   On/Off-SW3

      ------------------------------------------------*/
  { 0x0051,    0x0066,   0x0221,    0x0223,     0x0052 ,0x008D}, // ROW0
  { 0x0224,    0x00E9,   0x009C,    0x0050,     0x0028 ,0x004F}, // ROW1
  { 0x0089,    0x00EA,   0x009D,    0x00E2,     0x003A ,0x01BD}, // ROW2
  { 0x003C,    0x003B,   0x003E,    0xF4FF,     0x003D ,0xF4FF}, // ROW3
  //{ 0x004F,    0xF407,   0xF402,    0x001F,     0x0052 }, // ROW4
  //{ 0x001E,    0x0027,   0xF401,    0xF406,     0xF424 }, // ROW5

  }
};

#ifdef FORCE_CONNECT_TO_HOST_ON

enum {
    MULTI_KEY_ACTION_BOND_TO_HOST1,
    MULTI_KEY_ACTION_BOND_TO_HOST2,
    MULTI_KEY_ACTION_BOND_TO_HOST3,
    MULTI_KEY_ACTION_CONNECT_TO_HOST1,
    MULTI_KEY_ACTION_CONNECT_TO_HOST2,
    MULTI_KEY_ACTION_CONNECT_TO_HOST3,
    MULTI_KEY_ACTION_CLEAR_BONDING_DATA
};
 
const struct multi_key_combinations_t multi_key_combinations[] = {
    {{{4,2},{5,0}},  MULTI_KEY_ACTION_BOND_TO_HOST1},    // Stop+1
    {{{4,2},{4,3}},  MULTI_KEY_ACTION_BOND_TO_HOST2},    // Stop+2
    {{{4,2},{3,4}},  MULTI_KEY_ACTION_BOND_TO_HOST3},    // Stop+3
    {{{5,2},{5,0}},  MULTI_KEY_ACTION_CONNECT_TO_HOST1}, // REW+1
    {{{5,2},{4,3}},  MULTI_KEY_ACTION_CONNECT_TO_HOST2}, // REW+2
    {{{5,2},{3,4}},  MULTI_KEY_ACTION_CONNECT_TO_HOST3},  // REW+3
    {{{4,2},{5,2}},  MULTI_KEY_ACTION_CLEAR_BONDING_DATA}  // REW+Stop
};   
#endif


/// @} APP

#endif //_APP_KBD_MATRIX_SETUP_16_H_
