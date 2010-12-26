/**
 * plan.h
 */

 #ifndef _PLAN_H_
 #define _PLAN_H_
 #include <pthread.h>
#include "time.h"
#include "ini.h"
#define  PLAN_CONFIG_MAX "PLAN_MAX"
#define  PLAN_CONFIG_MODE "MODE"
#define  PLAN_CONFIG_DES ".DES"
#define  PLAN_CONFIG_MAX_AC ".MAX_AC"
#define  PLAN_CONFIG_AC ".AC"

#define MAX_PLAN 8 /* */
struct st_plan_action{
    int id;  /*only one */
    struct st_time start;
    struct st_time end;
    struct list_head node;
};

struct st_plan{
    char des[32];
    struct list_head ac_list;
};



extern  int         plan_max;
extern struct st_plan plan[MAX_PLAN];
extern pthread_mutex_t plan_mutex;
extern pthread_mutex_t plan_flag_mutex;
extern int plan_flag;

extern int plan_run_flag;
extern void start_plan(void);
extern void stop_plan(void);
extern int add_action(struct list_head *list, char *buff);
extern int plan_ini_save_ac(int planpos, char *buff);
extern int plan_load_cfg(INI *ini);
extern int plan_del_ac(int mNum, char *buff);
extern void  plan_start(void);

 #endif
