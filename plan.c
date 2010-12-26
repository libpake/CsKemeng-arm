#include "time.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "debug.h"
#include "plan.h"
#include "ini.h"
#include "list.h"
#include "action.h"
#include "playsound.h"
#include "config.h"
#include "matrix.h"
/*  max */
int         plan_max;
struct st_plan plan[MAX_PLAN];
pthread_mutex_t plan_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plan_flag_mutex = PTHREAD_MUTEX_INITIALIZER;;
int plan_flag;
void start_plan()
{
	struct st_matrix *matrix;
	matrix = gmatrix;
	
    printf("start_plan and lock\n");
    pthread_mutex_lock( &plan_flag_mutex);
    plan_flag=1;
	dev_reset(0,&(matrix->dev));
	stop_sound();
    printf("start_plan and unlock \n");
    pthread_mutex_unlock( &plan_flag_mutex);
    printf("end \n");
}
void stop_plan()
{
	struct st_matrix *matrix = gmatrix;
        printf("stop_plan and lock\n");
    pthread_mutex_lock( &plan_flag_mutex);
        printf("stop_plan and lock OK \n");
    plan_flag =0;
	dev_reset(0,&(matrix->dev));
    stop_sound();
        printf("stop_plan and unlock \n");
    pthread_mutex_unlock( &plan_flag_mutex);
    printf(" stop end \n");
}

static void dec(char *a, char *b, char *c, char *buff)
{
    char *iter, *curr = buff;
    int i=0;

    for( iter=buff; *iter; iter++ , i++  ) {
        if( *iter == ';' ) {
            strncpy(a, curr, i);
            curr=iter+1;
            break;
        }
    }

    for(iter=curr, i=0;  *iter; iter++, i++) {
        if( *iter == ';') {
            strncpy(b, curr, i);
            curr = iter+1;
            break;
        }
    }

    if( *curr) {
        strcpy(c, curr);
      //  c = curr;
    }
}


/*   D E L _ A C T I O N _ I N I   */
/*-------------------------------------------------------------------------
    从文件删除标识这个 计划被删除
-------------------------------------------------------------------------*/
int del_action_ini(int num, char *buff)
{
	char seg[16], tmp[32];
	int max,i, ret=-1;
	INI *ini;
	bzero(seg, 16);
	bzero(tmp, 32);
	ini = ini_open(PLAN_INI);
	if (!ini)  return -1;
	set_string(seg, "MODE", num);

	strcpy(tmp, seg);
	strcat(tmp, PLAN_CONFIG_MAX_AC);
	max = ini_get_int(ini,tmp);

	for(i=0; i < max; i++){
		char ac[16];
		char acbuff[32];
		bzero(acbuff, 32);
		bzero(ac, 16);
		set_string(ac, PLAN_CONFIG_AC, i);
		strcpy(tmp, seg);
		strcat(tmp, ac);
		if (!strcmp(ini_get(ini, tmp, acbuff), buff)){
			ret = ini_set_int(ini, tmp, -1);
			if( !ret) return -1;
			ret = ini_save(ini);
			if (!ret) return -1;
			ini_close(ini);
			break;
		}

	}


	return 0;

}

/*   D E L _ A C T I O N   */
/*-------------------------------------------------------------------------
    从链表里删除 节点

    BUFF:
-------------------------------------------------------------------------*/
int del_action(struct list_head *list, char *buff)
{
	int start_hour, start_min, start_sec, end_hour, end_min, end_sec;
	char a[32], b[32], c[32];
	int id;
	char tmp[32];
	struct st_plan_action *action;
	struct list_head *pos;

	memset( tmp, 0, 32);
    dec(a, b, c, buff);

    tmp[0] = a[0];
    tmp[1] = a[1];
    start_hour  = atoi(tmp);
    tmp[0] = a[2];
    tmp[1] = a[3];
    start_min   = atoi(tmp);
    tmp[0] = a[4];
    tmp[1] = a[5];
    start_sec  = atoi(tmp);

    tmp[0] = b[0];
    tmp[1] = b[1];
    end_hour  = atoi(tmp);
    tmp[0] = b[2];
    tmp[1] = b[3];
    end_min   = atoi(tmp);
    tmp[0] = b[4];
    tmp[1] = b[5];
    end_sec  = atoi(tmp);

    id = atoi(c);

	list_for_each_prev(pos, list){
		action = list_entry(pos, struct st_plan_action, node);

		if(action->start.hour == start_hour && action->start.min == start_min &&
					action->start.sec  == start_sec  && action->id 	 == id){
			break;
		}
	}
	if( pos == list) return 0;
	list_del(&(action->node));
	free(action);
	action = NULL;
	return 0;
}

/**
 * add_action - add action to action-list
 *
 * @author chenxb (9/27/2010)
 *
 * @param list struct list_head  is
 *                  struct st_plan{
 *                  struct list_head list
 *                  }
 * @param buff
 *
 * @return int
 */
 int add_action(struct list_head *list, char *buff)
{
    struct st_plan_action *action;
    char a[32], b[32], c[32];
    char tmp[32];
    int start_hour, start_min, start_sec, end_hour, end_min, end_sec;
    int id;
	pthread_mutex_lock(&malloc_flag_mutex);
    action = (struct st_plan_action *)malloc(sizeof(struct st_plan_action));
	pthread_mutex_unlock(&malloc_flag_mutex);
    if( !action) {
        plan_debug("(add_action): err ! malloc err\n");
        return -1;
    }
    memset( tmp, 0, 32);
    dec(a, b, c, buff);

    tmp[0] = a[0];
    tmp[1] = a[1];
    start_hour  = atoi(tmp);
    tmp[0] = a[2];
    tmp[1] = a[3];
    start_min   = atoi(tmp);
    tmp[0] = a[4];
    tmp[1] = a[5];
    start_sec  = atoi(tmp);

    tmp[0] = b[0];
    tmp[1] = b[1];
    end_hour  = atoi(tmp);
    tmp[0] = b[2];
    tmp[1] = b[3];
    end_min   = atoi(tmp);
    tmp[0] = b[4];
    tmp[1] = b[5];
    end_sec  = atoi(tmp);

    id = atoi(c);

    action->id = id;
    action->start.hour = start_hour;
    action->start.min  = start_min;
    action->start.sec   = start_sec;

    action->end.hour = end_hour;
    action->end.min  = end_min;
    action->end.sec   = end_sec;

    plan_debug("id=%d\n\tstart: hour=%d, min=%d, sec=%d\n\tend:hour=%d, min=%d, sec=%d\n",
                             id, start_hour, start_min, start_sec, end_hour, end_min, end_sec);
    list_add(&(action->node), list);
    return 0;
}


/**
 *
 *
 * @author chenxb (9/27/2010)
 *
 * @param ini  struct *ini
 *
 * @return int  0 == true, -1 ==false
 */
int plan_load_cfg(INI *ini)
{
    char buff[32];
    int  i;

    plan_max = ini_get_int(ini, PLAN_CONFIG_MAX);
    if( plan_max > MAX_PLAN) {
        plan_debug("(plan_load_cfg): plan_max > %d\n", MAX_PLAN );
        return -1;
    }
    plan_debug("(plan_load_cfg): plan_max = %d\n", plan_max);
    for( i = 0; i < plan_max; i++) {
        char des[32];
        int ac_max=0, j;
        struct list_head *ac_list;
        ac_list = &(plan[i].ac_list);

        INIT_LIST_HEAD(ac_list);

        set_string( buff, PLAN_CONFIG_MODE, i);
        strcat(buff, PLAN_CONFIG_DES);
        ini_get(ini, buff, des);
        plan_debug("(plan_load_cfg): buff = %s, des = %s\n", buff, des);

        set_string( buff, PLAN_CONFIG_MODE, i);
        strcat(buff, PLAN_CONFIG_MAX_AC);
        ac_max =ini_get_int(ini, buff);
        plan_debug("(plan_load_cfg): buff = %s, ac_max= %d\n", buff, ac_max);

        for( j=0 ; j < ac_max; j++) {
            char tmpb[32], tmpa[32];

            set_string(buff, PLAN_CONFIG_MODE,i);
            set_string(tmpb, PLAN_CONFIG_AC, j);
            strcat(buff, tmpb);
			/*为了区分 这个 AC 值是否有效*/
            ini_get(ini, buff, tmpa);
            if(!strcmp(tmpa, "-1")) continue;
            plan_debug("(plan_load_cfg): buff = %s, value = %s\n", buff, tmpa);
            add_action(ac_list, tmpa);
        }
        plan_debug("(plan_load_cfg): %d\n", i);

        strcpy(plan[i].des, des);
        plan_debug("(plan_load_cfg): %s\n", plan[0].des);

    }
#if 0
    {
         struct st_plan_action *action;
         struct list_head node;
         struct list_head pos;
         list_for_each_prev(pos, &(plan[0].ac_list)){
                        action = list_entry(pos, struct st_plan_action, node);
                        printf("a");
                        printf("%x", action);

         }
    }
#endif
    return 0;
}



/*   P L A N _ D E L _ A C   */
/*-------------------------------------------------------------------------
    用于删除 计划中事件
-------------------------------------------------------------------------*/
int plan_del_ac(int mNum, char *buff)
{
	int ret = -1;
	struct list_head *list;
	list = &(plan[mNum].ac_list);
	/* 从列表中删除 */
	ret = del_action(list,buff);
	if(ret < 0) goto out;
	/* 从配置文件中删除*/
	ret = del_action_ini(mNum, buff);
out:

	return ret;
}
/*   P L A N _ S A V E _ A C   */
/*-------------------------------------------------------------------------
    plan_save
-------------------------------------------------------------------------*/
#include "config.h"
static int plan_ini_get_acid(INI *ini, int planpos, int max)
{
	char seg[32], ac[32];
	int i;
	bzero(seg, 32);
	bzero(ac, 32);


	set_string(seg, "MODE", planpos);
	for(i=0; i < max; i++){
		char tmp[32];
		char buff[32];
		bzero(tmp, 32);
		strcpy(tmp, seg);
		set_string(ac, PLAN_CONFIG_AC, i);
		strcat(tmp, ac);
		if (!strcmp(ini_get(ini, tmp, buff), "-1")) break;
	}
	return i;
}
int plan_ini_save_ac(int planpos, char *kbuff)
{
	INI *ini;
	char buff[32];
	char sbuff[32];
	int max, acid;
	ini = ini_open(PLAN_INI);
	if( !ini) return -1;
	bzero(buff, 32);

	set_string(buff, "MODE", planpos);
	strcat(buff, ".MAX_AC");
	max = ini_get_int(ini, buff);

	bzero(buff, 32);
	bzero(sbuff, 32);
	acid = plan_ini_get_acid(ini, planpos, max);
	set_string(buff, "MODE", planpos);
	set_string(sbuff,".AC", acid );
	strcat(buff, sbuff);
	if (!ini_set(ini,buff, kbuff)){
		ini_append(ini,buff,kbuff);
		set_string(buff, "MODE", planpos);
		strcat(buff, ".MAX_AC");
		ini_set_int(ini,buff,max+1);
	}
	ini_save(ini);
	ini_close(ini);

    return 0;
}

void  plan_start()
{
    start_plan();
    time_start();
}
