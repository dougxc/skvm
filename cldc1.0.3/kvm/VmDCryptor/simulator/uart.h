/*
 *	Header File	: uart.h
 *	Author		: Antony Ng
 *	Date		: 25 Aug, 2000
 *
 * Copyright 2000 D'Crypt Pte. Ltd.
 * All rights reserved.
 *
 *	UART Interface Routines
 *
 *	void	uart_putc(char ch)
 *
 * Puts a character to the UART. THe routine waits until an entry is available
 * in the FIFO. It then transmits the character.
 *
 *	void	uart_putv(unsigned int val, int digits)
 *
 * Prints the unsigned integer "val" to the UART in hexadecimal. "digits" is
 * the number of hexadecimal digits (starting at least significant digit) to
 * print. If the number cannot be represented in the number of digits, the
 * high order digits will be ignored.
 *
 *	void	uart_puts(char* str)
 *
 * Puts a 0-terminated string to the UART. Returns when transmission of entire
 * string is successful. Uses uart_putc for the individual characters.
 *
 *	char	uart_getc()
 *
 * Retrieves a character from the UART. Routine will wait until a character is
 * available.
 *
 *	unsigned int	uart_getv(int digits)
 *
 * Retrieves a hexadecimal value from the UART. The routine waits for exactly
 * "digits" hexadecimal digits. Only '0' to '9' and 'a' to 'f' are supported.
 *	
 * $Id: uart.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 *
 * $Log: uart.h,v $
 * Revision 1.1.1.1  2002/05/21 20:38:47  dougxc
 * All the Secure KVM project stuff under one top level CVS module
 *
 * Revision 1.1.1.1  2002/02/26 00:18:38  dsimon
 * initial import of SVM to CVS
 *
 * Revision 1.1  2000/08/25 01:00:06  anpc
 * Initial revision
 *
 */
#ifndef _UART_H
#define _UART_H

void	uart_putc(char ch);
void	uart_putv(unsigned int val, int digits);
void	uart_puts(char* str);

char		uart_getc(void);
unsigned int	uart_getv(int digits);

int   uart_stat(void);

#endif
