/**
 * dev.h
 */
 #ifndef _DEV_H_
 #define _DEV_H_

 #include "rs232.h"
 #include "basetype.h"
 #include "list.h"
 #include "dev.h"

#define MAX_DES_NAME 32
struct st_dev;
typedef int( *dev_fun_ptr)(struct st_dev *);

extern struct list_head dev_list;
 struct st_dev{
     int id;
     int type;
     WORD addr;
     int power;
     char des[DEV_MAX_DES];

     dev_fun_ptr dev_reset;
     struct st_rs232 *rs232;
     struct list_head node;
 };

//
 enum{
     DEV_ERR=-1,
     DEV_IDEL=0,
     DEV_BUSY=1
 };

 extern int init_dev(void);
 extern void add_dev( struct st_dev *dev);
 extern void del_dev( struct st_dev *dev);
 extern void dev_reset(int flag, struct st_dev *set_dev);
 extern struct st_dev *seach_dev(int addr);

 #endif
