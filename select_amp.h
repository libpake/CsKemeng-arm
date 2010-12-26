/**
 * select_amp.h
 */
 #ifndef _SELECT_AMP_H_

 #define _SELECT_AMP_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"
#include "action.h"
#include "basetype.h"


#define SELECT_AMP_BASE_ADDR 0xb0
#define SELECT_AMP_TYPE    SELECT_AMP_BASE_ADDR

/* fn define */
#define SELECT_AMP_ON_SOUND     1
#define SELECT_AMP_OFF_SOUND    2
#define SELECT_AMP_ON_CALL      3
#define SELECT_AMP_OFF_CALL     4


/* config */
#define SELECT_CONFIG_MAX "SELECT_AMP.MAX"
#define SELECT_CONFIG_DES "SELECT_AMP.DES"
#define SELECT_CONFIG_POWER "SELECT_AMP.POWER"
#define SELECT_CONFIG_UART "SELECT_AMP.UART"
#define SELECT_CONFIG_ADDR "SELECT_AMP.ADDR"
#define SELECT_CONFIG_OUT "SELECT_AMP.OUT"

#define SELECT_AMP_STATE_SOUND 1
#define SELECT_AMP_STATE_CALLER 0
#define SELECT_AMP_STATE 2
enum{
    SELECT_AMP_IDLE,
    SELECT_AMP_CALLER,
    SELECT_AMP_BUSY,
    SELECT_AMP_ERR = -1
};
struct st_select_amp{
    struct st_dev   dev;
    struct st_select_amp *next;
    WORD  select_state[3];
    int  out; /* zhege zhi shi zai peizhi wenjian li guding de */
    int  in; /* matrix  jueding */
};
extern struct st_select_amp *search_out_select_amp(int outline);
extern int select_amp_load_cfg(INI *ini);
extern int set_select_amp_ex(  struct st_select_amp *amp,  struct st_action_unit *ac );
extern struct st_select_amp *gselect_amp;
extern int select_amp_load_state(struct st_select_amp *amp);
extern int select_amp_get_sound_state_ex(struct st_select_amp * amp, WORD *pdate, int len);

extern int select_amp_load_state_ex(struct st_select_amp *amp, WORD *pdate, int len);
extern struct st_select_amp *search_select_amp(int addr);
extern int save_select_amp_ex(WORD *pdata, int len, struct st_action_unit *unit);
extern int  select_amp_get_sound_state(struct st_select_amp *amp, int  control);
extern WORD select_amp_caller[2];
extern int select_amp_rec_op(struct st_action_unit *unit);

 #endif
