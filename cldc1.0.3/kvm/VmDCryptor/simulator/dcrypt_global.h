/*
 *	Header File	: global.h
 *	Original Author	: K H Ong
 *	Created		: 16 Aug 2000
 *
 * Copyright 2000 D'Crypt Pte Ltd
 * All rights reserved.
 *
 * This file contains global definitions.
 *     
 * $Id: dcrypt_global.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 *
 */
#ifndef _GLOBAL_H
#define _GLOBAL_H

typedef enum _RESULT
{
	ERROR,
	OK
} RESULT ;

#include <sys/types.h>

typedef int     reg_t ;

/*
 * The MASK_GEN macro forms a mask based on the start bit position and the
 * size of the mask.
 *
 * The FMT macro takes a value and formats it for inclusion in a given field.
 */

#define	MASK_GEN(B,N)	(((1<<(N))-1)<<(B))

#define	FMT(D,B,M)	(((D)<<(B))&(M))

/* 
 * Timer ids used in calls to the timer_init(...) function in the timer
 * module. They are defined here to minimize the chances whereby anther module
 * uses an id that has already been used in another module. Each module that
 * uses the timer_init(...) function should define its id here.
 */
typedef enum _TIMER_ID
{
	TIMEOUT_ID_1,		/* For general use in non-ISRs */
	TIMEOUT_ID_2,		/* For general use in non-ISRs */
	TIMEOUT_ID_3,		/* For general use in non-ISRs */
	TIMEOUT_ID_4,		/* For general use in non-ISRs */
	TIMEOUT_ID_5,		/* For general use in non-ISRs */
	TIMEOUT_ID_6,		/* For general use in non-ISRs */
	TIMEOUT_ID_7,		/* For general use in non-ISRs */
	TIMEOUT_ID_8,		/* For general use in non-ISRs */
	NS_TIMEOUT_ID,		/* Used in noise source module */
	RX_TIMEOUT_ID,		/* Used in gsm softmodem module */
	CHG_PAT_TIMEOUT_ID,	/* Used in gsm softmodem module */
	RING_TIMEOUT_ID,	/* Used in gsm softmodem module */
	PLUS_CHAR_TIMEOUT_ID,	/* Used in gsm softmodem module */
	ANSWER_TIMEOUT_ID,	/* Used in voice encryptor module */
	CMDMODE_TIMEOUT_ID,	/* Used in link module */
	NUM_TIMER_IDS
} TIMER_ID ;

#endif
