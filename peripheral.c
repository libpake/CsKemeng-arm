#include "peripheral.h"
#include "action.h"

struct st_peripheral *gperipheral;
int     per_max;

static int peripheral_reset(struct st_dev *dev)
{
    struct st_action_unit unit;
    struct st_peripheral *peripheral;
    int ret=-1;
    bzero(&unit, 0);

    /* reset cd */
    unit.dev_addr = dev->addr;
    unit.sdata[0]  = 0x29;
    unit.sdata[1]  = 0x26;
    unit.ssize = 2;
    if ( (ret = dispersion(&unit) < 0)) {
        peripheral = search_peripheral(dev->addr - PERIPHERAL_BASE_ADDR);
        if( !peripheral ) return -1;
 //       peripheral->state = DEV_ERR;
    }else{
 //       peripheral->state = DEV_IDEL;
    }

    return ret;
}
static struct st_peripheral*  new_per()
{
    struct st_peripheral  *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_peripheral *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

static int add_per(int id, char *des, int power, int addr, struct st_rs232 *prs232)
{
    struct st_peripheral per;
    struct st_dev *dev = &(per.dev);
    memset( &per, 0,  sizeof(per));

    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = PER_TYPE;
    dev->addr   = (WORD)(PERIPHERAL_BASE_ADDR + addr );
    dev->power = power;
    dev->dev_reset = peripheral_reset;
    if( !gperipheral) {
        gperipheral = new_per();
        if(!gperipheral)  {
            peripheral_debug(" err, can't malloc new peripheral\n");
            exit(-1);
        }
        memcpy(gperipheral,  &per, sizeof(per));
        peripheral_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    per.dev.id, per.dev.des, per.dev.rs232->fd);
         dev = &(gperipheral->dev);
         add_dev(dev);
         return 0;
    }else{
        struct st_peripheral *new=NULL;
        new =  new_per();
        if(!new) {
            peripheral_debug("err, can't malloc new peripheral\n");
            exit(-1);
        }
        memcpy(new, &per, sizeof(per));
        gperipheral->next = new;
        dev  = &(new->dev);
        add_dev(dev);
        return 0;
    }
    return -1;
}

/**
 * peripheral_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int peripheral_load_cfg(INI *ini)
{
     char des[32];
    char buff[32];
    int  i;
    /**/
    per_max =  ini_get_int(ini, PER_CONFIG_MAX);
    peripheral_debug("(load_cfg): max = %d\n", per_max);

    for( i =0; i < per_max ; i++) {
        struct st_rs232 *prs232;
        int uart, power, addr;
        /*  load per_des */
        set_string(buff, PER_CONFIG_DES, i);
        peripheral_debug("(load_cfg):  lead_amp des = %s\n", buff);
        ini_get(ini, buff, des);

        /* load per uart*/
        set_string(buff, PER_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
             peripheral_debug("(load_cfg )  number %d ,  set uart err\n", per_max);
            return -1;
        }
         /* load per power*/
        set_string(buff, PER_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);
          /* load per addr */
        set_string(buff, PER_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);
        add_per( i, des, power, addr, prs232 );
    }

    return 0;
}

struct st_peripheral *search_peripheral(int addr)
{
    struct st_peripheral *pos = NULL;

    for( pos = gperipheral; pos; pos = pos->next) {
        if( (pos->dev.addr - PERIPHERAL_BASE_ADDR) == addr)
            break;
        }
    return pos;
}

int PERIPHERAL_STATE;

int peripheral_op(int fn, int addr)
{
    struct st_peripheral *peri = NULL;
    int ret = -1,i;
    struct st_action_unit unit;
    peri = search_peripheral(addr);
    if( !peri ) {
        peripheral_debug("peripheral_op: err no dev\n");
        return -1;
    }
    switch( fn ) {
    case PERIPHERAL_SEND_ONLINE:
        bzero(&unit, sizeof(unit));
        unit.sdata[0] = 0x27;
        unit.sdata[1] = 0x26;
        unit.ssize      = 2;
        unit.dev_addr= peri->dev.addr;
        ret = dispersion(&unit);
        if( ret < 0) {
            peripheral_debug("peripher_op: SEND_ONLINE err\n");
        }

        break;
    case PERIPHERAL_SEND_SUM:
         bzero(&unit, sizeof(unit));
        unit.sdata[0] = 0x28;
        unit.sdata[1] = 0x26;
        unit.ssize      = 2;
        unit.dev_addr= peri->dev.addr;
        ret = dispersion(&unit);
        if( ret < 0) {
            peripheral_debug("peripher_op: SEND_ONLINE err\n");
        }

        break;
    case PERIPHERAL_SEND_RESET:
        bzero(&unit, sizeof(unit));
        unit.sdata[0] = 0x29;
        unit.sdata[1] = 0x26;
        unit.ssize      = 2;
        unit.dev_addr= peri->dev.addr;
        ret = dispersion(&unit);
        if( ret < 0) {
            peripheral_debug("peripher_op: SEND_ONLINE err\n");
        }

        break;
    case     PERIPHERAL_SEND_ENABLE:
        bzero(&unit, sizeof(unit));
        unit.sdata[0] = 0x00;
        unit.sdata[1] = 0x25;
        unit.ssize      = 2;
        unit.dev_addr= peri->dev.addr;
        ret = dispersion(&unit);
        if( ret < 0) {
            peripheral_debug("peripher_op: SEND_ONLINE err\n");
        }

        break;
    case PERIPHERAL_SEND_ADD_LIST:
        for(i=0; i < PERIPHERAL_CD_MAX; i++) {
            bzero(&unit, sizeof(unit));

//            peri->peripheral_state[ i+ 2]; // fix!!
            if( peri->peripheral_state[ i+2] ) {
                unit.sdata[0] = i+2;
                unit.sdata[1] = peri->peripheral_state[i+2];
                unit.ssize = 2;
                unit.dev_addr = peri->dev.addr;
                ret = dispersion(&unit);
                if( ret < 0) {
                    peripheral_debug("peripher_op: SEND_ONLINE err\n");
                    break;
                }
            }
            break;
         }

        break;
    default:
        peripheral_debug("peripheral_op:  no fn \n");
        return -1;
    }
    return ret;
}

int peripheral_rec_op(struct st_action_unit *unit)
{
    switch(PERIPHERAL_STATE ) {
    case PERIPHERAL_SEND_ONLINE:
         if( unit->rdata[0] == 0xf9) {
            return 0;
        }else{
            return -1;
        }
        break;
    case PERIPHERAL_SEND_SUM:
        if( unit->rdata[0] == 0x18) {
            return (int)(unit->rdata[1]);
        }else{
            return -1;
        }
        break;
    case PERIPHERAL_SEND_RESET:
         if( unit->rdata[0] == 0xf9) {
            return 0;
        }else{
            return -1;
        }
        break;
    case PERIPHERAL_SEND_ENABLE:
         if( unit->rdata[0] == 0xf9) {
            return 0;
        }else{
            return -1;
        }
        break;
    }
return 0;
}
