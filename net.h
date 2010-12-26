#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

/* */
#include "debug.h"
#include "basetype.h"
#include "select_amp.h"
#include "trigger.h"

#define NET_MAX_BUFF 10240
#define NET_PORT 4320
#define NET_TIMR_PORT 4321

#define MUSIC_DIR "/music"

extern void net_start(void);
#endif
