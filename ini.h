#ifndef _LIBINI_H_
#define _LIBINI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LINE_LEN 128
#define SECT_LEN 64	
#define VINT_LEN 64	
#define VCHR_LEN 64	

#define DEF_SECT_ROOT "General"	

typedef struct ini_item_ {
	char			*key;
	char			*value;
	char			*section;
	struct ini_item_	*next;

} ini_item;

typedef struct {
	char			*fname;
	struct ini_item_	*header;
	struct ini_item_	*tailer;
	int			 length;
} INI;


INI *ini_open(const char *fname);
INI *ini_create(const char *fname);
int ini_save(INI *pini);
void ini_close(INI *pini);
char *ini_get(INI *pini,const char *key,char *value);
int ini_get_int(INI *pini,const char *key);
int ini_set(INI *pini,const char *key,const char *value);
int ini_set_int(INI *pini,const char *key,int value);
int ini_append(INI *pini,const char *key,const char *value);
int ini_append_int(INI *pini,const char *key,int value);
int ini_remove(INI *pini,const char *key);

#endif
