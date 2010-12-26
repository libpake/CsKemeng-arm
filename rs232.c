#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "rs232.h"
#include "basetype.h"
#include "dev.h"

struct st_rs232 uart1, uart2;
/**
 * _open - open rs232
 *
 * @author chenxb (9/24/2010)
 *
 * @param struct st_rs232 fprs232
 *
 * @return int  err ruturn -1,  OK return 0;
 */
static int _open(struct st_rs232 *prs232)
{
	int fd;
	struct termios Termios;
	if(!prs232) return (-1);

	memset(&Termios, 0, sizeof(Termios));
    if(prs232->device[0] == 0) return ( -1);

    fd = open(prs232->device, O_RDWR | O_NOCTTY | O_NDELAY );// | O_NDELAY);//| O_NOCTTY | O_NONBLOCK );//| O_NDELAY);

    if(fd < 0 ){
        rs232_debug("err open\n");
        exit(-1) ;
    }
#if 0
	//Termios.c_cflag = CS8 | CSTOPB;
    Termios.c_cflag |= CLOCAL | CREAD;
    Termios.c_cflag &= ~CSIZE;
    Termios.c_cflag |= CS8;
    Termios.c_cflag  |= CSTOPB ;

    Termios.c_cflag &= ~PARENB;
#endif
	Termios.c_cflag = CS8  |  CLOCAL | CREAD | CSTOPB;
	Termios.c_iflag = IGNPAR;
	Termios.c_oflag = 0;
	Termios.c_lflag = 0;
    Termios.c_oflag &=
	/*  TIME OUT */
	Termios.c_cc[VTIME] = 10;
	Termios.c_cc[VMIN]  = 1;

        cfsetispeed(&Termios, B2400);
        cfsetospeed(&Termios, B2400);


	//Termios.c_oflag = 0;
	//Termios.c_lflag = 0;
	/*  TIME OUT */

	tcgetattr(fd, &(prs232->old));
	tcflush(fd, TCIOFLUSH);
	if ( tcsetattr(fd, TCSANOW, &Termios) != 0){
        rs232_debug("(open_rs232): err! \n");
        return -1;
    }

	prs232->fd  = fd;
    prs232->state = RS232_S_OPEN;

	memset( prs232->rec_buff, 0, RS232_MAX_BUFF);
	memset( prs232->snd_buff,0, RS232_MAX_BUFF);
	return RS232_S_OPEN;
}


static int _close(struct st_rs232 *fprs232)
{
	if( !fprs232 ) return (-RS232_S_CLOSE);

	close(fprs232->fd);
	fprs232->fd 	  = 0;
	fprs232->state = RS232_S_CLOSE;
	memset( fprs232->rec_buff, 0, RS232_MAX_BUFF);
	memset( fprs232->snd_buff,0, RS232_MAX_BUFF);
	return RS232_S_CLOSE;
}


static int _recv(struct st_rs232 *prs232)
{
	int  fd;
	int  startflag;
	BYTE c;
	BYTE	start = 0xfc;
	BYTE	end   = 0xfe;

	fd 			  = prs232->fd;
	startflag = 0;
	if( !prs232 || prs232->state != RS232_S_WRITE)  return RS232_S_READ;
{
    unsigned char tmp[MAX_ACTION_RBUFF + 4];
    BYTE data=0;
    int rec_size, i =0;
    int count=2;
    //bzero( tmp, MAX_ACTION_RBUFF + 4);
    //usleep(50000);
    /* FIX 可能要重新设置 open 的时候的VMIN */
rep:
   bzero( tmp, MAX_ACTION_RBUFF + 4);
    while( read(fd, &c, 1 ) > 0) {
        tmp[i] = c;
        i++;
        if( i == 1 && tmp[0] != 0xfc) {
            i = 0; continue;
        }
    }
    if ( i == 0 && count) {
        usleep(5000);
        count--;
        goto rep;
      }else if ( i == 0 && !count) {
        tcflush(fd, TCIOFLUSH);
        return -1;
    }
    	tcflush(fd, TCIOFLUSH);
     rec_size = i;
    #ifdef RS232_DEBUG
    printf("rec_data: ");
        for(i=0; i < rec_size; i++) {
            printf(" %x ", tmp[i]);
        }
        printf("\n");
     #endif

    if( tmp[0] != start) {
              rs232_debug(" (start err):rec_len \n"
                          );
              return ( -RS232_S_READ);
     }
     if( tmp[1] != prs232->snd_buff[1]) {
           rs232_debug(" sed addr=%x, rec addr=%x\n", prs232->snd_buff[1], tmp[1]);
           return ( -RS232_S_READ);
     }
     for( i=0; i < rec_size-3; i++ )
         data += tmp[i+1];

     if( (data & 0x7f) != tmp[rec_size-2]) {
         rs232_debug(" (check err): check = %x, rec_check=%x \n",data & 0x7f , tmp[rec_size-2]);
         return ( -RS232_S_READ);
     }
     if( tmp[rec_size-1] != end) {
         rs232_debug(" (end flag err):  %x \n", tmp[rec_size-1]);
         return ( -RS232_S_READ);
     }
     memcpy( prs232->rec_buff,  tmp, rec_size);
     prs232->rec_size = rec_size;
     return RS232_S_READ;
}
}

#define MAX_O 3
static int _write( struct st_rs232 *fprs232)
{
	int fd, ret, size, i=0;
	BYTE *pbuff;

	if( !fprs232 && fprs232->state != RS232_S_OPEN )  return ( -RS232_S_WRITE);

	pbuff = fprs232->snd_buff;
	size   = fprs232->snd_size;
	fd	     = fprs232->fd;

	while( TRUE) {
		ret = write( fd, pbuff, size) ;
		if( ret == size)  {
			fprs232->state = RS232_S_WRITE;
			return RS232_S_WRITE;
		} else if ( ( ret <0) && ( i++ < MAX_O) )  {
			continue;
		} else{
			fprs232->state = (-RS232_S_WRITE);
			return  ( -RS232_S_WRITE);
		}
	}

	return (-RS232_S_WRITE);
}
#undef  MAX_O


static int Op(char cFunction, struct st_rs232 *fprs232)
{
	int iStatus;

	switch(cFunction)
	{
		case COM_OPEN:
			iStatus = _open(fprs232); break;
		case COM_CLOSE:
			iStatus = _close(fprs232); break;
		case COM_READ:
			iStatus = _recv(fprs232); break;
		case  COM_WRITE:
			iStatus = _write(fprs232); break;
		default:
			iStatus = (unsigned int)COM_ERR; break;
	}

	return iStatus;
}

int write_uart( struct st_rs232 *fprs232, char *buff, int len)
{
	if( !fprs232)  return -1;

	memcpy( fprs232->snd_buff,  buff, len);
	fprs232->snd_size = len;
	return Op( COM_WRITE, fprs232);

}

int read_uart( struct st_rs232 *fprs232)
{
	memset(fprs232->rec_buff, 0, RS232_MAX_BUFF);

	return  Op(COM_READ, fprs232);
}

int open_uart(struct st_rs232 *pUart, char *pstr)
{
	memset(pUart, 0, sizeof(*pUart));
	memcpy(pUart->device, pstr, strlen(pstr)+1);
	return Op(COM_OPEN, pUart);
}

void close_uart()
{
    close(uart1.fd);
}
void uart_start()
{

	//getcfg   fix!!
//    if( (ret = open_uart(&(uart2), "/dev/ttyS0")) == -1){
//        rs232_debug("err open uart1");
//	}
//ret = open_uart((&uart1), "/dev/ttyS0");
//uart2 = uart1;
#if 1
	int ret;
    if( (ret = open_uart(&(uart1), "/dev/ttySAC1")) == -1){
        rs232_debug("err open uart1");
	}
    if( (ret = open_uart(&(uart2), "/dev/ttySAC2"))  == -1) {
         rs232_debug("err open uart2");
    }
#endif
}

int sed_pack(struct  st_pack *pack)
{
#if 1
    BYTE buff[MAX_ACTION_SBUFF + 4];
    int ret;
    int addr = pack->dev_addr;
    int send_max = *(pack->ssize);
    struct st_dev *dev;
    struct st_rs232 *rs232;
    dev = seach_dev(addr);
    if(!dev) {
        rs232_debug("(sed_pack): can't find dev\n");
        return -1;
    }

    rs232 = dev->rs232;
    if (!rs232)  return -1;
{
    int i;
    BYTE data = 0;
    buff[0] = 0xFA;
    buff[1] = dev->addr;
    for(i=0; i < send_max ; i++){
        buff[i + 2] = pack->sbuff[i];
        data += buff[i + 2];
    }

    buff[i+2] = (buff[1] + data)&0x7f;
    buff[i+3] = 0xFD;
    #ifdef RS232_DEBUG
    rs232_debug("(sed_pack): ");
    for( i=0 ; i < send_max + 4; i++) {
        printf(" %x", buff[i]);
    }
    printf("\n");
    #endif
}

    ret = write_uart(rs232 , (char *)buff, send_max + 4 );
    if(ret < 0) {
        rs232_debug(" (sed_pack):  write_uart  err! -> ret = %d\n", ret);
        return  -1;
    }
   //sleep(1);
    msleep(500);
    ret = read_uart( rs232);
    if( ret < 0) {
        rs232_debug("(sed_pack):  read_uart err ! -> ret = %d\n", ret);
        return -1;
    }

    memcpy( pack->rbuff, &(rs232->rec_buff[2]),  rs232->rec_size -4);
    *(pack->rsize) = rs232->rec_size-4;
    #endif
    return 0;
}

