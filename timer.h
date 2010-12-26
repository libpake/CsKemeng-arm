#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <pthread.h>


#include "basetype.h"

#define MAX_TIMER_NUM 		1000
#define TIMER_START 		1
#define TIMER_TICK			1
#define INVALID_TIMER_ID 	(-1)

typedef int timer_id;

/**
 * The type of callback function to be called by timer scheduler when a timer
 * has expired.
 *
 * @param id		The timer id.
 * @param user_data     The user data.
 * $param len		The length of user data.
 */
typedef int timer_expiry(timer_id id, void *user_data, int len);

/**
 * The type of the timer
 */
struct timer {
	LIST_ENTRY(timer) entries;	/**< list entry		*/

	timer_id id;			/**< timer id		*/

	int interval;			/**< timer interval(second)*/
	int elapse; 			/**< 0 -> interval 	*/

	timer_expiry *cb;		/**< call if expiry 	*/
	void *user_data;		/**< callback arg	*/
	int len;			/**< user_data length	*/
};

/**
 * The timer list
 */
struct timer_list {
	LIST_HEAD(listheader, timer) header;	/**< list header 	*/
	int num;				/**< timer entry number */
	int max_num;				/**< max entry number	*/

	void (*old_sigfunc)(int);		/**< save previous signal handler */
	void (*new_sigfunc)(int);		/**< our signal handler	*/

	struct itimerval ovalue;		/**< old timer value */
	struct itimerval value;			/**< our internal timer value */
};

extern int init_timer(int count);

extern int destroy_timer(void);

extern timer_id
add_timer(int interval, timer_expiry *cb, void *user_data, int len);

extern int del_timer(timer_id id);


extern int timer_start(void);













#endif
