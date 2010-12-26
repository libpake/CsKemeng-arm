
/**
 * sound.h 
 */
 #ifndef _SOUND_H_

 #define _SOUND_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"
#define SOUND_BASE_ADDR 0xd0 /*  用于虚拟的 地址 对外以后不可用 */
#define SOUND_TYPE    SOUND_BASE_ADDR



/* config */
#define SOUND_CONFIG_MAX "SOUND.MAX"
#define SOUND_CONFIG_DES "SOUND.DES"
#define SOUND_CONFIG_POWER "SOUND.POWER"
#define SOUND_CONFIG_UART "SOUND.UART"
#define SOUND_CONFIG_ADDR "SOUND.ADDR"

struct st_sound_config{
    int i;
};
struct st_sound{
    struct st_dev   dev;
    void                *pri;
    struct st_sound *next;
};

extern int sound_load_cfg(INI *ini);
/* 声卡的 链表*/
extern struct st_sound *gsound;
 #endif
