

#include "power.h"


struct st_power *gpower;
int     power_max;


static struct st_power*  new_power()
{
    struct st_power  *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_power *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

static int add_power(int id, char *des, int ipower,int addr,  struct st_rs232 *prs232)
{
     struct st_power power;
    struct st_dev *dev = &(power.dev);
    memset( &power, 0,  sizeof(power));

    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = POWER_TYPE;
    dev->addr   = (WORD)(POWER_BASE_ADDR +addr);
    dev->power = ipower;

    if( !gpower) {
        gpower = new_power();
        if(!gpower)  {
            power_debug(" err, can't malloc new power\n");
            exit(-1);
        }
        memcpy(gpower,  &power, sizeof(power));
        power_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    power.dev.id, power.dev.des, power.dev.rs232->fd);
         dev = &(gpower->dev);
         add_dev(dev);
          return 0;
    }else{
        struct st_power *new=NULL;
        new =  new_power();
        if(!new) {
            power_debug("err, can't malloc new power\n");
            exit(-1);
        }
        memcpy(new, &power, sizeof(power));
        power_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    power.dev.id, power.dev.des, power.dev.rs232->fd);
        gpower->next = new;
        dev  = &(new->dev);
        add_dev(dev);
        return 0;
    }
    return -1;
}

/**
 * power_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int power_load_cfg(INI *ini)
{
     char des[32];
    char buff[32];
    int  i;
    /**/

    power_max =  ini_get_int(ini, POWER_CONFIG_MAX);
    power_debug("(load_cfg): max = %d\n", power_max);

    for( i =0; i < power_max ; i++) {
        struct st_rs232 *prs232;
        int uart, ipower, addr;
        /*  load power_des */
        set_string(buff, POWER_CONFIG_DES, i);
        power_debug("(load_cfg):  power des = %s\n", buff);
        ini_get(ini, buff, des);
        /* load power uart*/
        set_string(buff, POWER_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
            power_debug("(load_cfg )  number %d ,  set uart err uart = %d\n", power_max, uart);
            return -1;
        }
         /* load power power*/
        set_string(buff, POWER_CONFIG_POWER, i);
        ipower = ini_get_int(ini, buff);

        /* load addr power*/
        set_string(buff, POWER_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);
        add_power( i, des, ipower, addr, prs232 );
    }
    return 0;
}
