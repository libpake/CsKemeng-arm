#include "config.h"
#include "head_amp.h"
#include "caller.h"
#include "select_amp.h"
#include "matrix.h"
#include "power.h"
#include "plan.h"
#include "dev.h"
#include "action.h"
#include "sound.h"
#include "peripheral.h"

static int dev_cfg( INI *dev)
{
    int ret = 0;
   config_debug("(dev_cfg): start\n");
   ret = lead_amp_load_cfg(dev);
   if( ret < 0) {
       config_debug( "(head_amp_load_cfg) : return err! dev = %x\n", (unsigned int)dev);
    }

   ret = peripheral_load_cfg(dev);
   if( ret < 0) {
       config_debug("(peripheral_load_cfg):  return err! ret=%d, dev=%x\n",ret,  (unsigned int)dev);
   }

   ret = caller_load_cfg(dev);
   if( ret < 0) {
       config_debug("(caller_load_cfg):  return err! ret=%d, dev=%x\n",ret,  (unsigned int)dev);
   }

   ret = select_amp_load_cfg(dev);
   if( ret < 0) {
       config_debug("(select_amp_load_cfg):  return err! ret=%d, dev=%x\n",ret,  (unsigned int)dev);
   }
   ret = matrix_load_cfg(dev);
   if(ret <0){
       config_debug("(matrix_load_cfg): return err! ret = %d, dev= %x\n", ret, ( unsigned int )dev);
   }
   ret = power_load_cfg(dev);
   if(ret <0){
       power_debug("(power_load_cfg): return err! ret = %d, dev= %x\n", ret, ( unsigned int )dev);
   }
   ret = sound_load_cfg(dev);
   if(ret < 0) {
       config_debug("(sound_load_cfg): err!,  ret = %d, dev = %x\n", ret, (unsigned int) dev);
   }
   return ret;
}

static int plan_cfg(INI *plan)
{

    return plan_load_cfg(plan);
}

static int ac_cfg(INI *ac)
{
    return ac_load_cfg(ac);
}

/**
 * 
 * 
 * @author chenxb (10/2/2010)
 * 
 * @return int 
 */
int load_cfg()
{
    INI  *inidev, *iniplan, *iniaction;
    inidev = ini_open(DEV_INI);
    if( !inidev)  {
        config_debug("(load_cfg) err: open %s\n", DEV_INI);
        return -1;
    }
    dev_cfg(inidev);
    ini_close(inidev);

    iniplan = ini_open(PLAN_INI);
    if( !iniplan) {
        config_debug("(load_cfg): err, open %s\n", PLAN_INI);
        return -1;
    }
    plan_cfg(iniplan);
	ini_close(iniplan);
	
    iniaction = ini_open(AC_INI);
    if(!iniaction) {
        config_debug("(load_cfg): err, open %s\n", AC_INI);
        return -1;
    }
    ac_cfg(iniaction);
	ini_close(iniaction);
	
    printf("load_cfg ok !\n");
    return 0;

}

int ini_save_action(INI *ini, struct st_action_unit *ac)
{
    INI *pac = ini;
    if( !pac) {
        pac = ini_open(AC_INI);
        if(!pac)  return -1;
    }
    
return 0;
}
