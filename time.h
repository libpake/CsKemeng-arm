#ifndef _APP_TIME_H_
#define _APP_TIME_H_
#include "basetype.h"
#include "list.h"
#include <semaphore.h>
#include <time.h>
//#include <rtc.h>
//extern sem_t action_sem;

struct st_time{
    int sec;
    int min;
    int hour;
    struct list_head node;
};

extern int num_plan;
extern sem_t time_run_sem;

extern int start_flags;
extern int set_system_time(char *dt);
extern int time_get_num_plan(void);
extern int time_start(void);



#endif
