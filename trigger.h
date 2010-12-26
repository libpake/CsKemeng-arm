#ifndef __TRIGGER_H__
#define __TRIGGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

#include "debug.h"
#include "caller.h"

extern void trigger_start(void);
extern int fire_area;
#endif
