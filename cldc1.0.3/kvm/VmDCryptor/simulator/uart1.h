/*****************************************************************************/
/*                                                                           
*/
/* SOF uart1.h - vlys                                                        
*/
/*                                                                           
*/
/*****************************************************************************/

#ifndef __UART1__
#define __UART1__

#define BAUD_230400 ((unsigned long) 0x00)
#define BAUD_115200 ((unsigned long) 0x01)
#define BAUD_57600 ((unsigned long) 0x03)
#define BAUD_38400 ((unsigned long) 0x05)
#define BAUD_19200 ((unsigned long) 0x0b)
#define BAUD_14400 ((unsigned long) 0x0f)
#define BAUD_9600 ((unsigned long) 0x17)
#define BAUD_4800 ((unsigned long) 0x2f)
#define BAUD_2400 ((unsigned long) 0x5f)
#define BAUD_1200 ((unsigned long) 0xbf)

typedef enum {HW_FLOW, SW_FLOW, NO_FLOW} uart_t;

extern void uart1_put_dtr(int);
extern void uart1_put_rts(int);
extern int uart1_get_cts(void);
extern int uart1_get_dcd(void);
extern int uart1_get_dsr(void);
extern int uart1_get_ri(void);
extern void uart1_init(int, uart_t);
extern void uart1_enable(void);
extern void uart1_disable(void);
extern void uart1_putc(char);
extern char uart1_getc(void);
extern int uart1_putc_rdy(void);
extern int uart1_getc_rdy(void);
extern int uart1_putc_avail(void);
extern int uart1_getc_avail(void);

#endif

/*****************************************************************************/
/*                                                                           
*/
/*  EOF uart1.h - vlys                                                       
*/
/*                                                                           
*/
/*****************************************************************************/

