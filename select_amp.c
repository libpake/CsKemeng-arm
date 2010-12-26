
#include "select_amp.h"
#include "caller.h"
struct st_select_amp *gselect_amp;
int     select_amp_max;

WORD select_amp_caller[2];

static int select_amp_reset(struct st_dev *dev)
{
    struct st_select_amp *amp;
    struct st_action_unit unit;
    int ret=0,i;
    for( i=1; i < 7; i++ ) {
        bzero(&unit, sizeof unit);
        unit.dev_addr = dev->addr;
        unit.sdata[0]  = i;
        unit.sdata[1]  = 0;
        unit.ssize = 2;
        if( ( ret = dispersion(&unit) ) < 0){
            amp = search_select_amp(dev->addr - SELECT_AMP_BASE_ADDR);
            if( !amp)  return -1;
            amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_ERR ;
         }
     }
    return ret;
}
static struct st_select_amp*  new_select_amp()
{
    struct st_select_amp  *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_select_amp *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

static int add_select_amp(int id, char *des, int power, int addr, int  outline, struct st_rs232 *prs232)
{
    struct st_select_amp select_amp;
    struct st_dev *dev = &(select_amp.dev);
    memset( &select_amp, 0,  sizeof(select_amp));

    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = SELECT_AMP_TYPE;
    dev->addr   = (WORD)(SELECT_AMP_BASE_ADDR + addr);
    dev->power = power;
    dev->dev_reset = select_amp_reset;
	select_amp.select_state[SELECT_AMP_STATE] = SELECT_AMP_IDLE;
    select_amp.out = outline;
    if( !gselect_amp) {
        gselect_amp = new_select_amp();
        if(!gselect_amp)  {
            select_amp_debug(" err, can't malloc new select_amp\n");
            exit(-1);
        }
        memcpy(gselect_amp,  &select_amp, sizeof(select_amp));
        select_amp_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    select_amp.dev.id, select_amp.dev.des, select_amp.dev.rs232->fd);
         dev = &(gselect_amp->dev);
         add_dev(dev);
          return 0;
    }else{
        struct st_select_amp *new=NULL;
        new =  new_select_amp();
        if(!new) {
            select_amp_debug("err, can't malloc new select_amp\n");
            exit(-1);
        }
        memcpy(new, &select_amp, sizeof(select_amp));
        select_amp_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    select_amp.dev.id, select_amp.dev.des, select_amp.dev.rs232->fd);
        {
            struct st_select_amp *pos;
            for(pos = gselect_amp; pos->next; pos = pos->next) ;
            pos->next = new;
            dev  = &(new->dev);
            add_dev(dev);
            return 0;
        }
    }
    return -1;
}

/**
 * select_amp_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int select_amp_load_cfg(INI *ini)
{
     char des[32];
     char buff[32];
     int  i;
    /**/

    select_amp_max =  ini_get_int(ini, SELECT_CONFIG_MAX);
    select_amp_debug("(load_cfg): max = %d\n", select_amp_max);

    for( i =0; i < select_amp_max ; i++) {
        struct st_rs232 *prs232;
        int uart, power, addr, outline;
        /*  load lead_amp_des */
        set_string(buff, SELECT_CONFIG_DES, i);
        select_amp_debug("(load_cfg):  caller des = %s\n", buff);
        ini_get(ini, buff, des);
        /* load lead_amp uart*/
        set_string(buff, SELECT_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
            select_amp_debug("(load_cfg )  number %d ,  set uart err uart = %d\n", select_amp_max, uart);
            return -1;
        }
        /* load select_amp output*/
        set_string(buff, SELECT_CONFIG_OUT, i);
        outline = ini_get_int(ini, buff);
         /* load lead_amp power*/
        set_string(buff, SELECT_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);

		/* load lead_amp addr */
		set_string(buff, SELECT_CONFIG_ADDR, i);
		addr = ini_get_int(ini, buff);
        add_select_amp( i, des, power,addr,outline,   prs232 );
    }
    return 0;
}

struct st_select_amp *search_select_amp(int addr)
{
	struct st_select_amp *pos;

	for( pos = gselect_amp; pos;  pos = pos->next) {
		if( (int)( (WORD)(pos->dev.addr) - SELECT_AMP_BASE_ADDR) == addr)
			return pos;
	}
	return NULL;
}


int set_select_amp_ex( struct st_select_amp *amp,  struct st_action_unit *ac )
{
	if ( !ac || !amp)  return -1;

	ac->dev_addr = amp->dev.addr;
	ac->ssize =2;
	//memcpy( ac->data, (BYTE *)&(amp->config), 2*sizeof(BYTE) );
	select_amp_debug(" ac->data = %x, %x\n", ac->sdata[0], ac->sdata[1]);
	return 0;
}

/**
 *
 *
 * @author chenxb (10/28/2010)
 *
 * @param amp
 * @param control  10 luzhongde yilu
 *
 * @return BYTE
 */
int  select_amp_get_sound_state(struct st_select_amp *amp, int  control)
{
    WORD state;
    state = amp->select_state[SELECT_AMP_STATE_SOUND] ;

    return  ( state & ( 0x1 << (control-1) ));
}

int select_amp_get_sound_state_ex(struct st_select_amp * amp, WORD *pdate, int len)
{
    return 0;
}

int select_amp_load_state(struct st_select_amp *amp)
{
    struct st_action_unit unit;
    int ret = -1;
    bzero(&unit, sizeof(unit));
    //fix GAO Di WEI XiangFan
    unit.sdata[0] = (amp->select_state[SELECT_AMP_STATE_SOUND] & 0xff00) >> 8;
    unit.sdata[1] = (amp->select_state[SELECT_AMP_STATE_SOUND] &0xff);

    caller_debug(" amp->select_state = %x\n", amp->select_state[SELECT_AMP_STATE_SOUND]);
    //memcpy( unit.sdata, &(amp->select_state[SELECT_AMP_STATE_SOUND]), 2*sizeof(BYTE));
    unit.sdata[0] |= 0x80;
    unit.ssize = 2;
    unit.dev_addr = amp->dev.addr;
    ret = dispersion(&unit);
    if (ret < 0) {
        select_amp_debug("load_state: sound err\n");
        return ret;
    }

    bzero(&unit, sizeof(unit));
    unit.sdata[0] = ( amp->select_state[SELECT_AMP_STATE_CALLER ] & 0xff00) >> 8;
    unit.sdata[1] = (amp->select_state[SELECT_AMP_STATE_CALLER] & 0xff);
    unit.dev_addr = amp->dev.addr ;
    unit.ssize = 2;
    ret=dispersion(&unit);
    if (ret < 0) {
        select_amp_debug("load_state: caller err\n");
        return ret;
    }
    return ret;
}


/*   S E L E C T _ A M P _ L O A D _ S T A T E _ E X   */
/*-------------------------------------------------------------------------
    È¡µÃ×´Ì¬
-------------------------------------------------------------------------*/
int select_amp_load_state_ex(struct st_select_amp *amp, WORD *pdate, int len)
{
    return 0;
}

/**
 * seach  select_amp out = outline
 *
 * @author chenxb (10/29/2010)
 *
 * @param outline
 *
 * @return struct st_select_amp*
 */
struct st_select_amp *search_out_select_amp(int outline)
{
    struct st_select_amp *amp;
    for( amp = gselect_amp ; amp ; amp = amp->next) {
        if( outline == amp->out)  return amp;
    }
    select_amp_debug(" Not Find Select_Amp Out-line\n");
    return NULL;
}
static void _save_select_amp( WORD  *pdata,
                                                                    struct st_action_unit *unit)
{
//!! fix diama  sdata[0]  he sdata[1] huhuan
    if( ( unit->sdata[0] & 0x1)  ) {
        *pdata |= ((unit->sdata[0] & 0x1) << 8);
    }else{
        *pdata &= ~(( 0x1) << 8);
    }
    if( unit->sdata[0] & 0x2 ) {
        *pdata |= ( (unit->sdata[0] & 0x2) << 8);
    }else{
        *pdata &= ~((0x2) << 9);
    }

    if( unit->sdata[1] & 0x1) {
        *pdata |= ( unit->sdata[1] & 0x1 );
    }else{
        *pdata &= ~(0x1 );
    }
    if( unit->sdata[1] & 0x2 ) {
        *pdata |= ( unit->sdata[1] & 0x2);
    }else{
        *pdata &= ~(0x2 );
    }
       if( unit->sdata[1] & 0x4 ) {
        *pdata |= ( unit->sdata[1] & 0x4);
    }else{
        *pdata &= ~( 0x4 );
    }
        if( unit->sdata[1] & 0x8 ) {
        *pdata |= ( unit->sdata[1] & 0x8);
    }else{
        *pdata &= ~( 0x8 );
    }
        if( unit->sdata[1] & 0x10 ) {
        *pdata |= ( unit->sdata[1] & 0x10);
    }else{
        *pdata &= ~(0x10 );
    }
        if( unit->sdata[1] & 0x20 ) {
        *pdata |= ( unit->sdata[1] & 0x20);
    }else{
        *pdata &= ~(0x20 );
    }
    if( unit->sdata[1] & 0x40 ) {
        *pdata |= ( unit->sdata[1] & 0x40);
    }else{
        *pdata &= ~(0x40 );
    }
        if( unit->sdata[1] & 0x80 ) {
        *pdata |= ( unit->sdata[1] & 0x80);
    }else{
        *pdata &= ~(0x80 );
    }
}



int save_select_amp_ex(WORD *pdata, int len, struct st_action_unit *unit)
{
    struct st_select_amp *amp;
    amp = search_select_amp(unit->dev_addr - SELECT_AMP_BASE_ADDR);
    if( !amp) {
        select_amp_debug("(save_select_amp):  save state  err! no dev\n");
        return -1;
    }
    if( unit->sdata[0] & 0x80 ) {
        _save_select_amp(&(pdata[1]), unit);
    }else{
        _save_select_amp(&(pdata[0]), unit);
    }
    return 0;
}

static void save_select_amp(struct st_action_unit *unit)
{
    struct st_select_amp *amp;
    amp = search_select_amp(unit->dev_addr - SELECT_AMP_BASE_ADDR);
    if( !amp) {
        select_amp_debug("(save_select_amp):  save state  err! no dev\n");
        return;
    }
    if( unit->sdata[0] & 0x80 ) {
        _save_select_amp(&(amp->select_state[SELECT_AMP_STATE_SOUND]), unit);
    }else{
        _save_select_amp(&(amp->select_state[SELECT_AMP_STATE_CALLER]), unit);
    }
    return;
}


int select_amp_rec_op(struct st_action_unit *unit)
{
    struct st_select_amp *amp;

    amp = search_select_amp(unit->dev_addr - SELECT_AMP_BASE_ADDR);

    if( unit->rsize > 0){
        if( unit->rdata[0] ==0xf9) {
            if( amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_CALLER) {
                save_select_amp_ex(select_amp_caller, 2, unit);
            }else {
                save_select_amp(unit);
            }
            return 0;
        }
    }else{
        /* fix  fasong zhuangtai xia */
                save_select_amp(unit);
                return 0;
    }
    return -1;
}

