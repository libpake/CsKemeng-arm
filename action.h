/**
 * action.h
 *
 */

 #ifndef _ACTION_H_
 #define _ACTION_H_

#include "dev.h"
#include "rs232.h"
#include "ini.h"
#include "basetype.h"
#include "list.h"
#include "pthread.h"
#include "plan.h"


#include <semaphore.h>
 #define ACTION_CONFIG_ACTION_MAX "ACTION_MAX"
 #define ACTION_CONFIG_ACTION   "ACTION"
 #define ACTION_CONFIG_DES  ".DES"
 #define ACTION_CONFIG_ID   ".ID"
 #define ACTION_CONFIG_AC_MAX   ".AC_MAX"
 #define ACTION_CONFIG_GAP ".AC_GAP"
 #define ACTION_CONFIG_AC   ".AC"

#define ACTION_MAX_SEND 2
#define MAX_ACTION 60
#define MAX_ACTION_SBUFF 2048
#define MAX_ACTION_RBUFF 32

/*  action  struct */
struct st_action{
    int id; /* only one */
    char des[32];
    struct list_head aclist;
    struct st_action *next;
	unsigned int gap;
};

struct st_action_unit{
    int dev_addr;
    int ssize, rsize;
    BYTE sdata[MAX_ACTION_SBUFF];
    BYTE rdata[MAX_ACTION_RBUFF];
	BYTE *mp3sdata;
    struct list_head node;
};
extern struct st_action *action_list;
extern sem_t action_run_sem;
extern int ac_load_cfg(INI *ini);
extern int ac_add_segment_cfg( INI *ini,  const char *des  );
extern int dispersion( struct st_action_unit *unit);

extern int add_ac(struct list_head *list,  BYTE *buff);
extern int ac_add_segment_byid( INI *ini, int id, const char *des, unsigned int gap);
extern int ac_max; /* max action_list*/
extern int get_ac_id(void);
extern int ac_add_key_cfg(INI *ini,  int id, struct st_action_unit *unit);
extern int ac_del_segment_id_cfg( INI *ini, int id);

extern int action_run( struct st_plan_action *now_action);


 #endif

