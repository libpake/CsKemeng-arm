#include "matrix.h"
#include "select_amp.h"
#include "config.h"
#include "basetype.h"
#include "caller.h"
#include "peripheral.h"
#include "sound.h"


/* moqian muren zhiyou yige 6x6 juzhen */
struct st_matrix *gmatrix;
int     matrix_max;


/* ???? ??CALLER ?matrix  ????*/
WORD matrix_caller[MATRIX_LINE_MAX];

int matrix_reset(struct st_dev *dev)
{
    struct st_matrix *matrix;
    struct st_action_unit unit;
    int ret, i;

for( i =1;  i < 7; i++){

        bzero(&unit, sizeof unit);
        unit.dev_addr = dev->addr;
        unit.sdata[0]  = i;
        unit.sdata[1]  = 0;
        unit.ssize = 2;
        if( (ret = dispersion(&unit)) < 0){
           matrix =  seach_matrix(dev->addr - MATRIX_BASE_ADDR);
           if( !matrix)  return -1;
           matrix->matrix_state[MATRIX_STATE] = MATRIX_ERR;
        }
}
    return ret;
}
static struct st_matrix*  new_matrix()
{
    struct st_matrix  *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_matrix *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

static int add_matrix(int id, char *des, int power, int addr, struct st_rs232 *prs232,
                                                                char  indes[MATRIX_LINE_MAX][MATRIX_STRING_MAXLEN])
{
    struct st_matrix matrix;
    struct st_dev *dev = &(matrix.dev);
    memset( &matrix, 0,  sizeof(matrix));

    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = MATRIX_TYPE;
    dev->addr   = (WORD)(MATRIX_BASE_ADDR + addr);
    dev->power = power;
    dev->dev_reset = matrix_reset;

    if( !gmatrix) {
        gmatrix = new_matrix();
        if(!gmatrix)  {
            matrix_debug(" err, can't malloc new matrix\n");
            exit(-1);
        }
        memcpy(gmatrix,  &matrix, sizeof(matrix));

        memcpy(gmatrix->indes[0],  indes[0],  sizeof(char)*(MATRIX_LINE_MAX * MATRIX_STRING_MAXLEN));

        matrix_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;"
                     "\n\tindes1=%s\n\tindes2 = %s\n\tindes3 = %s\n\tindes4=%s\n\tindes5=%s\n\tindes6=%s\n",
                                    matrix.dev.id, matrix.dev.des, matrix.dev.rs232->fd, gmatrix->indes[0], gmatrix->indes[1],
                                    gmatrix->indes[2], gmatrix->indes[3], gmatrix->indes[4], gmatrix->indes[5]);
         dev = &(gmatrix->dev);
         add_dev(dev);
         return 0;
    }else{
        struct st_matrix *new=NULL, *m = NULL;
        new =  new_matrix();
        if(!new) {
            matrix_debug("err, can't malloc new matrix\n");
            exit(-1);
        }
        memcpy(new, &matrix, sizeof(matrix));
        memcpy(new->indes[0],  indes[0],  sizeof(char)*(MATRIX_LINE_MAX * MATRIX_STRING_MAXLEN));
        matrix_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n\t indes1 = %s\n\tindes2 = %s\n",
                                    matrix.dev.id, matrix.dev.des, matrix.dev.rs232->fd, new->indes[0], new->indes[1]);
        for( m=gmatrix; m->next; m=m->next) ;
        m->next = new;
        dev  = &(new->dev);
        add_dev(dev);
        return 0;
    }
    return -1;
}

/**
 * matrix_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int matrix_load_cfg(INI *ini)
{
    char des[32];
    char buff[32];
    int  i;
    /**/

    matrix_max =  ini_get_int(ini, MATRIX_CONFIG_MAX);
    matrix_debug("(load_cfg): max = %d\n", matrix_max);

    for( i =0; i < matrix_max ; i++) {
        struct st_rs232 *prs232;
        int uart, power,addr;
        char indes[MATRIX_LINE_MAX][MATRIX_STRING_MAXLEN];

        /*  load matrix_des */
        set_string(buff, MATRIX_CONFIG_DES, i);
        matrix_debug("(load_cfg):  matrix des = %s\n", buff);
        ini_get(ini, buff, des);
        /* load matrix uart*/
        set_string(buff, MATRIX_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
            matrix_debug("(load_cfg )  number %d ,  set uart err uart = %d\n", matrix_max, uart);
            return -1;
        }
         /* load matrix power*/
        set_string(buff, MATRIX_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);


        /* load matrix addr */
        set_string(buff, MATRIX_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);

        /* load matrix in des*/
        set_string(buff, MATRIX_CONFIG_IN1_DES, i);
        ini_get(ini, buff, indes[0]);
        set_string(buff, MATRIX_CONFIG_IN2_DES, i);
        ini_get(ini, buff, indes[1]);
        set_string(buff, MATRIX_CONFIG_IN3_DES, i);
        ini_get(ini, buff, indes[2]);
        set_string(buff, MATRIX_CONFIG_IN4_DES, i);
        ini_get(ini, buff, indes[3]);
        set_string(buff, MATRIX_CONFIG_IN5_DES, i);
        ini_get(ini, buff, indes[4]);
        set_string(buff, MATRIX_CONFIG_IN6_DES, i);
        ini_get(ini, buff, indes[5]);
        add_matrix( i, des, power,addr,  prs232, indes );
    }
    return 0;
}

struct  st_matrix *seach_matrix( int addr)
{
    struct st_matrix *matrix;

    for( matrix = gmatrix; matrix;  matrix = matrix->next) {
        if( (matrix->dev.addr - MATRIX_BASE_ADDR) == addr) {
            return matrix;
        }
        continue;
    }

    matrix_debug("(seach_matrix): no dev!\n");
    return NULL;
}

int matrix_load_state(struct st_matrix *matrix)
{
    struct st_action_unit unit;
    int ret =-1 , i ;

    for( i=0; i < MATRIX_LINE_MAX; i++) {
        bzero(&unit, sizeof(unit));
        unit.sdata[0] = i+1;
        unit.sdata[1] = (BYTE)(matrix->matrix_state[i]);
        unit.dev_addr = matrix->dev.addr;
        unit.ssize = 2;
		ret = dispersion( &unit);
        if( ret < 0) {
            matrix_debug("load state: dispersion err\n");
            break;
        }
    }
    return ret;
}

/*   M A T R I X _ L O A D _ S T A T E _ E X   */
/*-------------------------------------------------------------------------
    设置 matrix pdate
-------------------------------------------------------------------------*/
int matrix_load_state_ex(struct st_matrix *matrix, WORD *pdate, int len)
{
	struct st_action_unit unit;
	int ret  = -1, i;
	if( !matrix || !pdate || len < 0 || len > MATRIX_LINE_MAX)
		return -1;
	for( i=0; i < len; i++){
		bzero(&unit, sizeof(unit));
        unit.sdata[0] = i+1;
        unit.sdata[1] = (BYTE)(pdate[i]);
        unit.dev_addr = matrix->dev.addr;
        unit.ssize = 2;
		ret = dispersion( &unit);
        if( ret < 0) {
            matrix_debug("load state: dispersion err\n");
            break;
        }
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////

/*   L O A D _ M A T R I X _ O U T _ S T A T E   */
/*-------------------------------------------------------------------------
    此函数用于获取 第 outNum 个 输出口 在被哪个输入口使用
    输入: utNum -> 0 - 6
   	输出:
   	返回: 输入口序列 -> 0 - 6  0 为 1路
-------------------------------------------------------------------------*/
int load_matrix_out_state( int outNum)
{
	int i;
	BYTE mask;
	struct st_matrix *matrix = gmatrix;/* 只存在一个， 以后 不会扩张*/

	if( outNum < 0 || outNum > 6 || !matrix){
		 matrix_debug("err!!\n");
	}

	mask = 0x1 << outNum;
	for(i = 0; i < MATRIX_LINE_MAX; i++){
		if( matrix->matrix_state[MATRIX_STATE] == MATRIX_BUSY &&
			(mask & matrix_caller[i])) break;
		else if( (matrix->matrix_state[i]) & mask )  break;
	}

	return ( i < MATRIX_LINE_MAX ? i: (-1));
}


///////////////////////////////////////////////////////////////////////////

/*   S A V E _ M A T R I X _ STATE   */
/*-------------------------------------------------------------------------
    此下三个函数用于保存matrix 状态
-------------------------------------------------------------------------*/
static void save_matrix_mask(WORD *pstate,  BYTE *sdata, BYTE mask, int line)
{
    int i;
    if( (*sdata) & mask){
        for( i=0; i < MATRIX_LINE_MAX; i++) {
           if (pstate[i] & mask){
               pstate[i] &= ~mask;
           }
        }
        pstate[line-1] |= mask;
    }else{
        pstate[line-1] &= ~mask;
    }
}
void save_matrix_ex( WORD *pdata, struct st_action_unit *unit)
{
	int line = unit->sdata[0];
	if( line > MATRIX_LINE_MAX +1 ) {
		matrix_debug("save_matrix: err line > %d\n", MATRIX_LINE_MAX);
		return;
	}
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x01,line );
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x02,line );
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x04,line );
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x08,line );
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x10,line );
	save_matrix_mask(pdata, &(unit->sdata[1]), 0x20,line );
}

void save_matrix(struct st_action_unit *unit)
{
    struct st_matrix *matrix;
    int line = unit->sdata[0];
    matrix = seach_matrix(unit->dev_addr- MATRIX_BASE_ADDR);
    if( line > MATRIX_LINE_MAX+1) {
        matrix_debug("save_matrix: err line > %d\n", MATRIX_LINE_MAX);
        return;
    }
    save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x01,line );
	save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x02,line );
	save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x04,line );
	save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x08,line );
	save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x10,line );
	save_matrix_mask(matrix->matrix_state, &(unit->sdata[1]), 0x20,line );

}
////////////////////////////////////////////////////


int matrix_op(int addr, int innum,  BYTE data, struct st_action_unit *unit)
{

    struct st_matrix *matrix;
    bzero( &unit, sizeof(unit));
    matrix = seach_matrix(addr);
    if( !matrix ) {
        matrix_debug("(matrix_op): no dev!\n");
        return -1;
    }

    unit->sdata[0] = innum;
    unit->sdata[1] = data;
    unit->ssize      = 2;
    unit->dev_addr = matrix->dev.addr;
    return 0;

}


/**
 * maxtrix_op_rec ->  rec op
 *
 * @author chenxb (10/13/2010)
 *
 * @param unit
 *
 * @return int
 */
int matrix_rec_op(struct st_action_unit *unit)
{

    struct st_matrix  *matrix;
	struct st_select_amp *amp;

    matrix = seach_matrix(unit->dev_addr - MATRIX_BASE_ADDR);

    if( unit->rsize > 0){
        if( unit->rdata[0] ==0xf9) {
            int i, in = unit->sdata[0];
            if(matrix->matrix_state[MATRIX_STATE] == MATRIX_CALLER ){
               save_matrix_ex(matrix_caller, unit);
            }else{
                save_matrix(unit);
            }
            for(i=0; i < 6; i++) {
				if (!unit->sdata[1]){
					for( amp = gselect_amp; amp; amp = amp->next ){
						if (amp->in == in) {
							amp->in = 0;
						}
					}
				}else if( unit->sdata[1] & ( 0x1 << i)) {
                    amp=  search_out_select_amp(i+1);
                    if (amp)  amp->in = in;
                }

            }
            return 0;
        }
    }else {
        /* fix */
        save_matrix(unit);
        return 0;
    }
    return -1;
}


/**
 * Get matrix des
 *
 * @author chenxb (10/30/2010)
 *
 * @param line
 *
 * @return char*
 */

char *matrix_get_des(int line)
{
    struct st_matrix *matrix = gmatrix;  /* muqian zhi moren yige */

    if( line > 6) {
        matrix_debug("LINE > 6\n");
        return NULL;
    }
    return  matrix->indes[line-1];
}
/////////////////////////////////////////////////////



/*   M A T R I X _ G E T _ S T A T E   */
/*-------------------------------------------------------------------------
     取得 现在matrix 状态信息
     flag = MATRIX_BUSY 表示读取呼叫站状态或者前置状态下的 matrix 信息
     flag = MATRIX_IDEL 表示读取在没有呼叫站或者前置状态下的matrix 信息
     pdate 非空    len pdata 大小 并且小于 matrix_line_max
-------------------------------------------------------------------------*/
int matrix_get_state(WORD *pdate,int len, int flag)
{
	int i;
	WORD *p;
	if( flag == MATRIX_BUSY)
		p = matrix_caller;
	else
		p = gmatrix->matrix_state;

	if (!p || !pdate || len > MATRIX_LINE_MAX )
		return -1;

	for(i=0; i < len; i++ )
		pdate[i] = p[i];
    return 0;
}


/*   M A T R I X _ S E T _ D E S   */
/*-------------------------------------------------------------------------
    设置 MATRIX 的描述
-------------------------------------------------------------------------*/
int matrix_set_des(int line, char *des)
{
    INI *ini;
    char buff[32];

    ini = ini_open(DEV_INI);
    if( !ini )  return -1;
    if ( line > 6 || line < 1)  return -1;
    switch(line) {
    case 1:
        set_string(buff, MATRIX_CONFIG_IN1_DES, 0);
        ini_set(ini, buff, des);
     break;
    case 2:
         set_string(buff, MATRIX_CONFIG_IN2_DES, 0);
         ini_set(ini, buff, des);
        break;
    case 3:
        set_string(buff, MATRIX_CONFIG_IN3_DES, 0);
        ini_set(ini, buff, des);
        break;
    case 4:
        set_string(buff, MATRIX_CONFIG_IN4_DES, 0);
        ini_set(ini, buff, des);
        break;
    case 5:
        set_string(buff, MATRIX_CONFIG_IN5_DES, 0);
        ini_set(ini, buff, des);
        break;
    case 6:
        set_string(buff, MATRIX_CONFIG_IN6_DES, 0);
        ini_set(ini, buff, des);
        break;
    default:
        printf("Err\n");
        return -1;
        break;
    }
    ini_save(ini);
    ini_close(ini);
    return 0;
}

/**
 * gengju duankou qude  lianjiezhege duankou de shebei dizhi
 *
 * @author pake (11/23/2010)
 *
 * @param num
 *
 * @return int
 */
int matrix_get_inaddr(int num)
{
    int ret = -1;
    switch(num) {
    case 1:
        /* qianzhi maikefeng */
        ret = 0;
        break;
    case 2:
        /* hujiaozhan */
        ret = CALLER_BASE_ADDR;
        break;
    case 3:
        /* arm */
        ret = SOUND_BASE_ADDR;
        break;
    case 4:
        /* cd */
        ret = PERIPHERAL_BASE_ADDR;
        break;
    case 5:
        ret = 0;
        break;
    case 6:
        ret = 0;
        break;
    default:
        ret = -1;
        break;
    }
    return ret;
}
