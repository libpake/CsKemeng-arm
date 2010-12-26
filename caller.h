
/**
 * caller.h 
 */
 #ifndef _CALLER_H_

 #define _CALLER_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"
#include "basetype.h"
#include "action.h"

#define CALLER_BASE_ADDR 0x90
#define CALLER_TYPE    CALLER_BASE_ADDR



/* config */
#define CALLER_CONFIG_MAX "CALLER.MAX"
#define CALLER_CONFIG_DES "CALLER.DES"
#define CALLER_CONFIG_POWER "CALLER.POWER"
#define CALLER_CONFIG_UART "CALLER.UART"
#define CALLER_CONFIG_ADDR "CALLER.ADDR"

#define CALLER_CMD_MAX_COUNT   3
#define CALLER_CMD_WAIT_USLEEP 5000
 
#define CALLER_CMD_LOOP_DATA1  0x1
#define CALLER_CMD_LOOP_DATA2  0x0
#define CALLER_CMD_REQUEST_DATA1 0x3
#define CALLER_CMD_REQUEST_DATA2 0x0
#define CALLER_CMD_OPEN_DATA1 0x2
#define CALLER_CMD_OPEN_DATA2 0x0
#define CALLER_CMD_IDLE_DATA1   0x7
#define CALLER_CMD_IDLE_DATA2   0x0
#define CALLER_CMD_BUSY_DATA1 0x6
#define CALLER_CMD_BUSY_DATA2 0X0

#define CALLER_CMD_REC_CALLER_REQUEST 0xf1
#define CALLER_CMD_REC_TRUE 0xf9
#define CALLER_CMD_REC_ERR   0xf8
#define CALLER_CMD_REC_REQUEST_BACKSOUND 0x1


enum{
    CALLER_STATE_BUSY=1,
    CALLER_STATE_IDLE,

    CALLER_STATE_REQUEST, 
    CALLER_STATE_OPEN,
    CALLER_STATE_ERR
};

enum{
    CALLER_SEND_REQUEST=1,
    CALLER_SEND_LOOP,
    CALLER_SEND_OPEN,
    CALLER_SEND_BUSY,
    CALLER_SEND_CANCEL,
    CALLER_SEND_EXIT,
    CALLER_SEND_OK,
    CALLER_SEND_IDEL
};


struct st_caller{
    struct st_dev   dev;
    int caller_state;
    struct st_caller *next;
};

extern int caller_load_cfg(INI *ini);
extern struct st_caller *gcaller;
extern int caller_op( int state );
extern int caller_rec_op(struct st_action_unit *unit);
extern int ARM_STATE;
extern int caller_sed_time(struct st_action_unit *unit);

 #endif
