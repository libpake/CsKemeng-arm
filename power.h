
/** 
  * power.h 
 */
 #ifndef _POWER_H_

 #define _POWER_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"

#define POWER_BASE_ADDR 0xa2
#define POWER_TYPE    POWER_BASE_ADDR



/* config */
#define POWER_CONFIG_MAX "POWER.MAX"
#define POWER_CONFIG_DES "POWER.DES"
#define POWER_CONFIG_POWER "POWER.POWER"
#define POWER_CONFIG_UART "POWER.UART"
#define POWER_CONFIG_ADDR "POWER.ADDR"

struct st_power_config{
    int i;
};
struct st_power{
    struct st_dev   dev;
    void                *pri;
    struct st_power *next;
};
extern int power_load_cfg(INI *ini);
extern struct st_power *gpower;
 #endif
