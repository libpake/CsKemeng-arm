#include "head_amp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ini.h"
#include "dev.h"
#include "config.h"


#define AMP_TYPE 0x70

struct st_lead_amp *glead_amp;
int             lead_amp_max;

extern  int lead_amp_set_default();
static int head_amp_reset(struct st_dev *dev)
{
    return lead_amp_set_default();

}
/**
 * new_lead_amp  -  malloc struct leadamp;
 * @author chenxb (9/24/2010)
 *
 * @return lead_amp*
 */
static struct st_lead_amp*  new_lead_amp()
{
    struct st_lead_amp *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_lead_amp *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

/**
 *  add_lead_amp - add struct *lead_amp to list
 *
 * @author chenxb (9/24/2010)
 *
 * @param id    唯一标志符号
 * @param des 描述整个设备
 *
 * @return int  0 表示添加成功
 */
int add_lead_amp(int id,  char *des, int power, int addr, struct st_rs232 *prs232, struct st_lead_amp_sound *connive)
{
    struct st_lead_amp amp;
    struct st_dev *dev = &(amp.dev);
    memset( &amp, 0,  sizeof(amp));
    int k;
    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = AMP_TYPE;
    dev->addr   = (WORD)(LEAD_AMP_BASE_ADDR + addr);
    dev->power = power;
    dev->dev_reset = head_amp_reset;

    if( !glead_amp) {
        glead_amp = new_lead_amp();
        if(!glead_amp)  {
            lead_amp_debug(" err, can't malloc new lead_amp\n");
            exit(-1);
        }
        memcpy(glead_amp,  &amp, sizeof(amp));
         lead_amp_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    glead_amp->dev.id, glead_amp->dev.des, glead_amp->dev.rs232->fd);
         dev = &(glead_amp->dev);
         for(k=0; k < 6; k++) {
             glead_amp->connive[k].height = connive[k].height;
             glead_amp->connive[k].low     = connive[k].low;
             glead_amp->connive[k].vol      = connive[k].vol;
         }
         glead_amp->lead_amp_state = DEV_IDEL;
         add_dev(dev);
          return 0;
    }else{
        struct st_lead_amp *new=NULL;
        new =  new_lead_amp();
        if(!new) {
            lead_amp_debug("err, can't malloc new lead_amp\n");
            exit(-1);
        }
        memcpy(new, &amp, sizeof(amp));
         for(k=0; k < 6; k++) {
             glead_amp->connive[k].height = connive[k].height;
             glead_amp->connive[k].low     = connive[k].low;
             glead_amp->connive[k].vol      = connive[k].vol;
         }
        glead_amp->next = new;
        new->lead_amp_state = DEV_IDEL;
        add_dev(dev);
        return 0;
    }

}

/**
 * get_lead_amp_max - get lead_amp max
 *
 * @author chenxb (9/24/2010)
 *
 * @return int  max
 */
int get_lead_amp_max()
{
    struct st_lead_amp *p;
    int i;
    for( p = glead_amp, i=0; p != NULL; p = p->next)  i++;
    return i;
}


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
int lead_amp_set(int id, BYTE  channel, int vol, int tvol, int lvol )
{

 #if 0
 struct st_lead_amp *p = glead_amp;
    if( !p)  return;
    while( p != NULL ) {
        struct st_dev *dev = &(p->dev);

        if( dev->id == id ) {
            struct st_lead_amp_config *config = (struct st_lead_amp_config *)&(p->dev.data);
            int ret;
            memset( config, 0, sizeof(*config));
            config->channel = channel;
            if( vol  != -1 )  config->vol   = vol;
            if( tvol != -1)  config->tvol = tvol;
            if( lvol != -1)  config->lvol  = lvol;
            ret =  sed_pack( dev);
            if( ret < 0)  return ret;
            else {
                BYTE *buff = dev->rs232->rec_buff;
                int     size    = dev->rs232->rec_size;
                if( buff[2] == 0xf9) return 0;
                else if ( buff[2] == 0xf8)  return -1;
            }
        }
        p = p->next;
    }
#endif
	return 0;
}

/**
 * lead_amp_load_cfg -  load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @return int   0 is true -1 is false
 */
int lead_amp_load_cfg(INI *ini)
{
    char des[32];
    char buff[32];
    int  i, k;
    /*  读取这个设备的个数并保存在全局变量里面*/
    lead_amp_max =  ini_get_int(ini, HEAD_AMP_CONFIG_MAX);
    lead_amp_debug("(load_cfg): max = %d\n", lead_amp_max);
    i = 0;
    for( i =0; i < lead_amp_max ; i++) {
        struct st_lead_amp_sound  connive[8]={{7,7,7}, {7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}};
        struct st_rs232 *prs232;
        int power, uart,addr;
        /*  load lead_amp_des */
        set_string(buff, HEAD_AMP_CONFIG_DES, i);
        lead_amp_debug("(load_cfg):  lead_amp des = %s\n", buff);
        ini_get(ini, buff, des);
        /* load lead_amp uart*/
        set_string(buff, HEAD_AMP_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = & uart2;
        }else {
             lead_amp_debug("(lead_amp_cfg )  number %d ,  set uart err\n", lead_amp_max);
            return -1;
        }
        /* load lead_amp power*/
        set_string(buff, HEAD_AMP_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);

        /* load lead_amp addr */
        set_string(buff, HEAD_AMP_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);

        /* load connive sound */
        for(k=0; k < 6; k++) {
            char tmpstr1[32];
            char tmpstr2[32];
            strcpy(tmpstr1, HEAD_AMP_CONFIG_CONNIVE);
            set_string(tmpstr2, tmpstr1, k);
            strcat(tmpstr2, "_H");
            set_string(buff, tmpstr2,i);
            connive[k].height = ini_get_int(ini, buff);

            strcpy(tmpstr1, HEAD_AMP_CONFIG_CONNIVE);
            set_string(tmpstr2,  tmpstr1, k);
            strcat(tmpstr2, "_L");
            set_string(buff, tmpstr2,i);
            connive[k].low = ini_get_int(ini, buff);

            strcpy(tmpstr1, HEAD_AMP_CONFIG_CONNIVE);
            set_string(tmpstr2, tmpstr1, k) ;
            strcat(tmpstr2, "_V");
            set_string(buff, tmpstr2,i);
            connive[k].vol  = ini_get_int(ini, buff);
        }
        /* all */
        add_lead_amp( i, des, power, addr,  prs232, connive );
    }
    return 0;
}
//*fix 扩充后 会造成 设置不正确的问题
int lead_amp_save_ini(INI *ini,struct st_lead_amp *amp, int num_connive, struct  st_lead_amp_sound *connive)
{
    char buff[32], setbuff[32];

    if( num_connive >6 )  return -1;
    set_string(buff, HEAD_AMP_CONFIG_CONNIVE, num_connive);
    strcat(buff, "_H");
    set_string(setbuff, buff, 0); /* 这里不应该是0 要扩展head—amp 出错*/

    ini_set_int(ini, setbuff, connive->height);
    set_string(buff, HEAD_AMP_CONFIG_CONNIVE, num_connive);
    strcat(buff, "_L");
    set_string(setbuff, buff, 0); /* 这里不应该是0 要扩展head—amp 出错*/
    ini_set_int(ini, setbuff, connive->low);

    set_string(buff, HEAD_AMP_CONFIG_CONNIVE,num_connive);
    strcat(buff, "_V");
    set_string(setbuff, buff, 0); /* 这里不应该是0 要扩展head—amp 出错*/
    ini_set_int(ini, setbuff, connive->vol);
    return 0;

}
int lead_amp_load_sound(struct st_lead_amp *amp, int num_connive, struct st_lead_amp_sound *connive)
{
    struct st_action_unit unit;
    int ret;
    INI *ini;
    bzero(&unit, sizeof unit);
    /* 分成两步*/

    /* 第一步发送 connive 到设备  */
    unit.dev_addr = amp->dev.addr;
    /* head_amp cong 1 kaishi*/
    unit.sdata[0] = ((num_connive+1) << 4) | (connive->vol);
    unit.sdata[1] = ((connive->height) << 4) | ( connive->low);
    unit.ssize = 2;
    ret = dispersion(&unit);
    
    /* 第二步 保存 connive 到配置文件*/
   // if( ret < 0) {
   //     printf("set vol err\n");
   //    return ret;
   // }
	memcpy( &(amp->connive[num_connive]), connive, sizeof *connive);
    printf(" num = %d, vol = %d, low =%d, height=%d\n", num_connive, connive->vol, connive->low, connive->height);
    ini = ini_open(DEV_INI);
    ret = lead_amp_save_ini(ini, amp,num_connive,connive);
    ini_save(ini);
    ini_close(ini);
    return ret;

}
struct st_lead_amp *seach_lead_amp(int addr)
{
    struct st_lead_amp *amp;
    for(amp=glead_amp; amp; amp= amp->next) {
        if( addr == ( amp->dev.addr - LEAD_AMP_BASE_ADDR) )
            return amp;
    }
    return NULL;
}
int lead_amp_set_default()
{
    struct st_action_unit unit;
    struct st_lead_amp *amp;
    int k, ret = -1;
    bzero( &unit, sizeof(unit));

    for( amp=glead_amp; amp;  amp=amp->next) {
        for( k=0; k < 6; k++) {
            unit.dev_addr = amp->dev.addr;
            unit.sdata[0] =  ((k+1) << 4) | (amp->connive[k].vol);
            unit.sdata[1] =  ((amp->connive[k].height) << 4) | (amp->connive[k].low );
            unit.ssize =2;
            ret = dispersion(&unit);
        }
    }
    return ret;
}

int lead_amp_rec_op( struct st_action_unit *unit)
{
    if( unit->rdata[0] != 0xf9) {
        struct st_lead_amp *amp = seach_lead_amp(unit->dev_addr - LEAD_AMP_BASE_ADDR);
        if(!amp ) return -1;
        amp->lead_amp_state = DEV_ERR;
        return -1;
    }
    return 0;
}

