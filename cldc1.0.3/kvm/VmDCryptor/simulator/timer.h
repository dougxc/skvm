/*
 *	Header File	: timer.h
 *	Original Author	: K H Ong
 *	Created		: 16 Aug 2000
 *
 * Copyright 2000 D'Crypt Pte Ltd
 * All rights reserved.
 *
 * This file provides the function prototypes for performing timer functions.
 *
 * $Id: timer.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 *
 */
#ifndef _TIMER_H
#define _TIMER_H

#include "dcrypt_global.h"

/*
 * OS Timer Counter Register is clocked by the 3.6864 MHz oscillator.
 * Therefore, number of ticks per millisecond is 3686.4 (rounded to 3686
 * to avoid floating-point calculation).
 */
#define TICKS_PER_MS	3686

/*
 * OS Timer Status Register
 */

#define OSSR_M0_B	0
#define OSSR_M0_N	1
#define OSSR_M1_B	1	
#define OSSR_M1_N	1
#define OSSR_M2_B	2	
#define OSSR_M2_N	1
#define OSSR_M3_B	3
#define OSSR_M3_N	1

#define OSSR_M0_M      MASK_GEN(OSSR_M0_B,OSSR_M0_N)
#define OSSR_M1_M      MASK_GEN(OSSR_M1_B,OSSR_M1_N)
#define OSSR_M2_M      MASK_GEN(OSSR_M2_B,OSSR_M2_N)
#define OSSR_M3_M      MASK_GEN(OSSR_M3_B,OSSR_M3_N)

/*
 * OS Timer Interrupt Enable Register 
 */

#define OIER_E0_B	0
#define OIER_E0_N	1
#define OIER_E1_B	1
#define OIER_E1_N	1
#define OIER_E2_B	2
#define OIER_E2_N	1
#define OIER_E3_B	3
#define OIER_E3_N	1

#define OIER_E0_M      MASK_GEN(OIER_E0_B,OIER_E0_N)
#define OIER_E1_M      MASK_GEN(OIER_E1_B,OIER_E1_N)
#define OIER_E2_M      MASK_GEN(OIER_E2_B,OIER_E2_N)
#define OIER_E3_M      MASK_GEN(OIER_E3_B,OIER_E3_N)

/* Call this function at the start of an event that we are interested to time */
extern void	timer_start(void) ;

/*
 * Call this function at the end of the event to retrieve the elapsed time 
 * in microseconds.
 */
extern uint	timer_end(void) ;

/* 
 * Call this function to store the current timer value for later use. The
 * timer value is stored into an array indexed by id. A maximum of
 * NUM_TIMER_IDS values can be stored. After this function is called, the 
 * timer_timeout(...) function can be called with the same id to determine 
 * whether a specified number of milliseconds has elapsed since the last call 
 * to this function.
 */
extern RESULT	timer_init (TIMER_ID id) ;

/*
 * Return OK when a specified number of milliseconds has elapsed since the
 * last call to timer_init(...), ERROR otherwise. The timer_init(...) function 
 * should always be called once before calling this function. See commments
 * for timer_init(...) function on the use of the id.
 */
extern RESULT	timer_timeout (TIMER_ID id, uint timeout) ;

/* Delay for a specified number of microseconds */
extern void	timer_delay_us (uint delay) ;

/* Delay for a specified number of milliseconds */
extern void	timer_delay_ms (uint delay) ;

typedef struct _OSTIMER
{
	volatile reg_t	osmr0 ;	/* at physical address 0x90000000 */
	volatile reg_t	osmr1 ; /* at physical address 0x90000004 */
	volatile reg_t	osmr2 ;	/* at physical address 0x90000008 */
	volatile reg_t	osmr3 ;	/* at physical address 0x9000000c */
	volatile reg_t	oscr ;	/* at physical address 0x90000010 */
	volatile reg_t	ossr ;	/* at physical address 0x90000014 */
	volatile reg_t	ower ;	/* at physical address 0x90000018 */
	volatile reg_t	oier ;	/* at physical address 0x9000001c */
} OSTIMER ;

/* Variable that will be assigned to a proper memory location by the builder */
extern OSTIMER	ostimer ;	/* mapped to physical address 0x90000000 size 4K */

#endif
