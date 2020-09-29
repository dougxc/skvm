/*
 *	Header File	: fx.h
 *	Author		: Antony Ng
 *	Date		: 25 Aug, 2000
 *
 * Copyright 2000 D'Crypt Pte. Ltd.
 * All rights reserved.
 *
 *	Programmer's interface to the FX program loader
 *
 *	void	fx_load(int block)
 *
 * Programs the ROMs starting at block "block". The routine initiates an XModem
 * transfer over the diagnostic UART port. Data received from the port via this
 * transfer is programmed into the ROM starting at block "block". Each block is
 * 128KB.
 *
 * fx_load does not return. Indeed, the entire program could be blown away and
 * overwritten by this routine.
 *
 * $Id: fx.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 *
 * $Log: fx.h,v $
 * Revision 1.1.1.1  2002/05/21 20:38:47  dougxc
 * All the Secure KVM project stuff under one top level CVS module
 *
 * Revision 1.1.1.1  2002/02/26 00:18:34  dsimon
 * initial import of SVM to CVS
 *
 * Revision 1.1  2000/08/25 01:03:20  anpc
 * Initial revision
 *
 */
#ifndef _FX_H
#define _FX_H

void	fx_load(int block);

#endif
