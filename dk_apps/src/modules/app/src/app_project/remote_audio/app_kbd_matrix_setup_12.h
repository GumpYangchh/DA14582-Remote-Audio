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


#ifndef _APP_KBD_MATRIX_SETUP_12_H_
#define _APP_KBD_MATRIX_SETUP_12_H_

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


#define KBD_NR_INPUTS       (5)
#define KBD_NR_OUTPUTS      (6)

#define COLUMN_0_PORT		(2)	//port: 2, pin: 7
#define COLUMN_0_PIN		(7)

#define COLUMN_1_PORT		(2)	//port: 2, pin: 8
#define COLUMN_1_PIN		(8)

#define COLUMN_2_PORT		(2)	//port: 2, pin: 9
#define COLUMN_2_PIN		(9)

#define COLUMN_3_PORT		(3)	//port: 3, pin: 0
#define COLUMN_3_PIN		(0)

#define COLUMN_4_PORT		(3)	//port: 3, pin: 1
#define COLUMN_4_PIN		(1)

// used for cycle optimization
#define P0_HAS_INPUT        (0)
#define P1_HAS_INPUT        (0)
#define P2_HAS_INPUT        (1)
#define P3_HAS_INPUT        (1)

#define ROW_0_PORT			(3)	//port: 3, pin: 2
#define ROW_0_PIN			(2)

#define ROW_1_PORT			(3)	//port: 3, pin: 3
#define ROW_1_PIN			(3)

#define ROW_2_PORT			(3)	//port: 3, pin: 4
#define ROW_2_PIN			(4)

#define ROW_3_PORT			(3)	//port: 3, pin: 5
#define ROW_3_PIN			(5)

#define ROW_4_PORT			(3)	//port: 3, pin: 6
#define ROW_4_PIN			(6)

#define ROW_5_PORT			(3)	//port: 3, pin: 7
#define ROW_5_PIN			(7)


// Masks for the initialization of the KBD controller
#define MASK_P0				(0x4000	| SET_MASK0_FROM_COLUMN(0)                | SET_MASK0_FROM_COLUMN(1)                | SET_MASK0_FROM_COLUMN(2)                | SET_MASK0_FROM_COLUMN(3)                \
									| SET_MASK0_FROM_COLUMN(4)                )
uint16_t mask_p0 = MASK_P0;

#define MASK_P12			(0x0000	| SET_MASK12_FROM_COLUMN(0)               | SET_MASK12_FROM_COLUMN(1)               | SET_MASK12_FROM_COLUMN(2)               | SET_MASK12_FROM_COLUMN(3)               \
									| SET_MASK12_FROM_COLUMN(4)               )
uint16_t mask_p12 = MASK_P12;

#define MASK_P3				(0x0000	| SET_MASK3_FROM_COLUMN(0)                | SET_MASK3_FROM_COLUMN(1)                | SET_MASK3_FROM_COLUMN(2)                | SET_MASK3_FROM_COLUMN(3)                \
									| SET_MASK3_FROM_COLUMN(4)                )
uint16_t mask_p3 = MASK_P3;

// Masks for the initialization of the WKUP controller
#define WKUP_MASK_P0		(	SET_WKUP_MASK_FROM_COLUMN(0, 0)         | SET_WKUP_MASK_FROM_COLUMN(0, 1)         | SET_WKUP_MASK_FROM_COLUMN(0, 2)         | SET_WKUP_MASK_FROM_COLUMN(0, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(0, 4)         )
uint16_t wkup_mask_p0 = WKUP_MASK_P0;

#define WKUP_MASK_P1		(	SET_WKUP_MASK_FROM_COLUMN(1, 0)         | SET_WKUP_MASK_FROM_COLUMN(1, 1)         | SET_WKUP_MASK_FROM_COLUMN(1, 2)         | SET_WKUP_MASK_FROM_COLUMN(1, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(1, 4)         )
uint16_t wkup_mask_p1 = WKUP_MASK_P1;

#define WKUP_MASK_P2		(	SET_WKUP_MASK_FROM_COLUMN(2, 0)         | SET_WKUP_MASK_FROM_COLUMN(2, 1)         | SET_WKUP_MASK_FROM_COLUMN(2, 2)         | SET_WKUP_MASK_FROM_COLUMN(2, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(2, 4)         )
uint16_t wkup_mask_p2 = WKUP_MASK_P2;

#define WKUP_MASK_P3		(	SET_WKUP_MASK_FROM_COLUMN(3, 0)         | SET_WKUP_MASK_FROM_COLUMN(3, 1)         | SET_WKUP_MASK_FROM_COLUMN(3, 2)         | SET_WKUP_MASK_FROM_COLUMN(3, 3)         \
							  | SET_WKUP_MASK_FROM_COLUMN(3, 4)         )
uint16_t wkup_mask_p3 = WKUP_MASK_P3;

KBD_TYPE_QUALIFIER uint8_t kbd_input_ports[] KBD_ARRAY_ATTRIBUTE =
{
	COL(0),                     // column 0 (P2[7])
	COL(1),                     // column 1
	COL(2),                     // column 2
	COL(3),                     // column 3
	COL(4)                      // column 4
};

KBD_TYPE_QUALIFIER uint8_t kbd_output_mode_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_OUTPUT_MODE_REG(0),
	SET_OUTPUT_MODE_REG(1),
	SET_OUTPUT_MODE_REG(2),
	SET_OUTPUT_MODE_REG(3),
	SET_OUTPUT_MODE_REG(4),
	SET_OUTPUT_MODE_REG(5)
};

KBD_TYPE_QUALIFIER uint8_t kbd_output_reset_data_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_RESET_REG(0),
	SET_RESET_REG(1),
	SET_RESET_REG(2),
	SET_RESET_REG(3),
	SET_RESET_REG(4),
	SET_RESET_REG(5)
};

KBD_TYPE_QUALIFIER uint16_t kbd_out_bitmasks[] KBD_ARRAY_ATTRIBUTE =
{
	SET_BITMAP(0),
	SET_BITMAP(1),
	SET_BITMAP(2),
	SET_BITMAP(3),
	SET_BITMAP(4),
	SET_BITMAP(5)
};

KBD_TYPE_QUALIFIER uint8_t kbd_input_mode_regs[] KBD_ARRAY_ATTRIBUTE =
{
	SET_INPUT_MODE_REG(0),
	SET_INPUT_MODE_REG(1),
	SET_INPUT_MODE_REG(2),
	SET_INPUT_MODE_REG(3),
	SET_INPUT_MODE_REG(4),
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
      0          1         2         3         4
    COL1        COL2      COL3       COL4      COL5
      ------------------------------------------------
    Motion-SW1   #-NC      #-NC       #-NC     Mute-SW2
    On/Off-SW3  Up-SW4    Left-SW5   Ok-SW6    Right-SW7
    Down-SW8    <Spk>-SW9 Vol+-SW10  Ch+-SW11  Vol--SW12
    Ch--SW13    1-SW14    2-SW15     3-SW16    4-SW17
    5-SW18      6-SW19    7-SW20     8-SW21    9-SW22
    0-SW23      Rew-SW24  Play-SW25  Fwd-SW26  Stop-SW27
      ------------------------------------------------*/
//{ 0x0010,     0xF4FF,   0xF4FF,    0xF4FF,   0xF405 }, // ROW0
  { 0xF4F5,     0xF4FF,   0xF4FF,    0xF4FF,   0xF405 }, // ROW0
  { 0xF424,     0x0052,   0x0050,    0x0028,   0x004F }, // ROW1
  { 0x0051,     0xF4F4,   0xF406,    0x0057,   0xF407 }, // ROW2
  { 0x0056,     0x001E,   0x001F,    0x0020,   0x0021 }, // ROW3
  { 0x0022,     0x0023,   0x0024,    0x0025,   0x0026 }, // ROW4
  { 0x0027,     0xF401,   0xF404,    0xF400,   0xF402 }, // ROW5

  }
};

#ifdef MULTI_KEY_COMBINATIONS_ON

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
    {{{5,4},{3,1}},  MULTI_KEY_ACTION_BOND_TO_HOST1},    // Stop+1
    {{{5,4},{3,2}},  MULTI_KEY_ACTION_BOND_TO_HOST2},    // Stop+2
    {{{5,4},{3,3}},  MULTI_KEY_ACTION_BOND_TO_HOST3},    // Stop+3
    {{{5,1},{3,1}},  MULTI_KEY_ACTION_CONNECT_TO_HOST1}, // REW+1
    {{{5,1},{3,2}},  MULTI_KEY_ACTION_CONNECT_TO_HOST2}, // REW+2
    {{{5,1},{3,3}},  MULTI_KEY_ACTION_CONNECT_TO_HOST3},  // REW+3
    {{{5,1},{5,4}},  MULTI_KEY_ACTION_CLEAR_BONDING_DATA}  // REW+Stop
};   
#endif

/// @} APP

#endif //_APP_KBD_MATRIX_SETUP_12_H_
