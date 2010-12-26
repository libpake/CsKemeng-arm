#ifndef _RS232_H_
#define _RS232_H_

#include "basetype.h"
#include "debug.h"
#include "action.h"
#include <termios.h>

/* rs232 funciton code information */
#define RS232_MAX_BUFF	512

#define COM_CLOSE	0x1
#define COM_OPEN	0x2
#define COM_READ	0x3
#define COM_WRITE	0x4
#define	COM_ERR		0xff

#define RS232_S_OPEN	1
#define RS232_S_CLOSE	2
#define RS232_S_WRITE	3
#define RS232_S_READ	4
/*	struct rs232 */
struct st_rs232{
	int	 fd;
	int	 state;
	char device[16];
		
	struct termios old;
	BYTE	snd_buff[RS232_MAX_BUFF];
	int 	snd_size;
	BYTE	rec_buff[RS232_MAX_BUFF];
	int 	rec_size;
};


/* struct pack */

struct st_pack{
    int dev_addr;
    int *ssize, *rsize;
    BYTE *sbuff, *rbuff;
};
extern struct st_rs232 uart1, uart2;
extern int write_uart(struct st_rs232 *prs232, char *buff, int len);
extern int read_uart (struct  st_rs232 *prs232);
extern int open_uart(struct st_rs232 *prs232, char *pstr);
extern void close_uart(void);
extern int sed_pack(struct  st_pack *pack);

/**
 * uart_init -  init uart
 * 
 * @author chenxb (9/24/2010)
 */
extern void uart_init(void);
extern void uart_start(void);

#endif
