#include "playsound.h"
#include <signal.h>
#include <sys/types.h>
static  pthread_t  soundid;
int stopmp3;
#if 0
void *madplay_run1( void *arg)
{
   	char *buff = NULL;

    struct st_action_unit *unit = ( struct st_action_unit *)arg;

	pthread_mutex_lock( &malloc_flag_mutex);
	buff = malloc(strlen((char *)unit->mp3sdata) + 64);
	pthread_mutex_unlock( &malloc_flag_mutex);
	if (!buff) {
		printf("malloc madplay run err\n");
		return NULL;
		}

    switch(unit->sdata[0]) {
    case 0:
    case 1:
        sprintf( buff, "cd /music && /usr/bin/madplay %s", (unit->mp3sdata));
        if( unit->sdata[0] == 1)
              strcat(buff, " -r");
        printf("%s\n", buff);
        system(buff);
        soundid = 0;
        break;
    case 4:
        sprintf(buff, "cd /music && /usr/bin/madplay *.mp3");
        system(buff);
        break;
    case 5:
        sprintf(buff, "cd /music && /usr/bin/madplay *.mp3 -r");
        system(buff);
        break;
    }

	pthread_mutex_lock( &malloc_flag_mutex);
    free(buff);
	free(unit->mp3sdata);
	free(unit);
	pthread_mutex_unlock( &malloc_flag_mutex);
    return 0;
}
#endif
void *madplay_run( void *arg)
{
	char buff[1024], mp3name[512], *item, *start;
	int i = 0;
	struct st_action_unit *unit = (struct st_action_unit *)arg;
	printf("== madplay_run ==\n");
	printf("mp3 = %s\n", unit->mp3sdata);
age:
	start = (char *)unit->mp3sdata;

	while((*start) && !stopmp3){
	    bzero(mp3name, 512);
		for( item = start;  *item; item++) {
			if (*item == ' '){
				mp3name[i++] = 0;
				start = item + 1;
				break;
			}else
				mp3name[i++] = *item;
			}
		sprintf(buff, "cd /music && madplay %s", mp3name);
		printf("%s\n", buff);
		system(buff);
		i = 0;
		        if (! *item) break;
	}
	if ( unit->sdata[0] ==  1 && !stopmp3 ) goto age;
	printf("==madplay end==\n");
	pthread_mutex_lock(&malloc_flag_mutex);
	free(unit->mp3sdata);
	free(unit);
	pthread_mutex_unlock(&malloc_flag_mutex);
    return NULL;
}
int play_sound(struct st_action_unit *unit)
{
    int res = -1;
	struct st_action_unit *p;
	stopmp3 = 0;

#if 1
	pthread_mutex_lock( &malloc_flag_mutex);
	p = (struct st_action_unit *)malloc(sizeof *unit);
	pthread_mutex_unlock( &malloc_flag_mutex);
	if ( !p ) goto err;
	p->dev_addr = unit->dev_addr;
	p->sdata[0] = unit->sdata[0];
	
	pthread_mutex_lock( &malloc_flag_mutex);
	p->mp3sdata = malloc(strlen((char *)unit->mp3sdata)+1);
	pthread_mutex_unlock( &malloc_flag_mutex);
	if (!p->mp3sdata){

		pthread_mutex_lock( &malloc_flag_mutex);
		free(p);
		pthread_mutex_unlock( &malloc_flag_mutex);
		goto err;
	}
	bzero(p->mp3sdata, strlen((char *)unit->mp3sdata)+1);
	strcpy((char *)p->mp3sdata, (char *)unit->mp3sdata);
    res = pthread_create(&soundid, NULL, (void *)madplay_run, (void *)p);
err:
	printf("ret = %d\n\n\n", res);
    if( res < 0) {
        printf(" play_sound:err\n");
    }
#endif
    return res;
}

int  stop_sound()
{
	stopmp3 = 1;

    return (system("killall madplay"));

}

int contiue_sound()
{
    return pthread_kill(soundid, SIGCONT);
}

