#include "basetype.h"

#include "select_amp.h"
#include "head_amp.h"
#include "config.h"
#include "dev.h"
#include "action.h"
#include "plan.h"

#include "net.h"
#include "matrix.h"
#include "rs232.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


pthread_mutex_t malloc_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
void load_default()
{

}
int main()
{
//	FILE *fp = fopen("/log","w+");
    init_dev();
//	fputs("==init_dev==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
    uart_start();
//	fputs("==uart_start==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
	timer_start();
//	fputs("==timer_start==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
    load_cfg();
//	fputs("==load_cfg==\n",fp);
//	fclose(fp);
//	sleep(1);
//dev_reset(1, NULL);
//	fp = fopen("/log", "w+");
	load_default();
//	fputs("==load_default==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
	plan_start();
//	fputs("==plan_start==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
    net_start();
//	fputs("==net_start==\n",fp);
//	fclose(fp);
//	sleep(1);

//	fp = fopen("/log", "w+");
	trigger_start();
//	fputs("==trigger_start==\n",fp);
//	fclose(fp);
//	sleep(1);

    while(1) {

       sleep(5);
    }

    close_uart();
    return 0;
}
