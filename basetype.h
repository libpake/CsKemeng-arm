/* 基础类型 定义*/

#ifndef _BASETYPE_H_
#define _BASETYPE_H_
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

typedef unsigned int   DBWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

typedef void* (*PRSRUN)(void);

extern pthread_mutex_t malloc_flag_mutex ;


#define TRUE  1
#define FALSE 0
#define true 1
#define false 0
#define DEV_MAX_DES     32
#define PLAN_MAX_DES   32
#define msleep(x) usleep(x*1000);


static inline int cf(int k)
{
	int i;
	int cont=1;
	for(i=0; i < k; i++){
		cont = cont *10;
	}
	return cont;
}
static inline  char *otostr(char *buff, int num)
{
	int tmp=num, i=1 ,k =0;
	while( (tmp = tmp/(cf(i))) ) i++;

	if( i == 1 ){
		buff[k++] = num + 48;
		buff[k]   = 0;
		return buff;
	}
	tmp = num;
	while(i >= 1){
		num = tmp;
		tmp = tmp/((cf(i)));
		if( k == 0 && tmp == 0) {
			tmp = num - cf(i)*tmp;
		 i--;
			continue;
		}
		buff[k] = tmp + 48;
		tmp = num - cf(i)*tmp;
		k++; i--;

	}
	buff[k++] = tmp + 48;
	buff[k] = 0;
	return buff;
}



static inline void set_string(char *buff, char *pstr,  int i)
{
    int len;
	char tmp[32];
    len = strlen( pstr );
    strcpy(buff, pstr);

    strcat(buff, otostr(tmp, i));
}
#endif
