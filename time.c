#if 1
#include "time.h"
#include "debug.h"
#include "basetype.h"
#include "list.h"
#include "action.h"
#include "plan.h"

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include "caller.h"
int num_plan;
sem_t   time_run_sem;


/**
 * set system-time
 *
 * @author chenxb (11/2/2010)
 *
 * @param dt  " ( 2010-11-02  11:17:00)"
 *
 * @return int
 */
int set_system_time(char *dt)
{
struct tm tma;
char buff[128];
bzero( buff, 128);
    sscanf( dt, "%d-%d-%d %d:%d:%d",  &tma.tm_yday,
                &tma.tm_mon, &tma.tm_mday, &tma.tm_hour,
                &tma.tm_min, &tma.tm_sec );
    sprintf(buff, "date %02d%02d%02d%02d%02d.%02d", tma.tm_mon, tma.tm_mday, tma.tm_hour,
                                                         tma.tm_min, tma.tm_yday,tma.tm_sec);
	printf("date = %s\n",buff);
	system(buff);
    sprintf(buff, "hwclock -w");
    system(buff);
    return 0;
#if 0
    struct tm  tma;
    struct tm _tm;
    struct timeval tv;

    time_t timep;
    sscanf( dt, "%d-%d-%d %d:%d:%d",  &tma.tm_yday,
                &tma.tm_mon, &tma.tm_mday, &tma.tm_hour,
                &tma.tm_min, &tma.tm_sec );

    _tm.tm_sec  = tma.tm_sec;
    _tm.tm_min = tma.tm_min;
    _tm.tm_hour = tma.tm_hour;
    _tm.tm_mday = tma.tm_mday;
    _tm.tm_mon = tma.tm_mon -1;
    _tm.tm_year = tma.tm_year - 1900;

    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;

    if( settimeofday( &tv, (struct timezone *)0) < 0) {
        time_debug("Set system datatime error!\n");
        return -1;
    }
    #endif
    return 0;
}
void *action_run_thread( void *arg)
{
    int ret = -1;
    struct st_plan_action *plac = (struct st_plan_action *)arg;
    ret = action_run(plac);

    return (void *)ret;
}

int create_action_run(struct st_plan_action *action )
{
    int res;
    pthread_t  runid;
    int *pret;
    res = pthread_create(&runid, NULL, action_run_thread,  (void *)action);
    if( res != 0 ) {
        time_debug("time_run pthread is err\n");
        exit(res);
    }
     pthread_join( runid, (void **)&pret );

     return (int)pret;
}

/*   T I M E _ G E T _ N U M _ P L A N   */
/*-------------------------------------------------------------------------
    取得当前计划序号
    返回 0-6
-------------------------------------------------------------------------*/
int time_get_num_plan()
{
	struct tm *p;
	time_t res;
	res = time(NULL);
	p = localtime(&res);
	return (p->tm_wday);
}
/**
 * time_run  - TIME-RUN 用于比较当前计划的时间
 * 使用全局计划 plan->ac_list  和plan load 互斥
 *
 * @author chenxb (9/30/2010)
 */

void *time_run(void *arg)
{
    struct list_head *pos;
    struct list_head *list;
    struct st_plan_action *action;

    int hour=0, min=0, wday =0;
    time_debug("thread start\n");
    while(TRUE) {
            struct tm *p;
            time_t res;
            res  =time(NULL);
            p = localtime(&res);
            num_plan = wday = p->tm_wday;
			//time_debug("num_plan= %d\n", p->tm_wday);
            /* 循环查找 此plan的动作列表  fix */
/**
 *  ruguo  zai  shoudong bing xiugai jihua de shihou yao yong suoyi bixu
 */

            pthread_mutex_lock( &plan_flag_mutex);
            //printf(" time_run lock OK\n");
            if (!plan_flag){
                    pthread_mutex_unlock(&plan_flag_mutex);
                   // printf("plan_flag = 0, time_run unlock ok\n");
                    sleep(1);
                    continue;
            }else{
                    pthread_mutex_unlock( &plan_flag_mutex);
                   // printf(" plan_flag =1, time_run unlock ok\n");
            }
            sleep(1);
            list = &(plan[num_plan].ac_list);
            while(TRUE) {
                pthread_mutex_lock( &plan_flag_mutex);
                    //            printf(" lock1\n");
                if (!plan_flag) {
                    pthread_mutex_unlock(&plan_flag_mutex);
                  //  printf("unlock1\n");
                    break;
                }
                res  =time(NULL);
                p = localtime(&res);

                if ( wday != p->tm_wday) {
                    pthread_mutex_unlock(&plan_flag_mutex);
                  //  printf("unlock1\n");
                    break;
                }

                if ( hour == p->tm_hour && min == p->tm_min) {
                    pthread_mutex_unlock(&plan_flag_mutex);
                 //   printf("unlock1\n");
                    sleep(1);
                    continue;
                }

                //list_for_each_prev(pos,  list){
                list_for_each(pos,  list){
                    action = list_entry(pos, struct st_plan_action, node);
                    if( action->id == -1) continue;
                    /* start action */
                    if( action->start.hour == p->tm_hour &&
                        action->start.min == p->tm_min ){
                        if (create_action_run(action) < 0){
                                  time_debug("create_action_run err\n");
                         }
                    }
                }
                hour= p->tm_hour; min = p->tm_min;
                pthread_mutex_unlock(&plan_flag_mutex);
                sleep(1);
       }
    }
    time_debug("thread  exit\n");
    pthread_exit(NULL);
}

extern int plan_flag;
void *caller_timerun(void *agv)
{
     struct tm *p;
     time_t res;
     struct st_action_unit unit_hour, unit_min;
     bzero( &unit_hour, sizeof(unit_hour));
     bzero( &unit_min, sizeof(unit_min));


     while(1) {
         if ( plan_flag == 0){
             sleep(1);
             continue;
         }
         res  =time(NULL);
         p = localtime(&res);

         unit_hour.sdata[1] = p->tm_hour;
         unit_hour.sdata[0] = 0x4;
         unit_hour.ssize = 2;
         unit_min.sdata[0]= 0x5;
         unit_min.sdata[1]=p->tm_min;
         unit_min.ssize = 2;
         if( p->tm_sec == 30) {
             caller_sed_time(&unit_hour);
             bzero(&unit_hour, sizeof(unit_hour));

             caller_sed_time(&unit_min);
             bzero(&unit_min, sizeof(unit_min));
         }
         sleep(1);
       }
}

/**
 * create_time fist run
 *
 * @author chenxb (10/1/2010)
 *
 * @return int
 */
int time_start()
{
    int res = -1;
    pthread_t  runid,  caller_timeid;
   // res = sem_init(&time_run_sem, 0, 0);

    res = pthread_create(&runid, NULL, time_run, NULL);
    if( res != 0 ) {
        time_debug("time_run pthread is err\n");
        exit(res);
    }

    res = pthread_create(&caller_timeid, NULL, caller_timerun, NULL );
    if( res != 0) {
        time_debug(" caller_timerun pthread is err\n");
        exit( res);
    }
    return 0;
}

#endif
