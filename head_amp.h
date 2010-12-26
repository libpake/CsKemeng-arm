/*    功放  头文件定义 */

#ifndef _HEAD_AMP_H_
#define _HEAD_AMP_H_

#include "basetype.h"
#include "list.h"
#include "rs232.h"
#include "ini.h"
#include "dev.h"
#include "debug.h"




#define  LEAD_AMP_BASE_ADDR  0x70

/* config */
/* 目前只是用于配置一个的宏*/
#define HEAD_AMP_CONFIG_MAX "HEAD_AMP.MAX"
#define HEAD_AMP_CONFIG_DES "HEAD_AMP.DES"
#define HEAD_AMP_CONFIG_POWER "HEAD_AMP.POWER"
#define HEAD_AMP_CONFIG_UART "HEAD_AMP.UART"
#define HEAD_AMP_CONFIG_ADDR "HEAD_AMP.ADDR"
#define HEAD_AMP_CONFIG_CONNIVE "HEAD_AMP.CONNIVE"


struct st_lead_amp_sound{
    BYTE height;
    BYTE low;
    BYTE vol;
};
struct st_lead_amp{
    struct st_dev               dev;
    BYTE lead_amp_state;
    struct st_lead_amp_sound  connive[6];
    struct st_lead_amp *next;
};

extern struct st_lead_amp *glead_amp;


/**
 * get_lead_amp_max - get lead_amp max
 * 
 * @author chenxb (9/24/2010)
 * 
 * @return int  max
 */
extern int get_lead_amp_max(void);
/**
 * lead_amp_set - 设置lead_amp 的值
 * 
 * @author chenxb (9/26/2010)
 * 
 * @param id    amp.id 
 * @param channel  channel of set
 * @param vol          vol  of set
 * @param tvol         top-vol of set
 * @param lvol          low-vol of set 
 * @return int           0 is true -1 is err 
 */
extern int lead_amp_set(int id, BYTE  channel, int vol, int tvol, int lvol );
/**
 * lead_amp_load_cfg -  load_cfg 
 * 
 * @author chenxb (9/26/2010)
 * 
 * @return int   0 is true -1 is false
 */
extern int lead_amp_load_cfg(INI *ini);
extern int lead_amp_rec_op( struct st_action_unit *unit);

extern int lead_amp_rec_op( struct st_action_unit *unit);

extern int lead_amp_load_sound(struct st_lead_amp *amp, int num_connive, struct st_lead_amp_sound *connive);
#endif
