/**
 * debug.h
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

#if 0
#define CALLER_DEBUG
#define CONFIG_DEBUG
#define LEAD_AMP_DEBUG
#define MATRIX_DEBUG
#define PERIPHERAL_DEBUG
#define RS232_DEBUG
#define SELECT_DEBUG
#define POWER_DEBUG
#define PLAN_DEBUG
#define TIME_DEBUG

#define ACTION_DEBUG

#define SOUND_DEBUG
#define NET_DEBUG
#define TRIGGER_DEBUG

#endif
#if 1
#define RS232_DEBUG
#endif
#if 0
#define MATRIX_DEBUG
#define ACTION_DEBUG
#define TIME_DEBUG
#define PLAN_DEBUG
#define DEV_DEBUG
#endif
#ifdef TRIGGER_DEBUG
    #define TRIGGER  "TRIGGER: "
    #define trigger_debug(format, arg...) \
    printf("%s"format, NET, ##arg)
#else
    #define trigger_debug(format, arg...) 
#endif

#ifdef NET_DEBUG
    #define NET  "net: "
    #define net_debug(format, arg...) \
    printf("%s"format, NET, ##arg)
#else
    #define net_debug(format, arg...) 
#endif

#ifdef SOUND_DEBUG
    #define SOUND  "sound: "
    #define sound_debug(format, arg...) \
    printf("%s"format, SOUND, ##arg)
#else
    #define sound_debug(format, arg...) 
#endif


#ifdef ACTION_DEBUG
    #define ACTION  "action: "
    #define action_debug(format, arg...) \
    printf("%s"format, ACTION, ##arg)
#else
    #define action_debug(format, arg...) 
#endif

#ifdef TIME_DEBUG
    #define TIME "time: "
    #define time_debug(format, arg...) \
    printf("%s"format, TIME, ##arg)
#else
    #define time_debug(format, arg...) 
#endif


#ifdef PLAN_DEBUG
    #define PLAN "plan: "
    #define plan_debug( format, arg...) \
    printf("%s"format, PLAN,  ##arg)
#else
    #define plan_debug(format, arg...)
#endif

#ifdef POWER_DEBUG
    #define POWER "power: "
    #define power_debug( format, arg...) \
    printf("%s"format, POWER,  ##arg)
#else
    #define power_debug(format, arg...)
#endif

#ifdef CALLER_DEBUG
    #define CALLER "caller: "
    #define caller_debug( format, arg...) \
    printf("%s"format, CALLER,  ##arg)
#else
    #define caller_debug(format, arg...)
#endif


#ifdef CONFIG_DEBUG
    #define CONFIG "config: "
    #define config_debug( format, arg...) \
    printf("%s"format, CONFIG,  ##arg)
#else
    #define config_debug(format, arg...)
#endif


#ifdef LEAD_AMP_DEBUG
    #define LEAD_AMP "lead_amp: "
    #define lead_amp_debug( format, arg...) \
    printf("%s"format, LEAD_AMP,  ##arg)
#else
    #define lead_amp_debug(format, arg...)
#endif


#ifdef MATRIX_DEBUG
    #define MATRIX "matrix: "
    #define matrix_debug( format, arg...) \
    printf("%s"format, MATRIX,  ##arg)
#else
    #define matrix_debug(format, arg...)
#endif


#ifdef PERIPHERAL_DEBUG
    #define PERIPHERAL "peripheral: "
    #define peripheral_debug( format, arg...) \
    printf("%s"format, PERIPHERAL,  ##arg)
#else
    #define peripheral_debug(format, arg...)
#endif


#ifdef  RS232_DEBUG
    #define RS232 "rs232: "
    #define rs232_debug( format, arg...) \
    printf("%s"format, RS232,  ##arg)
#else
    #define rs232_debug(format, arg...)
#endif


#ifdef SELECT_DEBUG
    #define SELECT "select: "
    #define select_amp_debug( format, arg...) \
    printf("%s"format, SELECT,  ##arg)
#else
    #define select_amp_debug(format, arg...)
#endif

#endif
