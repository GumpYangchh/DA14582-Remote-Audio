/**
 ****************************************************************************************
 *
 * @file app_console.h
 *
 * @brief Application utility functions header file.
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

#ifndef _APP_CONSOLE_H_
#define _APP_CONSOLE_H_


// printf() functionality
#if defined (CFG_PRINTF)

#include <stdarg.h>

typedef struct __print_msg {
	char *pBuf;
	struct __print_msg *pNext;
} printf_msg;

typedef enum {
   ST_INIT,
   ST_NORMAL,
   ST_PERCENT,
   ST_NUM,
   ST_QUAL,
   ST_TYPE
} printf_state_t;

void printint(unsigned long val, int sign, int width, char fill, int base);

void printstr(const char *s, int width);

void arch_puts(const char *s);

int arch_vprintf(const char *fmt, va_list args);

int arch_printf(const char *fmt, ...);

#ifndef putchar
#define putchar(c)                              __putchar(c)
#endif

void arch_printf_process(void);

#else // CFG_PRINTF

#define arch_puts(s) {}
#define arch_vprintf(fmt, args) {}
#define arch_printf(fmt, args...) {}
#define arch_printf_process() {}    
void print_hex (unsigned char val, unsigned char delim);
    
#endif // CFG_PRINTF

void print_hex_wr (unsigned char val, unsigned char delim);
    
#endif // _APP_CONSOLE_H_
