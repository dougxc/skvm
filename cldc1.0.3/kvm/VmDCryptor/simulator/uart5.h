/*****************************************************************************/
/*                                                                           */
/*  Header for uart5.c                                                       */
/*                                                                           */
/*  Copyright 2000, D'Crypt Pte Ltd                                          */
/*  All Rights Reserved                                                      */
/*                                                                           */
/*  File   : uart5.h                                                         */
/*  Created: 000828                                                          */
/*  Author : Victor Low                                                      */
/*                                                                           */
/*****************************************************************************/

#ifndef __UART5__
#define __UART5__


extern void uart5_init(void);
extern void uart5_enable(void);
extern void uart5_disable(void);
extern unsigned uart5_getc(void);
extern void uart5_putc(unsigned char);
extern int uart5_getc_rdy(void);
extern int uart5_putc_rdy(void);
extern int uart5_get_dtr(void);
extern int uart5_get_rts(void);
extern void uart5_put_cts(int);
extern void uart5_put_dsr(int);
extern void uart5_put_ri(int);
extern void uart5_put_dcd(int);

extern int	pcmcia_online(void);
#endif


/*****************************************************************************/
/*                                                                           */
/* EOF uart5.h                                                               */
/*                                                                           */
/*****************************************************************************/
