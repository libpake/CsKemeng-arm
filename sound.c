/* 声卡 做为虚拟外部设备同样需要加载 */
#include "sound.h"

struct st_sound *gsound;
int    sound_max;

/**
 * 创建一个新的声音设备
 *

 * @author chenxb (10/3/2010)
 *
 * @return struct st_sound*
 */
static struct st_sound*  new_sound()
{
    struct st_sound *p;
	pthread_mutex_lock(&malloc_flag_mutex);
    p = (struct st_sound *)malloc(sizeof( struct st_sound) );
	pthread_mutex_unlock(&malloc_flag_mutex);
    if( !p) {
        sound_debug("(new_sound): err!,  can't malloc\n");
        return NULL;
    }
    memset(p, 0, sizeof(*p));
    return p;
}

static int add_sound(int id, char *des, int power, int addr)
{
     struct st_sound sound;
    struct st_dev *dev = &(sound.dev);
    memset( &sound, 0,  sizeof(sound));

    dev->id = id;
    strncpy(dev->des, des, strlen(des));
    dev->rs232 = NULL;
    dev->type   = SOUND_TYPE;
    dev->addr   = (WORD)(SOUND_BASE_ADDR + addr );
    dev->power = power;

    if( !gsound) {
        gsound = new_sound();
        if(!gsound)  {
            sound_debug(" err, can't malloc new sound\n");
            exit(-1);
        }
        memcpy(gsound,  &sound, sizeof(sound));
        sound_debug("\n\tid=%d;\n\tdes=%s;\n",
                                    sound.dev.id, sound.dev.des);
         dev = &(gsound->dev);
         add_dev(dev);
          return 0;
    }else{
        struct st_sound *new=NULL;
        new =  new_sound();
        if(!new) {
            sound_debug("err, can't malloc new sound\n");
            exit(-1);
        }
        memcpy(new, &sound, sizeof(sound));
        gsound->next = new;
        dev  = &(new->dev);

        add_dev(dev);
        sound_debug("\n\tid=%d;\n\tdes=%s;\n",
                                    sound.dev.id, sound.dev.des);
        return 0;
    }
    return -1;
}

/**
 * sound_load_cfg
 *
 * @author chenxb (9/26/2010)
 *
 * @param ini  struct * ini;
 *
 * @return int  0 is true -1 is false
 */
int sound_load_cfg(INI *ini)
{
     char des[32];
    char buff[32];
    int  i;
    /**/
    sound_max =  ini_get_int(ini, SOUND_CONFIG_MAX);
    sound_debug("(load_cfg): max = %d\n", sound_max);

    for( i =0; i < sound_max ; i++) {
        int  power, addr;
        /*  load sound_des */
        set_string(buff, SOUND_CONFIG_DES, i);
        sound_debug("(load_cfg):  sound des = %s\n", buff);
        ini_get(ini, buff, des);
#if 0
        /* load sound uart*/
        set_string(buff, SOUND_CONFIG_UART, i);
        uart = ini_get_int(ini, buff);
        if( uart == 1) {
            prs232 = &uart1;
        }else if (uart == 2 ) {
            prs232 = &uart2;
        }else {
             peripheral_debug("(load_cfg )  number %d ,  set uart err\n", per_max);
            return -1;
        }
#endif
         /* load sound power*/
        set_string(buff, SOUND_CONFIG_POWER, i);
        power = ini_get_int(ini, buff);

          /* load sound addr */
        set_string(buff, SOUND_CONFIG_ADDR, i);
        addr = ini_get_int(ini, buff);
        add_sound( i, des, power, addr );
    }
    return 0;
}
