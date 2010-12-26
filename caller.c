/*-------------------------------------------------------------------------

-------------------------------------------------------------------------*/
#include "caller.h"

#include "select_amp.h"
#include "matrix.h"
#include "peripheral.h"

struct st_caller *gcaller;
int     caller_max;

int     caller_cont;
int ARM_STATE =  CALLER_SEND_IDEL;

/*   N E W _ C A L L E R   */
/*-------------------------------------------------------------------------
    新建一个CALLER
    返回st_caller *指针
-------------------------------------------------------------------------*/
static struct st_caller*  new_caller()
{
    struct st_caller  *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p  = (struct st_caller *)malloc(sizeof(*p));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if(!p) return NULL;
    memset(p, 0, sizeof(*p));

    return p;
}

static int add_caller(int id, char *des, int power,int addr,  struct st_rs232 *prs232)
{
    struct st_caller caller;
    struct st_dev *dev = &(caller.dev);
    memset( &caller, 0,  sizeof(caller));
    caller.caller_state = CALLER_STATE_IDLE;
    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = prs232;
    dev->type   = CALLER_TYPE;
    dev->addr   =(WORD)(CALLER_BASE_ADDR + addr);
    dev->power = power;
    dev->dev_reset = NULL;
    if( !gcaller) {
        gcaller = new_caller();
        if(!gcaller)  {
            caller_debug(" err, can't malloc new caller\n");
            exit(-1);
        }
        memcpy(gcaller,  &caller, sizeof(caller));
        caller_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    caller.dev.id, caller.dev.des, caller.dev.rs232->fd);
         dev = &(gcaller->dev);
         add_dev(dev);
          return 0;
    }else{
        struct st_caller *new=NULL;
        new =  new_caller();
        if(!new) {
            caller_debug("err, can't malloc new peripheral\n");
            exit(-1);
        }
        memcpy(new, &caller, sizeof(caller));
        caller_debug("\n\tid=%d;\n\tdes=%s;\n\tuart's fd=%x;\n",
                                    caller.dev.id, caller.dev.des, caller.dev.rs232->fd);
        {
            struct st_caller *pos;
            for( pos = gcaller;  pos->next != NULL;  pos= pos->next) ;
            pos->next = new;
            dev = &( new->dev);
            add_dev(dev);
        }

        return 0;
    }
    return -1;
}

/**
 * caller_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int caller_load_cfg(INI *ini)
{
     char des[32];
    char buff[32];
    int  i;
    /**/

    caller_max =  ini_get_int(ini, CALLER_CONFIG_MAX);
    caller_debug("(load_cfg): max = %d\n", caller_max);

    for( i =0; i < caller_max ; i++) {
        struct st_rs232 *prs232;
        int uart, power, addr;
        /*  load lead_amp_des */
        set_string(buff, CALLER_CONFIG_DES, i);
        caller_debug("(load_cfg):  caller des = %s\n", buff);
        ini_get(ini, buff, des);
        /* load lead_amp uart*/
        set_string(buff, CALLER_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
            caller_debug("(load_cfg )  number %d ,  set uart err uart = %d\n", caller_max, uart);
            return -1;
        }
         /* load caller power*/
        set_string(buff, CALLER_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);

         /* load  caller addr*/
        set_string(buff, CALLER_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);
        add_caller( i, des, power, addr, prs232 );
    }
    return 0;
}



static struct st_caller * caller_seach( int addr)
{
    struct st_caller *pos;

    for( pos = gcaller; pos ; pos = pos->next) {
        if( pos->dev.addr == addr) {
            return pos;
        }
    }
    return NULL;
}

static struct st_action_unit request_unit;

int caller_op( int state )
{
    struct st_action_unit unit;
    struct  st_action_unit select_amp_unit;
    struct st_action_unit matrix_unit;
    struct st_caller *caller = NULL;
    int i;
    int cont= CALLER_CMD_MAX_COUNT;
    ARM_STATE = state;

    while(1) {
        int ret;
		caller_debug("caller ON \n");
        bzero( &unit, sizeof(unit));
        switch( ARM_STATE) {
        case  CALLER_SEND_LOOP:
            for( caller = gcaller;  caller; caller = caller->next ) {
                unit.sdata[0]=   CALLER_CMD_LOOP_DATA1;
                unit.sdata[1] =  CALLER_CMD_LOOP_DATA2;
                unit.ssize = 2;
                unit.dev_addr = caller->dev.addr;
                ARM_STATE = CALLER_SEND_LOOP;
                ret = dispersion(&unit);
                if( ret < 0) {
                    caller_debug("(CALLER_SEND_LOOP):  dispersion return %x\n",ret);
                    caller->caller_state = CALLER_STATE_ERR;
                    continue;
                }

                if( caller->caller_state == CALLER_STATE_REQUEST) {
                    ARM_STATE = CALLER_SEND_REQUEST;
                    caller_debug("(caller_op): caller->caller_state= %x\n", CALLER_STATE_REQUEST);
                    break;
                }
            }
            if( !cont--){
                ARM_STATE=CALLER_SEND_IDEL;
                return -1;
            }
            break;
        case CALLER_SEND_REQUEST:
            unit.sdata[0] = CALLER_CMD_REQUEST_DATA1;
            unit.sdata[1] = CALLER_CMD_REQUEST_DATA2;
            unit.ssize = 2;
            unit.dev_addr = caller->dev.addr;

            ret = dispersion(&unit);
            if( ret < 0 ) {
                caller_debug("(caller_op):  CALLER_SEND_RESQUST err!\n");
                ARM_STATE = CALLER_SEND_REQUEST;
                return ret;
            }

            bzero( &matrix_unit, sizeof(matrix_unit));
            matrix_unit.sdata[0] = CALLER_LINE_SET_ON;
            for( i = 0; i < 6; i++) {
                BYTE data1=0 , data2=0,tmp=0;
                BYTE data[2]={0, 0};
                bzero( &select_amp_unit, sizeof(select_amp_unit));
                data1 = request_unit.rdata[ i*2 + 1 ];
                data2 = request_unit.rdata[ i*2 + 2 ];
				tmp = data1;
				data[1] = tmp;
				tmp = 0;
				tmp = (data2) & 0x1;
				data[0] |= tmp;
				tmp = (data2) & 0x8;
				data[0] |= tmp << 1;

                if( !data[0] && !data[1])  continue;

                {
                    matrix_unit.sdata[1] |= (0x1 << i);
                }

                data[0] |= 0x80;
                memcpy(select_amp_unit.sdata, data, 2*sizeof(BYTE));
                select_amp_unit.ssize = 2;
                {
                    int tmp_ret;
                    struct st_select_amp *amp;
                    amp = search_out_select_amp(i+1);
                    //if(!amp)  continue; //fix 如果没有找到 这个分区矩阵同样要让呼叫站运行
					if (amp) //fix 为了配合
						tmp_ret = set_select_amp_ex(  amp,  &select_amp_unit );
                    //if( tmp_ret < 0)  continue; //fix 修改 如果分区矩阵返回失败 呼叫站同样运行
					if (amp){
						amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_CALLER;
                    	tmp_ret = dispersion(&select_amp_unit);
                    	if( tmp_ret < 0) {
                       	 	caller_debug("set_select_amp(%x): err\n", SELECT_AMP_BASE_ADDR +i);
                        	//continue; //fix 修改 如果分区矩阵 返回失败 呼叫站同样运行
                    	}

                    	amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_BUSY;
					}
                }
            }

            {
                struct st_matrix *matrix;
                int ret = -1;
                matrix = gmatrix;
                matrix_unit.ssize = 2;
                matrix_unit.dev_addr = matrix->dev.addr;
                matrix->matrix_state[MATRIX_STATE] = MATRIX_CALLER;
                ret = dispersion(&matrix_unit);
                if( ret < 0) {
                    caller_debug(" matrix(%x): err\n",MATRIX_BASE_ADDR);
                    matrix->matrix_state[MATRIX_STATE] = MATRIX_ERR;
                    ARM_STATE = CALLER_SEND_IDEL;
                    caller->caller_state = CALLER_STATE_IDLE;
                }
                matrix->matrix_state[MATRIX_STATE] = MATRIX_BUSY;
            }

            if( request_unit.rdata[0] == CALLER_CMD_REC_REQUEST_BACKSOUND ) {
                system("cat /start.wav > /dev/dsp1");
            //fix ... tian jia dao dongzuo tuilie  (ti shi yin)
            }
            ARM_STATE = CALLER_SEND_OPEN;
            caller->caller_state = CALLER_STATE_OPEN;
            caller_debug("(caller_op):  CALLER_SEND_RESQUST\n");
            break;
        case CALLER_SEND_OPEN:
            unit.sdata[0] = CALLER_CMD_OPEN_DATA1;
            unit.sdata[1] = CALLER_CMD_OPEN_DATA2;;
            unit.ssize = 2;
            unit.dev_addr = caller->dev.addr;
            ret = dispersion(&unit);
            if( ret < 0) {
                ARM_STATE = CALLER_SEND_IDEL;
                {
                    struct st_caller *caller;
                    caller = caller_seach(unit.dev_addr - CALLER_BASE_ADDR);
                    caller->caller_state = CALLER_STATE_ERR;
                }
                caller_debug("SEND_OPEN: err\n");
                return ret;
            }
            break;
        case CALLER_SEND_BUSY:
            {
                struct st_caller *caller;
                int ret;
                for( caller = gcaller; caller;  caller = caller->next) {
                    unit.sdata[0] = CALLER_CMD_BUSY_DATA1;
                    unit.sdata[1] = CALLER_CMD_BUSY_DATA2;
                    unit.ssize = 2;
                    unit.dev_addr = caller->dev.addr;
                    ret = dispersion(&unit);
                    if( ret < 0) {
                        caller_debug("(case CALLER_SEND_BUSY):  err! dispersion return %x\n", ret);
                        caller->caller_state = CALLER_STATE_ERR;
                        continue;
                    }
                }
                ARM_STATE = CALLER_SEND_BUSY;
                return 0;
            }
            break;
        case CALLER_SEND_IDEL:
            {
            int ret=0;
            struct st_caller *caller;

             {
                struct st_matrix *matrix;
                matrix = gmatrix;
                if( matrix->matrix_state[MATRIX_STATE] == MATRIX_BUSY) {
                    matrix->matrix_state[MATRIX_STATE] = MATRIX_CALLER;
                    ret = matrix_load_state(matrix);
                    if( ret < 0)  matrix->matrix_state[MATRIX_STATE] =  MATRIX_ERR;
                    else matrix->matrix_state[MATRIX_STATE] = MATRIX_IDLE;

                }
            }

            {
                struct st_action_unit new_unit;
                struct st_select_amp *amp;
                bzero(&new_unit, sizeof(new_unit));

                for( amp= gselect_amp; amp; amp = amp->next) {
                    if( amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_BUSY){
                        int ret;
                        amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_CALLER;
                        ret = select_amp_load_state(amp);
                        if( ret < 0){
                               amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_ERR;
                               continue;
                        }
                        amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_IDLE;
                    }
                }
            }

            for( caller = gcaller; caller;  caller = caller->next) {
                unit.sdata[0] = CALLER_CMD_IDLE_DATA1;
                unit.sdata[1] = CALLER_CMD_IDLE_DATA2;
                unit.ssize = 2;
                unit.dev_addr = caller->dev.addr;
                ret = dispersion(&unit);
                if( ret < 0) {
                    caller_debug("SEND_IDEL: err!  \n");
                    caller->caller_state = CALLER_STATE_ERR;
                    continue;
                }else{
                    caller->caller_state = CALLER_STATE_IDLE;
                }
            }



            ARM_STATE = CALLER_SEND_IDEL;
            return 0;
            }
            break;
        default:
               caller_debug("(caller_rec_op):  no fn\n");
               break;
        }
    }

    return 0;
}

int caller_sed_time(struct st_action_unit *unit)
{
    struct st_caller *caller;
                 for( caller = gcaller;  caller; caller = caller->next ) {
                     unit->dev_addr = caller->dev.addr;
                     dispersion(unit);
                 }

	return 0;
}

/*   C A L L E R _ R E C _ O P   */
/*-------------------------------------------------------------------------

-------------------------------------------------------------------------*/
int caller_rec_op(struct st_action_unit *unit)
{

    struct st_caller *caller;

    caller = caller_seach( unit->dev_addr);
    if( !caller) {
        caller_debug("(caller_rec_op) err: 1: impossible happy\n");
    }

    switch( ARM_STATE) {
     case CALLER_SEND_LOOP:
        if( unit->rdata[0] == CALLER_CMD_REC_CALLER_REQUEST){
            caller->caller_state = CALLER_STATE_REQUEST;
            caller_debug( "( caller_rec_op): caller_state= CALLER_STATE_REQUEST\n");
        }else{
            caller_debug( "( caller_rec_op): loop rec err\n");
            return -1;
        }
        break;
    case CALLER_SEND_REQUEST:
        memcpy(&request_unit, unit, sizeof(*unit));
        break;
    case CALLER_SEND_OPEN:
        if( unit->rdata[0] == CALLER_CMD_REC_TRUE) {;
            ARM_STATE=CALLER_SEND_BUSY;
        }else{
                ARM_STATE=CALLER_SEND_IDEL;
                caller->caller_state = CALLER_STATE_ERR;
                caller_debug("SEND_OPEN: err\n");
                return -1;
        }
        break;
     case CALLER_SEND_BUSY:
        if( unit->rdata[0] == 0xf9) {
            return 0;
        }else{

        }
        break;
    case CALLER_SEND_IDEL:
        if( unit->rdata[0] == 0xf9) {
  //          ARM_STATE  = CALLER_SEND_OK;
            return 0;
        }else  {
//                ARM_STATE=CALLER_SEND_EXIT;
//                caller->caller_state = CALLER_STATE_ERR;
                //caller_cont = 0;
                return -1;
        }

        break;
    default:
           caller_debug("(caller_rec_op):  no fn\n");
           break;
    }

    return 0;
}
