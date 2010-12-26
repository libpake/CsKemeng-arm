/**
 * peripheral.h 
 */
 #ifndef _PERIPHERAL_H_

 #define _PERIPHERAL_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"
#define PERIPHERAL_BASE_ADDR 0x80
#define PER_TYPE    PERIPHERAL_BASE_ADDR



/* config */
#define PER_CONFIG_MAX "PERIPHERAL.MAX"
#define PER_CONFIG_DES "PERIPHERAL.DES"
#define PER_CONFIG_POWER "PERIPHERAL.POWER"
#define PER_CONFIG_UART "PERIPHERAL.UART"
#define PER_CONFIG_ADDR "PERIPHERAL.ADDR"



/**/
#define    PERIPHERAL_CD_MAX 16
enum{
    PERIPHERAL_SEND_ENABLE,
    PERIPHERAL_SEND_ONLINE,
    PERIPHERAL_SEND_SUM,
    PERIPHERAL_SEND_RESET,
    PERIPHERAL_SEND_ADD_LIST,
};


struct st_peripheral{
    BYTE    state;
    struct st_dev   dev;
    WORD peripheral_state[18];

    struct st_peripheral *next;
};

extern struct st_peripheral *gperipheral;
extern struct st_peripheral *search_peripheral(int addr);
extern int peripheral_rec_op(struct st_action_unit *unit);
extern int peripheral_load_cfg(INI *ini);

 #endif
