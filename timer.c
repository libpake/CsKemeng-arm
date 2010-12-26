#include "timer.h"

static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timer_list timer_list;

static void sig_func(int signo);

/**
 * Create a timer list.
 *
 * @param count    The maximum number of timer entries to be supported
 *			initially.
 *
 * @return         0 means ok, the other means fail.
 */
int init_timer(int count)
{
	int ret = 0;

	if(count <=0 || count > MAX_TIMER_NUM) {
		printf("the timer max number MUST less than %d.\n", MAX_TIMER_NUM);
		return -1;
	}

	memset(&timer_list, 0, sizeof(struct timer_list));
	LIST_INIT(&timer_list.header);
	timer_list.max_num = count;

	/* Register our internal signal handler and store old signal handler */
	if ((timer_list.old_sigfunc = signal(SIGALRM, sig_func)) == SIG_ERR) {
		return -1;
	}
	timer_list.new_sigfunc = sig_func;

	/* Setting our interval timer for driver our mutil-timer and store old timer value */
	timer_list.value.it_value.tv_sec = TIMER_START;
	timer_list.value.it_value.tv_usec = 0;
	timer_list.value.it_interval.tv_sec = TIMER_TICK;
	timer_list.value.it_interval.tv_usec = 0;
	ret = setitimer(ITIMER_REAL, &timer_list.value, &timer_list.ovalue);

	return ret;
}

/**
 * Destroy the timer list.
 *
 * @return          0 means ok, the other means fail.
 */
int destroy_timer(void)
{
	struct timer *node = NULL;

	if ((signal(SIGALRM, timer_list.old_sigfunc)) == SIG_ERR) {
		return -1;
	}

	if((setitimer(ITIMER_REAL, &timer_list.ovalue, &timer_list.value)) < 0) {
		return -1;
	}

	while (!LIST_EMPTY(&timer_list.header)) {/* Delete. */
		node = LIST_FIRST(&timer_list.header);
	LIST_REMOVE(node, entries);
		/* Free node */
		printf("Remove id %d\n", node->id);
		free(node->user_data);
		free(node);
	}

	memset(&timer_list, 0, sizeof(struct timer_list));

	return 0;
}

/**
 * Add a timer to timer list.
 *
 * @param interval  The timer interval(second).
 * @param cb  	    When cb!= NULL and timer expiry, call it.
 * @param user_data Callback's param.
 * @param len  	    The length of the user_data.
 *
 * @return          The timer ID, if == INVALID_TIMER_ID, add timer fail.
 */
timer_id
add_timer(int interval, timer_expiry *cb, void *user_data, int len)
{
	struct timer *node = NULL;

    pthread_mutex_lock(&timer_mutex);

	if (cb == NULL || interval <= 0) {
		return INVALID_TIMER_ID;
	}

	if(timer_list.num < timer_list.max_num) {
		timer_list.num++;
	} else {
		return INVALID_TIMER_ID;
	}
	pthread_mutex_lock(&malloc_flag_mutex);
	node = malloc(sizeof(struct timer));
	pthread_mutex_unlock(&malloc_flag_mutex);
	if(node == NULL) {
		return INVALID_TIMER_ID;
	}
	if(user_data != NULL || len != 0) {
		node->user_data = malloc(len);
		memcpy(node->user_data, user_data, len);
		node->len = len;
	}

	node->cb = cb;
	node->interval = interval;
	node->elapse = 0;
	node->id = timer_list.num;


	LIST_INSERT_HEAD(&timer_list.header, node, entries);
    pthread_mutex_unlock(&timer_mutex);
	return node->id;
}

/**
 * Delete a timer from timer list.
 *
 * @param id  	    The timer ID.
 *
 * @return          0 means ok, the other fail.
 */
int del_timer(timer_id id)
{

	if (id <0 || id > timer_list.max_num) {
		return -1;
	}
	struct timer *node = timer_list.header.lh_first;
	for ( ; node != NULL; node = node->entries.le_next) {
		if (id == node->id) {
			LIST_REMOVE(node, entries);
			timer_list.num--;
			free(node->user_data);
			free(node);
			return 0;
		}
	}
	/* Can't find the timer */
	return -1;
}

/* Tick Bookkeeping */
static void sig_func(int signo)
{
    pthread_mutex_lock(&timer_mutex);
	struct timer *node = timer_list.header.lh_first;
	age:
        node = timer_list.header.lh_first;
	for ( ; node != NULL; node = node->entries.le_next) {
		node->elapse++;
		if(node->elapse >= node->interval) {
			node->elapse = 0;
			node->cb(node->id, node->user_data, node->len);
			goto age;
		}
	}
		    pthread_mutex_unlock(&timer_mutex);
}

 char *fmt_time(char *tstr)
{
	time_t t;

	t = time(NULL);
	strcpy(tstr, ctime(&t));
	tstr[strlen(tstr)-1] = '\0';

	return tstr;
}

/* Unit Test */
timer_id id[2], call_cnt = 0;
int timer1_cb(timer_id id, void *arg, int len)
{
	char tstr[200];
	static int i, ret;

	/* XXX: Don't use standard IO in the signal handler context, I just use it demo the timer */
	printf("hello [%s]/id %d: timer1_cb is here.\n", fmt_time(tstr),id);
	if (i > 10) {
		ret = del_timer(id);
		printf("timer1_cb: %s del_timer/id %d::ret=%d\n", fmt_time(tstr),id,ret);
	}
	i++;
	call_cnt++;

	return 0;
}

#if 0
static void *
timer_main(void *sarg)
{
    int ret;
	char tstr[200];
	struct timespec time;
	time.tv_sec = 0,
	time.tv_nsec= 100000000;

	init_timer(MAX_TIMER_NUM);

	while (1) {
		nanosleep(&time, NULL);
	}

	ret = destroy_timer();
	printf("main: %s destroy_timer, ret=%d\n", fmt_time(tstr), ret);
	return NULL;
}
#endif
int timer_start()
{

	return	init_timer(MAX_TIMER_NUM);
}
