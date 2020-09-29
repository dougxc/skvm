/*
 *	Header File	: flash.h
 *	Author		: Antony Ng
 *	Date		: 25 Aug, 2000
 *
 * Copyright 2000 D'Crypt Pte. Ltd.
 * All rights reserved.
 *
 *	Routines to manipulate flash memory.
 *
 * Note that flash memory is mapped to 0xc000,0000.
 *
 *	int	flash_read(int addr, unsigned char* buf);
 *
 * Read a 128-byte buffer from address "addr" in the Flash to the buffer at
 * "buf". The buffer must be word-aligned since it will be written in words
 * (4 bytes). A zero is returned if successful and a one is returned on error.
 *
 *	int	flash_write(int addr, unsigned char* buf);
 *
 * Write the 128-byte buffer at "buf" to the address "addr" in the Flash. The
 * buffer must be word-aligned since it will be fetched by words (4 bytes).
 * A zero is returned if successful and a one is returned on error.
 *
 *	int	flash_unlock(int addr);
 *
 * Unlock the block at address "addr". A zero is return if successful and a
 * one is returned on error.
 *
 *      int     flash_erase(int addr);
 *
 * Erase the block at address "addr". A zero is returned if successful and a
 * one is returned on error.
 *
 *	int	flash_config(int addr);
 *
 * Retrieve the flash configuration register at the given address.
 *
 *	int	flash_status(int addr);
 *
 * Retrieve the flash status register. The address is a dummy field.
 *
 *	int	flash_clear(int addr);
 *
 * Clear the flash status register.
 *	
 * $Id: flash.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 *
 * $Log: flash.h,v $
 * Revision 1.1.1.1  2002/05/21 20:38:47  dougxc
 * All the Secure KVM project stuff under one top level CVS module
 *
 * Revision 1.1.1.1  2002/02/26 00:18:33  dsimon
 * initial import of SVM to CVS
 *
 * Revision 1.2  2001/01/07 13:47:06  anpc
 * Separated the unlock function from the erase function
 *
 * Revision 1.1  2000/08/25 01:08:49  anpc
 * Initial revision
 *
 */
#ifndef _FLASH_H
#define _FLASH_H

#define	FLASH_BASE	0xc0000000	/* Base address of flash memory */

extern int	flash_read(int addr, unsigned char* buf);
extern int	flash_write(int addr, unsigned char* buf);
extern int	flash_unlock(int addr);
extern int	flash_erase(int addr);
extern int	flash_config(int addr);
extern int	flash_status(int addr);
extern int	flash_clear(int addr);

#endif
