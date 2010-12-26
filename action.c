#include "action.h"
#include "ini.h"
#include "debug.h"
//#include "plan.h"
#include "time.h"
#include "dev.h"
#include  "caller.h"
#include "matrix.h"
#include "select_amp.h"
#include "playsound.h"
#include "peripheral.h"
#include "head_amp.h"
#include "timer.h"
int gac_max;
struct st_action *action_list;
pthread_cond_t disp_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t disp_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t disp_sem;
int disp_num=0;
/**
 *
 *
 * @author chenxb (10/12/2010)
 *
 * @param inbuff
 * @param outbuff
 * @param len
 * malloc outbuff;
 * @return int  strlen( char)；
 */
static int  dec(char *inbuff, char ** outbuff)
{

    char *iter, *curr = inbuff;
    int i=0;
    for( iter=inbuff, i=0; *iter; iter++ , i++  ) {
        if( *iter == ';' ) {
			pthread_mutex_lock(&malloc_flag_mutex);
			*outbuff = malloc(i*sizeof(char)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			if( !(*outbuff)) return -1;
            strncpy( *outbuff, curr, i );
            (*outbuff)[i]=0;
        	return i;
	}
    }
    if( i != 0){
		pthread_mutex_lock(&malloc_flag_mutex);
		*outbuff = malloc(i*sizeof(char)+1);
		pthread_mutex_unlock(&malloc_flag_mutex);
		if( !(*outbuff)) return -1;
		strncpy( *outbuff, curr, i+1);
	}
    return  (i);
}
/**
 * hexval  -> str
 *
 * @author chenxb (11/7/2010)
 *
 * @param val
 * @param buff
 *
 * @return int
 */
static void  hexvaltostr(int val, char *buff)
{
    int  i=0, tmp, j=0;
    bzero(buff, 64);
	if( val < 0) return;
	if (val == 0 ) {
        buff[j] = 48;
	}
    while(TRUE) {
        if( !( (val >> i*4) & 0xf) && !( val >> i*4 ) ) break;
        i++;
    }
    while( i) {
        i--;
        tmp = ( val >> i*4 ) & 0xf;
        if ( tmp >= 0xa && tmp <= 0xf ) {
            buff[j++] = tmp + 87;
        }else if ( tmp <= 9 && tmp >=0) {
            buff[j++] = tmp + 48;
        }
    }
}
static int hexstrtoval(const char *pStr)
{
    unsigned int val = 0;

    while (*pStr != '\0' )
    {
        if (*pStr >= '0' && *pStr <= '9' ) {
            val =val <<4;
            val |= *pStr - '0';
        }else if ( *pStr >= 'a' && *pStr <= 'f' ) {
            val =val << 4;
            val |= *pStr - 'a' + 10;
        }else if ( *pStr >= 'A' && *pStr <= 'F' ){
            val =val << 4;
            val |= *pStr - 'A' +10;
        }else{
            return -1;
        }
        pStr++;
    }
    return val;
}

/* add_ac */
int add_ac(struct list_head *list,  BYTE *buff)
{
    struct st_action_unit *action_unit;
    BYTE  *iter;
    int len=0, count=0;

	pthread_mutex_lock(&malloc_flag_mutex);
    action_unit = (struct st_action_unit *)malloc( sizeof(struct st_action_unit));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if( !action_unit) {
        action_debug("(add_ac): err! can't malloc\n");
        return -1;
    }

	bzero(action_unit, sizeof(*action_unit));
    #if 1
   {
    int curr_int=0;
	BYTE *mp3sdata= NULL;
	char *tmp = NULL;
	BYTE sdata[32];
	memset(sdata, 0, 32);
    for(iter = buff; *iter; count++ ) {
        len = dec((char *)iter, &tmp);
		if( len ==0)  break;
		if( len == -1) return -1;
		if( hexstrtoval((const char *)tmp) >= 0) {
        	sdata[curr_int] = (BYTE)hexstrtoval((const char *)tmp);
	    	curr_int++;
    	}else if(hexstrtoval((const char *)tmp) < 0){
    		int len = strlen(tmp);
			pthread_mutex_lock(&malloc_flag_mutex);
    		mp3sdata = malloc( len +1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			if (!mp3sdata) {
				pthread_mutex_lock(&malloc_flag_mutex);
				free(tmp);
				pthread_mutex_unlock(&malloc_flag_mutex);
				return -1;
			}
			bzero(mp3sdata , len+1);
			strcpy((char *)mp3sdata, (const char *)tmp);
    	}
    	if (!*(iter + len)) {
            pthread_mutex_lock(&malloc_flag_mutex);
			free(tmp);
			pthread_mutex_unlock(&malloc_flag_mutex);
			tmp = NULL;
            break;
    	}
    	iter += (len)+1;
		if (tmp != NULL){
			pthread_mutex_lock(&malloc_flag_mutex);
			free(tmp);
			pthread_mutex_unlock(&malloc_flag_mutex);
			tmp = NULL;
		}
	 }

	action_unit->dev_addr = sdata[0];

	if ( !mp3sdata){
   		memcpy( action_unit->sdata, &(sdata[1]), curr_int-1);
   		action_unit->ssize = curr_int-1;
	}else{
		action_unit->sdata[0] = sdata[1];
		action_unit->mp3sdata = mp3sdata;
		action_unit->ssize = curr_int-1;
	}
   }

    #endif

    list_add(&(action_unit->node), list);

    return 0;
}
#define MAX_SEGMENT 100
int get_ac_segment(INI *ini)
{
    int max,i,id;
    char buff[64];
    max = ini_get_int(ini, ACTION_CONFIG_ACTION_MAX);
    for(i=0; i < max; i++) {
        set_string(buff, ACTION_CONFIG_ACTION,i );
        strcat(buff, ACTION_CONFIG_ID);
        id = ini_get_int(ini, buff);
        if (id == -1)  return i;
    }

    return -1;
}

int get_ac_id()
{
    int i;
    struct st_action  *ac;

    for( i=1; i < MAX_ACTION; i++) {
        for(ac=action_list; ac ; ac=ac->next) {
           if( i == ac->id )  break;
        }
        if( !ac)  return i;
    }
    return -1;
}

/**
 *
 *
 * @author chenxb (11/7/2010)
 *
 * @param ini
 * @param id   segment->ID
 *
 * @return int  NUM: action(NUM)
 */
int ac_del_segment_id_cfg( INI *ini, int id)
{
    int max, i, retnum= -1;
    char buff[64];
    max = ini_get_int(ini, ACTION_CONFIG_ACTION_MAX);

    for( i=0; i < max; i++) {
        set_string(buff, ACTION_CONFIG_ACTION, i );
        strcat( buff, ACTION_CONFIG_ID);
        if ( id == ini_get_int(ini, buff)){
            ini_set_int(ini, buff, -1);
            set_string(buff, ACTION_CONFIG_ACTION, i);
            strcat( buff, ACTION_CONFIG_AC_MAX );
            ini_set_int(ini, buff, 0);
            retnum = i;
        }
    }
    return retnum;
}
/**
 *
 *
 * @author chenxb (11/7/2010)
 *
 * @param ini
 * @param num  action(num)
 *
 * @return int
 */
int ac_del_segment_num_cfg(INI *ini, int num)
{
    int max;
    char buff[64];

    max = ini_get_int(ini, ACTION_CONFIG_ACTION_MAX);
    if( num > max-1) {
        action_debug("del_segment_cfg: err\n");
        return -1;
    }
    set_string(buff,  ACTION_CONFIG_ACTION, num );
    strcat( buff,  ACTION_CONFIG_ID);
    ini_set_int(ini,  buff, -1);

    set_string(buff, ACTION_CONFIG_ACTION, num);
    strcat(buff,  ACTION_CONFIG_AC_MAX);
    ini_set_int(ini, buff, 0);

     return 0;
}

int ac_add_segment_byid( INI *ini, int id, const char *des, unsigned int gap)
{
    int max;
    char buff[64];
    int num, retnum = -1;

    num = get_ac_segment(ini);

    if( num < 0) {
        // new segment

        if (id > 0) {
            char segbuff[512];
            max = ini_get_int(ini,  ACTION_CONFIG_ACTION_MAX);
            if( max >=MAX_ACTION)  return -1; // 最多支持100
            set_string(buff,  ACTION_CONFIG_ACTION, max );
            strcpy(segbuff, buff);
            strcat(buff, ".DES");
            ini_append(ini, buff, des);

            set_string(buff, ACTION_CONFIG_ACTION, max);
            strcat(buff, ACTION_CONFIG_AC_MAX);
            ini_append_int(ini, buff, 0);

            set_string(buff, ACTION_CONFIG_ACTION,max);
            strcat(buff, ACTION_CONFIG_ID);
            ini_append_int(ini, buff, id);

			set_string(buff, ACTION_CONFIG_ACTION,max);
			strcat(buff, ACTION_CONFIG_GAP);
			ini_append_int(ini, buff, gap);

            ini_set_int(ini, ACTION_CONFIG_ACTION_MAX, ++max);
            retnum = max-1;
        }
    }else{
        if( id > 0) {
            set_string(buff,  ACTION_CONFIG_ACTION, num);
            strcat(buff, ACTION_CONFIG_ID);
            ini_set_int(ini, buff, id);
            set_string(buff, ACTION_CONFIG_ACTION, num);
            strcat(buff, ACTION_CONFIG_DES);
            ini_set(ini, buff, des);

			set_string(buff, ACTION_CONFIG_ACTION, num);
			strcat(buff, ACTION_CONFIG_GAP);
			ini_set_int(ini, buff, gap);
            retnum = num;
        }
    }

    //fix
    return retnum;
 }
/**
 * zai lianbiaoli  he wenjian li  dou xuyao xiugai
 *
 * @author chenxb (11/7/2010)
 *
 * @param ini
 * @param des
 *
 * @return int
 */
int ac_add_segment_cfg( INI *ini,  const char *des )
{
    int max;
    char buff[64];
    int id,num, retnum = -1;

    num = get_ac_segment(ini);

    if( num < 0) {
        // new segment
        id = get_ac_id();

        if (id > 0) {
            char segbuff[512];
            max = ini_get_int(ini,  ACTION_CONFIG_ACTION_MAX);
            if( max >=100 )  return -1; // 最多支持100
            set_string(buff,  ACTION_CONFIG_ACTION, max );
            strcpy(segbuff, buff);
            strcat(buff, ".DES");
            ini_append(ini, buff, des);

            set_string(buff, ACTION_CONFIG_ACTION, max);
            strcat(buff, ACTION_CONFIG_AC_MAX);
            ini_append_int(ini, buff, 0);

            set_string(buff, ACTION_CONFIG_ACTION,max);
            strcat(buff, ACTION_CONFIG_ID);
            ini_append_int(ini, buff, id);

            ini_set_int(ini, ACTION_CONFIG_ACTION_MAX, ++max);
            retnum = max-1;
        }
    }else{
        id = get_ac_id();
        if( id > 0) {
            set_string(buff,  ACTION_CONFIG_ACTION, num);
            strcat(buff, ACTION_CONFIG_ID);
            ini_set_int(ini, buff, id);
            set_string(buff, ACTION_CONFIG_ACTION, num);
            strcat(buff, ACTION_CONFIG_DES);
            ini_set(ini, buff, des);
            ini_save(ini);
            retnum = num;
        }
    }

    //fix
    return retnum;
}

int ac_id_to_num(INI *ini, int id)
{
    int max, i, retnum = -1;
    char buff[64];

    max = ini_get_int(ini,  ACTION_CONFIG_ACTION_MAX);
    for( i=0; i < max; i++ ) {
        set_string(buff, ACTION_CONFIG_ACTION, i  );
        strcat(buff,  ACTION_CONFIG_ID);
        if ( id == ini_get_int(ini, buff)){
            retnum = i; break;
        }
    }

    return retnum;
}

int ac_num_to_id( INI *ini, int num)
{
    char buff[64];
    set_string( buff,  ACTION_CONFIG_ACTION, num);
    strcat(buff, ACTION_CONFIG_ID );

    return ( ini_get_int(ini, buff));
}

/**
 *
 *
 * @author pake (11/15/2010)
 *
 * @param ini
 * @param id   action->id
 * @param unit  action->ac_list { action->unit }
 *
 * @return int
 */
int ac_add_key_cfg(INI *ini,  int id, struct st_action_unit *unit)
{
    int i, ret=-1;
    int num, ac_max;
    char buff[64], tmpstr[64];
	char *setbuff;

	if ( !unit->mp3sdata){
		pthread_mutex_lock(&malloc_flag_mutex);
		setbuff = malloc((unit->ssize +1 )*3 + 1);   //fix zheli cuowu
		pthread_mutex_unlock(&malloc_flag_mutex);
		if ( !setbuff)  return -1;
		bzero( setbuff, (unit->ssize +1 )*3 +1 );
	}else{
		int len;
		len = strlen((char *)(unit->mp3sdata)) +1 + (unit->ssize +1)*3+1;
		pthread_mutex_lock(&malloc_flag_mutex);

		setbuff = malloc(len);
		pthread_mutex_unlock(&malloc_flag_mutex);
		if ( !setbuff ) return -1;
		bzero( setbuff, len);
	}

    bzero(buff, 64); bzero( tmpstr, 64);
    num = ac_id_to_num(ini, id);
    if(num < 0)  {
		pthread_mutex_lock(&malloc_flag_mutex);
		free(setbuff);
		pthread_mutex_unlock(&malloc_flag_mutex);
		return -1;
    }
    set_string(buff, ACTION_CONFIG_ACTION, num);
    strcat(buff,  ACTION_CONFIG_AC_MAX);
    ac_max = ini_get_int(ini, buff);

    set_string(buff,  ACTION_CONFIG_ACTION, num);
    strcat(buff, ACTION_CONFIG_AC);
    set_string(buff,ACTION_CONFIG_AC, ac_max); /* cong 0 kaishi*/

    hexvaltostr(unit->dev_addr,  tmpstr);
    strcat(setbuff, tmpstr);
    strcat(setbuff, ";");
    if( unit->dev_addr >= 0xd0 && unit->dev_addr <= 0xdf ) {
        BYTE *ptr = &(unit->sdata[1]);
        if (!ptr)  return -1;
        hexvaltostr(unit->sdata[0], tmpstr);
        strcat(setbuff, tmpstr);
        strcat(setbuff, ";");
		if ( unit->mp3sdata){
			strcat( setbuff, (char *)unit->mp3sdata);
        }
    }else{
        for( i=0; i < unit->ssize; i++) {
            hexvaltostr( unit->sdata[i], tmpstr);
            strcat( setbuff, tmpstr);
            if ( i +1 < unit->ssize) strcat( setbuff, ";");
        }
    }
    set_string(tmpstr, ACTION_CONFIG_ACTION, num);
    strcat(tmpstr, ACTION_CONFIG_AC);
    set_string(buff,tmpstr , ac_max);
    ret = ini_set(ini, buff, setbuff);
    if( ret == 0) ret= ini_append(ini, buff, setbuff);
    set_string(buff,  ACTION_CONFIG_ACTION, num);
    strcat(buff, ACTION_CONFIG_AC_MAX );
    ini_set_int(ini, buff, ++ac_max);

    return ret;
}

int ac_load_cfg(INI *ini)
{
    char buff[32];
    int  i;
	char *tmpa;


    gac_max = ini_get_int(ini, ACTION_CONFIG_ACTION_MAX);
    if( gac_max > MAX_ACTION) {
        action_debug("(ac_load_cfg): err  gac_max > %d\n", MAX_ACTION );
        return -1;
    }
    //action_debug("(ac_load_cfg): max = %d\n", ac_max);
	pthread_mutex_lock(&malloc_flag_mutex);
	tmpa = (char *)malloc(20480*sizeof(char));
	pthread_mutex_unlock(&malloc_flag_mutex);
	if (!tmpa) return -1;

    for( i = 0; i < gac_max; i++) {
        char des[32];
        int max, id, j;
		unsigned int gap;
        struct st_action *ac_now;
        struct list_head *action_unit_list;

        set_string(buff, ACTION_CONFIG_ACTION, i);
        strcat(buff,  ACTION_CONFIG_ID);
        id = ini_get_int(ini, buff);
        if( -1 == id) continue;
        //id

        if( !action_list) {
			pthread_mutex_lock(&malloc_flag_mutex);
            action_list = (struct st_action *)malloc(sizeof(*action_list));
			pthread_mutex_unlock(&malloc_flag_mutex);
            if(!action_list) {
                action_debug("(ac_load_cfg): err !,   malloc\n");
				pthread_mutex_lock(&malloc_flag_mutex);
				free(tmpa);
				pthread_mutex_unlock(&malloc_flag_mutex);
                return -1;
            }
            memset( action_list, 0, sizeof(*action_list));
            ac_now = action_list;
        }else{
        	pthread_mutex_lock(&malloc_flag_mutex);
            ac_now = (struct st_action *)malloc( sizeof(*ac_now));
			pthread_mutex_unlock(&malloc_flag_mutex);
            if( !ac_now) {
                action_debug("(ac_now): err!, malloc\n");
				pthread_mutex_lock(&malloc_flag_mutex);
				free(tmpa);
				pthread_mutex_unlock(&malloc_flag_mutex);
                return -1;
            }
            memset(ac_now, 0, sizeof(*ac_now));
            {
                struct st_action *tmp;
                for( tmp= action_list; tmp->next; tmp=tmp->next) ;
                tmp->next = ac_now;
            }
        }

        action_unit_list = &(ac_now->aclist);

        INIT_LIST_HEAD(action_unit_list);

        set_string( buff, ACTION_CONFIG_ACTION, i);
        strcat(buff, ACTION_CONFIG_DES);
        ini_get(ini, buff, des);
        //action_debug("(ac_load_cfg): buff = %s, des = %s\n", buff, des);

        set_string( buff, ACTION_CONFIG_ACTION, i);
        strcat(buff, ACTION_CONFIG_AC_MAX);
        max =ini_get_int(ini, buff);
        //action_debug("(ac_load_cfg): buff = %s, ac_max= %d\n", buff, max);

		set_string(buff, ACTION_CONFIG_ACTION, i);
		strcat(buff, ACTION_CONFIG_GAP);
		gap = ini_get_int(ini, buff);

        for( j=0 ; j < max; j++) {
            char tmpb[2];
            tmpb[0]= j+48;
            tmpb[1]=0;
            set_string(buff, ACTION_CONFIG_ACTION,i);
            strcat(buff, ACTION_CONFIG_AC);
            strcat(buff, tmpb);
            ini_get(ini, buff, tmpa);
            action_debug("(ac_load_cfg): buff = %s, value = %s\n", buff, tmpa);
            add_ac(action_unit_list, (BYTE *)tmpa);

        }

        /* ac_now*/
        ac_now->id = id;
		ac_now->gap = gap;
        strcpy(ac_now->des, des);
        ac_now->next = NULL;
    }
	pthread_mutex_lock(&malloc_flag_mutex);
	free(tmpa);
	pthread_mutex_unlock(&malloc_flag_mutex);
    return 0;
}

static struct st_action *seach_action(struct st_plan_action *plan_action)
{
    struct st_action *pos;
    for( pos=action_list; pos != NULL;  pos = pos->next) {
        if(plan_action->id == pos->id ) {
            return pos;
        }
    }
    return NULL;
}

static int fill_send_rec_pack (struct st_action_unit *unit)
{
    int ret = -1;
    struct st_pack pack;
    bzero(&pack, sizeof(pack));

    pack.dev_addr = unit->dev_addr;
    pack.sbuff       = unit->sdata;
    pack.rbuff       = unit->rdata;
    pack.ssize       = &(unit->ssize);
    pack.rsize       = &(unit->rsize);
#ifdef ACTION_DEBUG
{
	int i;
    action_debug("(dispersion): addr=%x\n", pack.dev_addr);
    action_debug("send_buff:  len=%d ", *(pack.ssize));
    for(i=0;  i < *(pack.ssize); i++)
        printf(" %x ", pack.sbuff[i]);
    action_debug("\n");
}
#endif
    ret = sed_pack(&pack);
    if( ret <0 ) {
        action_debug("(dispersion): sed_pack err! %x\n", ret);
        return ret;
    }
    return ret;
}

/**
 * 对外的 rs232 命令
 *
 * @author chenxb (10/4/2010)
 *
 * @param unit
 *
 * @return int
 */

static int dispersion_select_amp(struct st_action_unit *unit )
{
    struct st_select_amp *amp;
    int ret = -1, count=ACTION_MAX_SEND;

   amp = search_select_amp((unit->dev_addr) - SELECT_AMP_BASE_ADDR);
   if(amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_CALLER ){
       bzero( select_amp_caller, 2*sizeof(WORD));
       /* 与上此AMP的原来状态 */
       /* fix 不用与上了 分区选择 现在只有一个音源输入 */
       #if 0
       if(unit->sdata[0] & 0x80){
        *( (WORD *)(unit->sdata) ) |= amp->select_state[SELECT_AMP_STATE_SOUND];
       }else{
        *( (WORD *)(unit->sdata) ) |= amp->select_state[SELECT_AMP_STATE_CALLER];
       }
       #endif
       while( ( (ret = fill_send_rec_pack(unit)) <0 )
                    && count--  ) ;
        if( ret < 0){
            action_debug("select_amp: err cont=%d\n", count);
            return ret;
        }
        if( (ret = select_amp_rec_op( unit) ) < 0){
           action_debug("select_amp_rec_op:  err ret=%d\n", ret);
           return ret;
        }
        return ret;
   }else if(amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_BUSY ){
       if( ( ret = select_amp_rec_op(unit) ) < 0){
           action_debug("select_amp_rec_op:  err ret=%d\n", ret);
           return ret;
        }
       //只有一个音源输入所以在BUSY 状态下只是保存不存在共用
       //fix 判断 是否在使用这个分区 如果没有使用那计划照做 如果在使用
       //把它改变进amp状态
       return SELECT_AMP_BUSY;
   }else if (amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_IDLE ){
       while( ( (ret = fill_send_rec_pack(unit)) <0 )  && count--  ) ;
        if( ret < 0 ){
            action_debug("select_amp: err cont=%d\n", count);
            return ret;
        }
       if( (ret = select_amp_rec_op( unit) ) < 0){
           action_debug("select_amp_rec_op:  err ret=%d\n", ret);
           return ret;
        }
       return ret;
   }
	return ret;
}

static int dispersion_matrix(struct st_action_unit *unit)
{
     struct st_matrix *matrix;
    int ret = -1, count=ACTION_MAX_SEND;

   matrix = seach_matrix(unit->dev_addr - MATRIX_BASE_ADDR);
   if( matrix->matrix_state[MATRIX_STATE] == MATRIX_CALLER){
   	   int i;
       bzero(matrix_caller, (MATRIX_LINE_MAX)*sizeof(WORD));
	   for(i=0; i < MATRIX_LINE_MAX; i++)
			matrix_caller[i] = matrix->matrix_state[i];
		while( ( (ret = fill_send_rec_pack(unit)) <0 )
                    && count--  ) ;

        if( ret < 0){
            action_debug("matrix: err cont=%d\n", count);
            return ret;
        }
        if( (ret = matrix_rec_op( unit) ) < 0){
           action_debug("matrix_rec_op:  err ret=%d\n", ret);
           return ret;
        }
        return ret;
        /* fix */
   }else if(matrix->matrix_state[MATRIX_STATE] == MATRIX_BUSY ){
       /* 即使 这个设备在BUSY 状态 CALLER 也有可能不会全部占掉所以的输出*/
       int line = unit->sdata[0];
       if( line > MATRIX_LINE_MAX-1){
           action_debug("dispersion_matrix: err! line > MATRIX_LINE_MAX\n");
           return -1;
       }
       if( !(matrix_caller[line] & unit->sdata[1]) ) {
           while( ( (ret = fill_send_rec_pack(unit)) <0 )
                        && count--  ) ;
            if( ret < 0){
                action_debug("matrix: err cont=%d\n", count);
                return ret;
            }
       }
       if( (ret = matrix_rec_op( unit) ) < 0){
           action_debug("matrix_rec_op:  err ret=%d\n", ret);
           return ret;
        }
       return MATRIX_BUSY;
   }else if (matrix->matrix_state[MATRIX_STATE] == MATRIX_IDLE ){
       while( ( (ret = fill_send_rec_pack(unit)) <0 )  && count--  ) ;

		if( ret < 0){
            action_debug("matrix: err cont=%d\n", count);
            return ret;
        }
       if( (ret = matrix_rec_op( unit) ) < 0){
           action_debug("matrix_rec_op:  err ret=%d\n", ret);
           return ret;
        }
       return ret;
   }

	return ret;
}
/**
 * 声卡设备分发
 *
 * @author chenxb (10/4/2010)
 *
 * @param unit
 *
 * @return int
 */
static int dispersion_sound(struct st_action_unit *unit)
{
	int ret = 0;
        /* 在这里  做本机操作*/
        int fn = unit->dev_addr;
        switch (fn) {
        case 0xd0:
            switch( unit->sdata[0]) {
            case 0:
            case 1:
                play_sound(unit);
                break;
            case 2:
                stop_sound();
                break;
            case 3:
                play_sound(unit);
                break;
            case 4:
            case 5:
                play_sound(unit);
                break;

            }
             /* 本机声卡*/
            break;
        case 0xd1:
            /* USB 声卡*/
            break;
        default:
            action_debug("(dispersion): err! no fn %d\n", fn);
            break;
        }
	return ret;
}


static int dispersion_caller( struct  st_action_unit *unit )
{
    int ret = -1;
/* 这里不做重复操作 重复操作在上层 一圈一座  不在这里单个操作*/
    ret = fill_send_rec_pack(unit) ;
    if( ret < 0){
        action_debug("select_amp: err ret=%d\n", ret);
        return ret;
    }

    if( (ret = caller_rec_op( unit) ) < 0){
       action_debug("select_amp_rec_op:  err ret=%d\n", ret);
       return ret;
    }
    return ret;
}

static int dispersion_peripheral( struct  st_action_unit *unit )
{
    int ret = -1;
/* 这里不做重复操作 重复操作在上层 一圈一座  不在这里单个操作*/
    ret = fill_send_rec_pack(unit) ;
    if( ret < 0){
        action_debug("select_amp: err cont=%d\n", ret);
        return ret;
    }

    if( (ret = peripheral_rec_op( unit) ) < 0){
       action_debug("select_amp_rec_op:  err ret=%d\n", ret);
       return ret;
    }
    return ret;
}

static int dispersion_lead_amp(struct st_action_unit *unit)
{
    int ret = -1, count =2;
    while( ( (ret = fill_send_rec_pack(unit)) <0 )  && count--  ) ;
        if( ret < 0 ){
            action_debug("lead_amp: err cont=%d\n", count);
            return ret;
        }
       if( (ret = lead_amp_rec_op( unit) ) < 0){
           action_debug("lead_amp_rec_op:  err ret=%d\n", ret);
           return ret;
        }
       return ret;
}
static int dispersion_external(struct st_action_unit *unit)
{
    int ret;
    /* !!! fix 在这里 解析 和分发 这个具体动作*/
    if( unit->dev_addr >= 0xb0 && unit->dev_addr <= 0xbf) {
        ret =  dispersion_select_amp(unit);
    }else if ( unit->dev_addr >= 0x90 && unit->dev_addr <= 0x9f) {
        ret = dispersion_caller( unit);
    }else if ( unit->dev_addr >= 0x60 && unit->dev_addr <= 0x6f) {
        ret = dispersion_matrix(unit);
    }else if (unit->dev_addr >= 0x70 && unit->dev_addr <= 0x7f) {
        ret = dispersion_lead_amp(unit);
    }else if (unit->dev_addr >= 0x80 && unit->dev_addr <= 0x8f) {
        ret = dispersion_peripheral(unit);
    }

    return ret;
}
static int dispersion_internal(struct st_action_unit *unit)
{
   return  dispersion_sound(unit);

}

int _dispersion( struct st_action_unit *unit)
{
    int ret;
    action_debug("dispersion lock\n");

    pthread_mutex_lock( &disp_mutex );
    if( unit->dev_addr  >= 0xd0 && unit->dev_addr <= 0xdf) {
        ret = dispersion_internal(unit);
    }else if ( (unit->dev_addr >= 0xb0 && unit->dev_addr <= 0xb9) ||
                  (unit->dev_addr >= 0x90 && unit->dev_addr <= 0x9f)  ||
                  (unit->dev_addr >= 0x60 && unit->dev_addr <= 0x6f) ||
                    (unit->dev_addr >= 0x70 && unit->dev_addr <= 0x7f) ||
                    (unit->dev_addr >= 0x80 && unit->dev_addr <= 0x8f)){
        ret = dispersion_external(unit);
    }else{
        ret = -1;
        action_debug("(dispersion): no addr to fn (unit->dev_addr=%x\n)", unit->dev_addr);
    }
    //disp_num--;
    pthread_mutex_unlock( &disp_mutex );
    action_debug("dispersion unlock\n");
    return ret;
}


int  dispersion(struct st_action_unit *unit)
{
    pthread_t thread;
    int *pret;

    pthread_create(&thread, NULL , (void *)_dispersion, (void *)unit);
    pthread_join( thread, (void **)&pret );

    //sem_post( &disp_sem);
    return (int)pret;
}


int  run_action_list( struct list_head *list)
{
    struct list_head *pos;
    struct st_action_unit *unit;
    int ret;
    pthread_mutex_lock( &plan_mutex );
    list_for_each_prev( pos, list){
        unit = list_entry( pos, struct st_action_unit, node);
        ret = dispersion(unit);
        if( ret < 0) {
            action_debug("(dispersion): return err\n");
        }
    }
    pthread_mutex_unlock( &plan_mutex);
    return ret;
}
/* 动作反操作解析 */
//////////////////////////////////////

struct st_deispersion_state{
	WORD matrix[MATRIX_LINE_MAX];
	WORD select[MATRIX_LINE_MAX][1];
};

struct st_destate_list{
	int id; /* action->id */
	struct st_deispersion_state  state;
	struct st_destate_list *next;
	struct st_destate_list *prev;
};

static struct st_destate_list *destate_list;

static void dedispersion_save_state(int id)
{
	struct st_destate_list *list;
	int ret;
	if( !destate_list) {
		destate_list = (struct st_destate_list *)malloc(sizeof *destate_list);
		list = destate_list;
		bzero(list, sizeof *list);
	}else{
		struct st_destate_list *new;
		list = destate_list;

		for(; list->next; list=list->next);
		new = (struct st_destate_list *)malloc(sizeof *destate_list);
		bzero(new, sizeof *list);
		new->next = NULL;
		new->prev = list;
		list->next = new;
		list = new;
	}
	list->id = id;
	ret = matrix_get_state(list->state.matrix, 6,MATRIX_IDLE);
	if (ret < 0 ) {
		action_debug("unlikely dedispersion_save_state\n");
		return;
	}
	ret = select_amp_get_sound_state_ex(NULL,NULL,0);

}
static void  dedispersion_load_state(int id, int flag)
{
	struct st_destate_list *list = destate_list;

	list = destate_list;
	for(; list; list = list->next){
		if( list->id == id){
			if (!list->prev && !list->next){
				destate_list = NULL;
			}else if ( !list->prev && list->next) {
				destate_list=list->next;
				destate_list->prev = NULL;
			}else if ( list->prev){
				list->prev->next = list->next;
			}else if (list->next){
				list->next->prev = list->prev;
			}
			break;
		}

	}
	if(!list) return;
	if(flag == 1)
		matrix_load_state_ex(gmatrix, list->state.matrix ,6);
	free(list);
}
///////////////////////////////////////

static int dedispersion_matrix(struct st_action_unit *unit)
{
	struct st_action_unit sunit;
    int ret = -1;
	bzero(&sunit, sizeof sunit);

	sunit.dev_addr =  unit->dev_addr;
	sunit.sdata[0] =  unit->sdata[0];

	sunit.sdata[1] =  0;
	sunit.ssize = 2;
	ret = dispersion(&sunit);
	if ( ret < 0){
		action_debug(" dedispersion_matrix sed cmd err\n");
		goto err;

	}
err:
	return ret;
}

static int dedispersion_sound(struct st_action_unit *unit)
{
	return stop_sound();

}

static int dedispersion_select_amp(struct st_action_unit *unit)
{


#if 0
	struct st_action_unit sunit;
	int ret= 0;

	if(!amp) return -1;
	if (amp->in == 0){
		bzero(&sunit, sizeof sunit);
		sunit.dev_addr = unit->dev_addr;
		sunit.sdata[0] = 0x80;
		sunit.sdata[1] = 0;
		sunit.ssize = 2;
		ret = dispersion(&sunit);
	}else ret = 0;

	return ret;
#endif
return 0;

}

static int dedispersion(struct st_action_unit *unit)
{
	int ret;
    if( unit->dev_addr  >= 0xd0 && unit->dev_addr <= 0xdf) {
        ret = dedispersion_sound(unit);
    }else if (unit->dev_addr >= 0xb0 && unit->dev_addr <= 0xb9){
		ret = dedispersion_select_amp(unit);
	}else if(unit->dev_addr >= 0x90 && unit->dev_addr <= 0x9f){
	//	ret = dedispersion_caller(unit);
	}else if(unit->dev_addr >= 0x60 && unit->dev_addr <= 0x6f){
		ret = dedispersion_matrix(unit);
	}else if(unit->dev_addr >= 0x70 && unit->dev_addr <= 0x7f){
	//	ret = dedispersion_lead_amp(unit);
	}else if(unit->dev_addr >= 0x80 && unit->dev_addr <= 0x8f){
	//	ret = dedispersion_peripheral(unit); //fix
	}
	return ret;
}
static int deaction_run(timer_id id, void *data, int len)
{
	struct list_head *list;
	struct list_head *pos;
	struct st_action_unit *unit;
	int  ret;
	struct st_action *action = (struct st_action *)(*(unsigned int *)data);

	//bzero( &action , sizeof action);
	//memcpy(&action, data, len);
	list = &action->aclist;
	printf("actiin->id = %d\n", action->id);
	pthread_mutex_lock( &plan_mutex);
	list_for_each_prev(pos, list){
		unit = list_entry(pos, struct st_action_unit, node);
		ret = dedispersion(unit);
		if (ret < 0){
			action_debug("dedispersion err\n");
		}
	}
	dedispersion_load_state(action->id, 1);
	ret = del_timer(id);
	pthread_mutex_unlock(&plan_mutex);

	return ret;
}
//////////////////////////////////////
int action_run( struct st_plan_action *now_action)
{
    struct st_action *action;
    int ret = -1;
    void *ptr;
    action = seach_action(now_action);
    if(action->id == -1) return ret;
    if(action != NULL) {
        action_debug("(action_run): des = %s\n", action->des);
		dedispersion_save_state(action->id);
        ret = run_action_list( &(action->aclist) );
		//if ( ret < 0) dedispersion_load_state( action->id, 0);

		/* 在这里要创建线程来执行一个此动作列表的反向列表  action->gap 来执行 */
		ptr = action;
		ret = (int)add_timer(action->gap * 60, deaction_run , &ptr, 4);

    }
    return ret;
}
