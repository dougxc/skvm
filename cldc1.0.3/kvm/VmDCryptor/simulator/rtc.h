/*
 *	Header File	: rtc.h
 *	Original Author	: K H Ong
 *	Created		: 16 Aug 00
 *
 * Copyright 2000 D'Crypt Pte Ltd
 * All rights reserved.
 *
 * This module provides the API function prototypes for programming the
 * real-time clock DS1670.
 *
 * $Id: rtc.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
 */
#ifndef rtc_h
#define rtc_h

#include "dcrypt_global.h"

typedef enum _DAYS
{
	MONDAY=1,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY
} DAYS ;

#define GET_DAY_STR(DAY_NDX)	(DAY_NDX == MONDAY) ? "Monday" : \
				(DAY_NDX == TUESDAY) ? "Tuesday" : \
				(DAY_NDX == WEDNESDAY) ? "Wednesday" : \
				(DAY_NDX == THURSDAY) ? "Thursday" : \
				(DAY_NDX == FRIDAY) ? "Friday" : \
				(DAY_NDX == SATURDAY) ? "Saturday" : \
				(DAY_NDX == SUNDAY) ? "Sunday" : "Unknown"

typedef struct _RTC_TIME
{
	uint	secs ;
	uint	mins ;
	/* 1-12 for 12-hour mode, 0-23 for 24-hour mode */
	uint	hrs ;
	DAYS	day ;
	uint	date ;
	uint	mth ;
	uint	yr ;
	/* Set to 1 for 12-hour mode and 0 for 24-hour mode */
	int	mode_12hr ;
	/* Set to 1 for pm and 0 for am (only for 12-hour mode) */
	int	mode_pm ;
} RTC_TIME ;

/* 
 * Shared variable for transferring data between this module and the calling
 * module when the following functions are called:
 *
 * void rtc_get_time(void) ;
 * void rtc_set_time(void) ;
 */
extern RTC_TIME	rtc_time_shared ;

/* 
 * Initialize the RTC.
 * Should be called once before any other function.
 */
extern void	rtc_init(void) ;

/* Get the time/date in the RTC */
extern void	rtc_get_time(void) ;

/* Set the time/date in the RTC */
extern void	rtc_set_time(void) ;

/* Enable the oscillator in the RTC to start timekeeping */
extern void	rtc_start_oscr(void) ;

/* Disable the oscillator in the RTC to stop timekeeping */
extern void	rtc_stop_oscr(void) ;

#endif
