/**
 * config.h
 */

#ifndef _config_h_
#define _config_h_
#include "basetype.h"
#include "ini.h"
#include "debug.h"

/*ARM���ļ���ַ*/
#define DEV_INI  "/config/dev.ini"
#define PLAN_INI  "/config/plan.ini"
#define AC_INI  "/config/action.ini"

/*PC���ļ���ַ*/
//#define DEV_INI  "/home/pake/config/dev.ini"
//#define PLAN_INI  "/home/pake/config/plan.ini"
//#define AC_INI  "/home/pake/config/action.ini"



extern int load_cfg(void);

#endif

