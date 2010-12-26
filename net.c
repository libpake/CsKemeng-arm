#include "net.h"
#include "caller.h"
#include "matrix.h"
#include "head_amp.h"
#include "sound.h"
#include "time.h"
#include "plan.h"
#include "list.h"
#include "action.h"
#include "ini.h"
#include "config.h"
#include "list.h"
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <asm/ioctls.h>

#include <dirent.h>



static int sock_opt =1;
static int update;

/*   A P P _ U P   */
/*-------------------------------------------------------------------------
    更新APP 到 主机
-------------------------------------------------------------------------*/
static char appname[128];
static int  appfilefd;
static int  appupdate;
static int net_app_up(int connfd, char *buff)
{
		int j=1, ret = -1;
		BYTE bpack_num[4];
		BYTE sed_buff[2];
		char tmp[32];
		/* exit update*/
		if ( buff[1] == 0xf) {
			appupdate = 0;
			if( appfilefd != 0){
				char tmpcmd[32];
				bzero(tmpcmd, 32);
			//	sprintf(tmpcmd, "mv /app/%stmp /app/app", appname);
			//	system(tmpcmd);
				close(appfilefd);

			}
			goto ok;
		}
		for( j=0; j < 4; j++) {
			bpack_num[j] =buff[1+j];
		}
		j = 1;
		bzero(tmp, 32);
		if( bpack_num[0] == 'S'  && appupdate == 0) {
			bzero(appname,128);
			strcpy(appname, &buff[5]);
			system("rm /app/app");
			sprintf(tmp, "/app/%s", &buff[5]);
			printf("file:	 %s\n", tmp);
			appfilefd = open(tmp, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR );
			if( appfilefd <= 0) {
				appupdate=0;
				goto err;
			}else{
				system("chmod +x /app/app");
				appupdate=1;
				goto ok;
			}
		}
		if( appupdate ) {
			if( bpack_num[0] != 0x45) {
				int  ret;
				ret = write(appfilefd, &buff[5], 4092);
				if( ret != 4092 ) {
					printf("\n\n\n\n\nererer\n");
					goto err;
				}
				goto ok;
			}else{
				int  ret;
				int size = bpack_num[1]*255 + bpack_num[2] ;
				ret = write(appfilefd, &buff[5], size);
				close(appfilefd);
				appfilefd = 0;
				appupdate = 0;
				goto ok;
			}
		}



		err:
				appupdate=0;
				sed_buff[1] = 0xf;
				printf("err:  %x\n", sed_buff[1]);
				j++;
		ok:
				sed_buff[0] = 0xff;
				printf("ok: %x\n", sed_buff[0]);
				usleep(100);

				ret = write(connfd,  sed_buff, j);

				if( ret != j) {
					printf("net_upmp3_info, err sed ok\n");
					ret = -1;
				}
				{
              #ifdef NET_DEBUG
						printf("net : %x, %x\n", ret, j);
						for( j=0 ; j < ret; j++) {
							printf(" %x", sed_buff[j]);
						}
						printf("\n");
               #endif
				}
		if( ret < j)  {
			net_debug(" sed mesg err(ret = %d)\n", ret);
		}
	return ret;


}


/*   N E T _ A C T I O N I N I _ U P   */
/*-------------------------------------------------------------------------
    更新 动作配置文件
-------------------------------------------------------------------------*/
	static char actionname[128];
	static int	actionfilefd;
	static int	actionupdate;

static int net_actionini_up(int connfd, char *buff)
{
		int j=1, ret = -1;
		BYTE bpack_num[4];
		BYTE sed_buff[2];
		char tmp[32];
		/* exit update*/
		if ( buff[1] == 0xf) {
			actionupdate = 0;
			if( actionfilefd != 0){
				char tmpcmd[32];
				bzero(tmpcmd, 32);
			//	sprintf(tmpcmd, "mv /app/%stmp /app/app", appname);
			//	system(tmpcmd);
				close(actionfilefd);

			}
			goto ok;
		}
		for( j=0; j < 4; j++) {
			bpack_num[j] =buff[1+j];
		}
		j = 1;
		bzero(tmp, 32);
		if( bpack_num[0] == 'S'  && actionupdate == 0) {
			bzero(actionname,128);
			strcpy(actionname, &buff[5]);
			system("rm /config/action.ini");
			sprintf(tmp, "/config/%s", &buff[5]);
			printf("file:	 %s\n", tmp);
			actionfilefd = open(tmp, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR );
			if( actionfilefd <= 0) {
				actionupdate=0;
				goto err;
			}else{
				//system("chmod +x /app/app");
				actionupdate=1;
				goto ok;
			}
		}
		if( actionupdate ) {
			if( bpack_num[0] != 0x45) {
				int  ret;
				ret = write(actionfilefd, &buff[5], 4092);
				if( ret != 4092 ) {
					printf("\n\n\n\n\nererer\n");
					goto err;
				}
				goto ok;
			}else{
				int  ret;
				int size = bpack_num[1]*255 + bpack_num[2] ;
				ret = write(actionfilefd, &buff[5], size);
				close(actionfilefd);
				actionfilefd = 0;
				actionupdate = 0;
				goto ok;
			}
		}



		err:
				actionupdate=0;
				sed_buff[1] = 0xf;
				j++;
		ok:
				sed_buff[0] = 0xfc;
				usleep(100);
				ret = write(connfd,  sed_buff, j);
				if( ret != j) {
					printf("net_upmp3_info, err sed ok\n");
					ret = -1;
				}
				{
              #ifdef NET_DEBUG
						printf("net : %x, %x\n", ret, j);
						for( j=0 ; j < ret; j++) {
							printf(" %x", sed_buff[j]);
						}
						printf("\n");
               #endif
				}
		if( ret < j)  {
			net_debug(" sed mesg err(ret = %d)\n", ret);
		}
	return ret;
}

/*   N E T _ P L A N I N I _ U P   */
/*-------------------------------------------------------------------------
    更新 计划配置文件
-------------------------------------------------------------------------*/
	static char planname[128];
	static int	planfilefd;
	static int	planupdate;

static int net_planini_up(int connfd, char *buff)
{

		int j=1, ret = -1;
		BYTE bpack_num[4];
		BYTE sed_buff[2];
		char tmp[32];
		/* exit update*/
		if ( buff[1] == 0xf) {
			planupdate = 0;
			if( planfilefd != 0){
				char tmpcmd[32];
				bzero(tmpcmd, 32);
			//	sprintf(tmpcmd, "mv /app/%stmp /app/app", appname);
			//	system(tmpcmd);
				close(planfilefd);

			}
			goto ok;
		}
		for( j=0; j < 4; j++) {
			bpack_num[j] =buff[1+j];
		}
		j = 1;
		bzero(tmp, 32);
		if( bpack_num[0] == 'S'  && planupdate == 0) {
			bzero(planname,128);
			strcpy(planname, &buff[5]);
			system("rm /config/plan.ini");
			sprintf(tmp, "/config/%s", &buff[5]);
			printf("file:	 %s\n", tmp);
			planfilefd = open(tmp, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR );
			if( planfilefd <= 0) {
				planupdate=0;
				goto err;
			}else{
				//system("chmod +x /app/app");
				planupdate=1;
				goto ok;
			}
		}
		if( planupdate ) {
			if( bpack_num[0] != 0x45) {
				int  ret;
				ret = write(planfilefd, &buff[5], 4092);
				if( ret != 4092 ) {
					printf("\n\n\n\n\nererer\n");
					goto err;
				}
				goto ok;
			}else{
				int  ret;
				int size = bpack_num[1]*255 + bpack_num[2] ;
				ret = write(planfilefd, &buff[5], size);
				close(planfilefd);
				planfilefd = 0;
				planupdate = 0;
				goto ok;
			}
		}



		err:
				planupdate=0;
				sed_buff[1] = 0xf;
				j++;
		ok:
				sed_buff[0] = 0xfe;
				usleep(100);
				ret = write(connfd,  sed_buff, j);
				if( ret != j) {
					printf("net_upmp3_info, err sed ok\n");
					ret = -1;
				}
				{
              #ifdef NET_DEBUG
						printf("net : %x, %x\n", ret, j);
						for( j=0 ; j < ret; j++) {
							printf(" %x", sed_buff[j]);
						}
						printf("\n");
               #endif
				}
		if( ret < j)  {
			net_debug(" sed mesg err(ret = %d)\n", ret);
		}
	return ret;
}

/*   N E T _ D E V I N I _ U P   */
/*-------------------------------------------------------------------------
    更新 设备配文件
-------------------------------------------------------------------------*/
	static char devname[128];
	static int	devfilefd;
	static int	devupdate;

static int net_devini_up(int connfd, char *buff)
{
	int j=1, ret = -1;
	BYTE bpack_num[4];
	BYTE sed_buff[2];
	char tmp[32];
	/* exit update*/
	if ( buff[1] == 0xf) {
		devupdate = 0;
		if( devfilefd != 0){
			char tmpcmd[32];
			bzero(tmpcmd, 32);
		//	sprintf(tmpcmd, "mv /app/%stmp /app/app", appname);
		//	system(tmpcmd);
			close(devfilefd);
				}
	goto ok;
	}
	for( j=0; j < 4; j++) {
		bpack_num[j] =buff[1+j];
	}
	j = 1;
	bzero(tmp, 32);
	if( bpack_num[0] == 'S'  && devupdate == 0) {
		bzero(devname,128);
		strcpy(devname, &buff[5]);
		system("rm /config/dev.ini");
		sprintf(tmp, "/config/%s", &buff[5]);
		printf("file:	 %s\n", tmp);
		devfilefd = open(tmp, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR );
		if( devfilefd <= 0) {
			devupdate=0;
			printf("open devfile err: %s\n", tmp);
			goto err;
		}else{
			//system("chmod +x /app/app");
			devupdate=1;
			goto ok;
		}
	}
	if( devupdate ) {
		if( bpack_num[0] != 0x45) {
			int  ret;
			ret = write(devfilefd, &buff[5], 4092);
			if( ret != 4092 ) {
				printf("\n\n\n\n\nererer\n");
				goto err;
			}
			goto ok;
		}else{
			int  ret;
			int size = bpack_num[1]*255 + bpack_num[2] ;
			ret = write(devfilefd, &buff[5], size);
			close(devfilefd);
			devfilefd = 0;
			devupdate = 0;
			goto ok;
		}
	}


	err:
			devupdate=0;
			sed_buff[1] = 0xf;
			j++;
	ok:
			sed_buff[0] = 0xfd;
			usleep(100);
			ret = write(connfd,  sed_buff, j);
			if( ret != j) {
				printf("net_upmp3_info, err sed ok\n");
				ret = -1;
			}
			{
            #ifdef NET_DEBUG
					printf("net : %x, %x\n", ret, j);
					for( j=0 ; j < ret; j++) {
						printf(" %x", sed_buff[j]);
					}
					printf("\n");
             #endif
			}
	if( ret < j)  {
		net_debug(" sed mesg err(ret = %d)\n", ret);
	}
	return ret;

}

/*   N E T _ S E L E C T _ D I S P E R S I O N   */
/*-------------------------------------------------------------------------
    分区矩阵信息
-------------------------------------------------------------------------*/
#if 0
static int net_select_dispersion(char* buff)
{
    int ret = 0;
#if 0
    struct st_action_unit ac;

    switch( buff[1]) {
    case SELECT_AMP_ON_SOUND:
      //  ac.sdata[0]=
        set_select_amp_ex(buff[0]-0xb0, &ac);
        set_select_amp(SELECT_AMP_ON_SOUND, buff[0]-0xb0, buff[2], &ac );
        dispersion(&ac);
        break;
    case SELECT_AMP_OFF_SOUND:
        set_select_amp( SELECT_AMP_OFF_SOUND, buff[0]-0xb0, buff[2], &ac);
        dispersion(&ac);
        break;
    default:
        net_debug("(net_select_dispersion): no fn \n");
        break;
    }
#endif
    return ret;
}
#endif

/*   N E T _ M A T R I X _ I N F O   */
/*-------------------------------------------------------------------------
    6x6 矩阵信息  0x11
-------------------------------------------------------------------------*/
static int net_matrix_info(int connfd, char *recbuff)
{
    int  ret = -1, cont=0, i;
    BYTE sedbuff[1024];

    bzero(sedbuff, 1024);
    sedbuff[cont++] = recbuff[0];
    sedbuff[cont++] = recbuff[1];

    switch(recbuff[1]) {
    case 0x1:
        /* get in des */
        {
            BYTE *num = &(sedbuff[cont++]);
            char *des;
            for (i=0; i<6; i++) {
                des = matrix_get_des(i+1);
                strcpy( (char *)&sedbuff[cont++], des);
                cont += strlen(des);
                (*num)++;
            }
        }
        break;
    case 0x2:
        /* set matrix_indes*/
        {
            BYTE num;
            int ret = -1;
            cont = 2;
            num = recbuff[cont++];
            ret = matrix_set_des(num, &(recbuff[cont]));
            return ret;  /* not recv */
        }
        break;
    case 0x3:
        {
            BYTE num;
            int ret = -1;
            cont = 2;
            num = recbuff[2];
            sedbuff[cont++] = num;
            ret = matrix_get_inaddr(num);
            if( ret < 0)  return -1;
            sedbuff[cont++] = (BYTE)ret;
            break;

        }
	case 0x4:
		/* num 输出的端口, inline 返回此输出端口正在被line使用*/
		{
			int ret  = -1;
			BYTE line;
			BYTE num = recbuff[2];

			ret  = load_matrix_out_state(num);
			if ( ret == -1){
				line = 6;
			}else
				line = (BYTE)ret;
			sedbuff[cont++] = num;
			sedbuff[cont++] = line;
			break;

		}
    default:   break;
    }
    {
            usleep(5000);
            ret = write(connfd,  sedbuff, cont);
            #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
                if( ret < i)  {
                    net_debug(" sed mesg err(ret = %d)\n", ret);
                }
                usleep(5000);
    }

    return ret;
}
static int net_plan_info( int connfd, char *recbuff)
{
    BYTE sedbuff[10240];
    int cont=0,i=0, ret;
    bzero(sedbuff, 10240);

    sedbuff[cont++] = recbuff[0];
    sedbuff[cont++] = recbuff[1];
    switch(recbuff[1]) {
    case 0x1:
        /* get plan one*/
        {
            /**
             *  0x12 | 0x1 | num_plan | num_ac | ac_start.hour |
             *  ac_start.min | ac->id |...|.....|......
             */
            BYTE *num= NULL;
            struct st_plan *planpos;

            if( recbuff[2] > plan_max) {
                net_debug("net_plan_info get num is %d, plan_max = %d\n", recbuff[2], plan_max);
                break;
            }
            planpos = &(plan[(int)recbuff[2]]);
            sedbuff[cont++] = recbuff[2];
            num = &(sedbuff[cont++]);

            {
                struct list_head *pos;
                /* plan */
                list_for_each_prev(pos, &(planpos->ac_list )){
                    struct st_plan_action *ac  = list_entry( pos, struct st_plan_action, node);
                    sedbuff[cont++] = (BYTE)ac->start.hour;
                    sedbuff[cont++] = (BYTE)ac->start.min;
                    sedbuff[cont++] = (BYTE)ac->id;
                    (*num)++;
                }

            }
        }
        break;
    case 0x2:
        /* set plan */
        {
        	int ret;
        	int planpos = recbuff[2];
			ret = add_action( &(plan[planpos].ac_list), &recbuff[3]);
			if( ret < 0 ){
				sedbuff[cont++] = 0xf;
				break;
			}
			ret = plan_ini_save_ac(planpos, &recbuff[3]);
			if (ret < 0){
				sedbuff[cont++] = 0xf;
				break;
			}
			sedbuff[cont++] = 0x1;
        }
        break;
    case 0x3:
        /* get num_plan */
        {
            sedbuff[cont++] = time_get_num_plan();
			break;
        }
    case 0x4:
        {
		/* del num_plan */
		int moshi = (int)recbuff[2];
		char buff[32];
        strcpy(buff, (char *)&recbuff[3]);
		  ret = plan_del_ac(moshi, buff);
		  if ( ret < 0){
			sedbuff[cont++] = 0xf;
		  }else
		  	sedbuff[cont++] = 0x1;
        }
    break;
    default:
        break;
    }



    usleep(1000);
    ret = write(connfd,  sedbuff, cont);
            #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(5000);
	return ret;
}

/* 0x5 */
static int net_vol_info(int connfd, char *recbuff)
{
    int ret =-1, i=0;
    char sedbuff[512];
    struct st_lead_amp *amp;

    bzero( sedbuff, 512 );
    sedbuff[i++] = recbuff[0];
    sedbuff[i++] = recbuff[1];
    switch( recbuff[1] ) {
    case 0x1:
        {
            int k=0;
               sedbuff[i++] = recbuff[2];
               for( amp=glead_amp; amp; amp=amp->next)
                   if( ++k == recbuff[2]) break;
               if( !amp)  return -1;
               for(k=0; k < 6; k++) {
                   memcpy( &(sedbuff[i]), &(amp->connive[k]), sizeof(struct st_lead_amp_sound));
                   i += sizeof( struct st_lead_amp_sound);
               }

        }
        break;
    case 0x2:
        {
            int k=0, j= recbuff[3];
            struct st_lead_amp_sound connive;
            memcpy( &connive, &recbuff[4], sizeof connive);
            for( amp = glead_amp; amp; amp=amp->next)
                if(++k ==recbuff[2])  break;
            if(!amp)  return -1;
            lead_amp_load_sound(amp,j, &connive);
            /* set vol */
        }
		break;
    }
    usleep(200);

    ret = write(connfd,  sedbuff, i);
            #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, i);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(200);
    return ret;
}

/* 0x4 */
static int net_time_info(int connfd, char *buff)
{
    char sedbuff[32];
    int i=0,ret=-1;
    struct tm *p;
    time_t res;
    bzero( sedbuff, 32);

    switch(buff[1]) {
    case 0x1:
        {
                res = time(NULL);
                p = localtime(&res);
                sedbuff[i++] = buff[0];
                sedbuff[i++] = buff[1];
                sedbuff[i++] = p->tm_year;
                sedbuff[i++] = p->tm_mon+1;
                sedbuff[i++] = p->tm_mday;
                sedbuff[i++] = p->tm_hour;
                sedbuff[i++] = p->tm_min;
                sedbuff[i++] = p->tm_sec;
        }
        break;
    case 0x2:
        {
            char timebuff[64];
            bzero(timebuff, 64);
			printf("time\n");
            sprintf(timebuff, "%d-%d-%d %d:%d:%d",buff[2]+1900, buff[3], buff[4],buff[5],buff[6],buff[7] );
            set_system_time(timebuff);


        }
        break;

    }
usleep(100);
    ret = write(connfd,  sedbuff, i);
            #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, i);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(500);
    return ret;
}
//#include "caller.h"

/* 0x6*/
static int net_action_info(int connfd, char *buff)
{

    BYTE sed_buff[10240]; /* all action_list */
    struct st_action *action;
    int i=0, ret = 0;
    bzero(sed_buff, 10240);
    sed_buff[i++] = buff[0];
    sed_buff[i++] = buff[1];
    switch( buff[1]) {
    case 0x1:
        {
            BYTE *action_max;
            /* get action  for vb init action_list */
            /*  sed_buff[2] == action_max */
            action_max = &(sed_buff[i++]);

            /* action.c { int ac_max  (max action)}*/ /* chenxb bug ac_max is drop*/
            //*action_max = ac_max;
            /* for all action_list all
               action_max : des : amax: .... ';'des: amax:....*/
            /* add chenxb fix!! amax and ... is drop new
               action_max id des*/
            for(action = action_list; action; action=action->next) {
                (*action_max)++;
                sed_buff[i++] = action->id;
                //code_convert()
                strcpy( (char *)&sed_buff[i++], action->des);
                i += (strlen(action->des));
                //printf(" action_max = %d\n", *action_max);
                //amax = &(sed_buff[i++]);
                //list_for_each(pos, &(action->aclist)){
                //    (*amax) ++;
                //    unit = list_entry( pos, struct st_action_unit, node);
                //}
            }

        }
        break;
    case 0x2:
        {
            /* add action */
            //char tmpconvert[64];
            unsigned int gap;
            int str_len=0, maxac=0, con=2, i=0, ret=-1;//, convert_ret=-1;
            struct st_action *tmpaction;
            struct st_action_unit *unit;
            struct list_head *pos;

            INI *ini;
			pthread_mutex_lock(&malloc_flag_mutex);
            action = (struct st_action *)malloc(sizeof(*action));
			pthread_mutex_unlock(&malloc_flag_mutex);
            if( !action ) goto malloc_action_err;
            bzero( action, sizeof(*action));

            action->next = NULL;
            INIT_LIST_HEAD(&(action->aclist));
            action->id = get_ac_id();
			if (action->id < 0){
				goto freeaction;

			}

			gap = buff[con++]*60;
			/* 在这里 要更新 gap 新添加信息*/
			action->gap = gap +  buff[con++];

			strcpy(action->des, &(buff[con]));

            str_len = strlen(&buff[con])+1;
            con += str_len;
            /* add ac */
            maxac = buff[con++];
            //printf(" AC_max = %d\n", maxac);
            for(i=0; i < maxac; i++) {

               // bzero( tmpconvert, 64);
               // convert_ret = code_convert("unicode", "utf-8", &(buff[con+1]), strlen(&buff[con+1])+1, tmpconvert, 64);
               // if( convert_ret !=0 ) goto freeall;
                ret = add_ac( &(action->aclist), (BYTE *)&buff[con++]);
                con += strlen(&(buff[con-1])) ;
            }

            /* add action to list */
            if (!action_list ) {
                action_list = action;
            }else{
                for(tmpaction = action_list; tmpaction->next ; tmpaction=tmpaction->next);
                tmpaction->next = action;
            }

            /* add action to file */
            if( ret < 0 && i) {
                goto freeall;
            }
            ini = ini_open(AC_INI);
            /* ini == NULL free action free ac */
            if( !ini){
                goto  freeall;
            }
            ret = ac_add_segment_byid( ini, action->id,action->des, action->gap);
			if (ret < 0){
				 ini_close(ini);
                 goto freeall;
            }
			list_for_each_prev( pos, &(action->aclist)) {
                unit = list_entry(pos, struct st_action_unit, node);
                ret = ac_add_key_cfg(ini, action->id, unit);
                if( ret < 0)
                {
                	ini_close(ini);
                    goto freeall;
                }

            }
			ini_save(ini);
            ini_close(ini);
			break;

freeall:
{
            struct st_action_unit  *unit;
			printf(" free action\n");
            int flags =1;
            while(flags) {
                flags =0;
                list_for_each(pos, &(action->aclist)){
                    list_del(pos);
                    unit = list_entry(pos, struct st_action_unit, node);
                    flags =1;break;
                }
                if( flags ==1 ){
					if(unit->mp3sdata){
						pthread_mutex_lock(&malloc_flag_mutex);
						free(unit->mp3sdata);
						pthread_mutex_unlock(&malloc_flag_mutex);
					}
					free(unit);
                }
            }
}
freeaction:
    printf("freeaction\n");
			pthread_mutex_lock(&malloc_flag_mutex);
            free(action);
			pthread_mutex_unlock(&malloc_flag_mutex);

malloc_action_err:
		return ret;
    	}
        break;
    case 0x3:
        /* del action */
        {
            int con =2, ret=-1, id;
            struct st_action *action, *tmp1action;
            INI *ini;
            ini = ini_open(AC_INI);

            /* buff[0]=0x6, buff[1]=0x3, buff[2]=id*/
            id = buff[con];
            for( action=action_list; action; action=action->next) {
                if( action->id == id)  break;
            }
            if( !action)  return -1;
            /* del form memory list*/
            if( action == action_list) {
                int flags =1;
                struct list_head *pos;
                struct st_action_unit *unit;
                action_list = action->next;
                while(flags) {
                    flags =0;
                    list_for_each(pos, &(action->aclist)){
                        list_del(pos);
                        unit = list_entry(pos, struct st_action_unit, node);
                        flags =1;break;
                    }
                    if( flags ==1 ){
						if (unit->mp3sdata) free(unit->mp3sdata);
						free(unit);
                    }
                }
                free(action);
            }else{
                for(tmp1action=action_list; tmp1action->next != action; tmp1action=tmp1action->next);
                tmp1action->next = action->next;
                {
                    int flags =1;
                    struct list_head *pos;
                    struct st_action_unit *unit;
                    while(flags) {
                        flags =0;
                        list_for_each(pos, &(action->aclist)){
                            list_del(pos);
                            unit = list_entry(pos, struct st_action_unit, node);
                            flags =1;
                            break;
                        }
                        if( flags ==1 ){
							if( unit->mp3sdata) free( unit->mp3sdata);
							free(unit);
                        }
                    }
                    free(action);
                }

            }

            /* del ac_del_segment */
            ret = ac_del_segment_id_cfg(ini, id);
            if ( ret < 0)  return -1;
            ini_save(ini);
            ini_close(ini);
        }
        break;

    }
    {

    usleep(500);
    ret = write(connfd,  sed_buff, i);
           #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, i);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed_buff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(500);
    }

    return ret;
}


/*   N E T _ M P 3 D I R _ I N F O   */
/*-------------------------------------------------------------------------
    mp3 dir op    fn = 0x8
-------------------------------------------------------------------------*/
static int mp3scandir(char *dir, int depth, char *setbuf)
{
	int cont = 0;
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	if( (dp = opendir(dir)) == NULL){
		fprintf(stderr, "cannot open dir : %s\n", dir);
		return -1;
	}
	chdir(dir);

	while( (entry = readdir(dp)) != NULL){
		lstat(entry->d_name, &statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			/*找到一个目录， 但是要ignore .. 和 . */
			if (strcmp(".", entry->d_name) == 0 ||
				strcmp("..", entry->d_name) == 0)
				continue;
			mp3scandir(entry->d_name, depth+4, setbuf);
		}
		else{
			sprintf(&(setbuf[cont]), "%s", entry->d_name);
			cont += (strlen(entry->d_name) +1);
		}
	}
	chdir("..");
	closedir(dp);
	return cont;
}
static int net_mp3dir_info(int connfd, BYTE *buff)
{
	int ret = -1, cont=0;
	BYTE type = buff[1];
	BYTE sed[10240];

	bzero(sed, 10240);

	sed[cont++] = buff[0];
	sed[cont++] = buff[1];
	switch(type){
	case 0x1:
		/*取得mp3目录信息*/
		ret = mp3scandir(MUSIC_DIR,0,(char *)&(sed[cont++]));
		if ( ret < 0)  return ret;
		cont += ret;
		break;
	default:
		printf("no fun\n");
		break;
	}

    {
		int i;
    	usleep(100);
    	ret = write(connfd,  sed, cont);
	  	 #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed[i]);
                }
                printf("\n");
		#endif
    	if( ret < i)  {
     	   net_debug(" sed mesg err(ret = %d)\n", ret);
    	}
   		usleep(100);
    }


	return ret;
}

/*   N E T _ M P 3 D E L _ I N F O   */
/*-------------------------------------------------------------------------
    删除mp3的动作 fn = 0x9
-------------------------------------------------------------------------*/
static int net_mp3del_info(int connfd, BYTE *buff)
{
	BYTE sed[32],tmp[1024], *ptr;
	int cont = 0, ret;
	sed[cont++] = buff[0];
	ptr = &buff[1];
	bzero(tmp, 1024);
	sprintf((char *)tmp,"cd /music && rm %s ", ptr);

	system((char *)tmp);
	sed[cont++] = 0xf;
    {
		int i;
    	usleep(100);
    	ret = write(connfd,  sed, cont);
	 #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed[i]);
                }
                printf("\n");
	#endif
    	if( ret < i)  {
     	   net_debug(" sed mesg err(ret = %d)\n", ret);
    	}
   		usleep(100);
    }

	return ret;
}
/*   N E T _ U P M P 3 _ I N F O   */
/*-------------------------------------------------------------------------
    mp3 update  fn = 0x7
-------------------------------------------------------------------------*/
	static int filefd;
	static char updatename[1024];

static int net_upmp3_info(int connfd, BYTE *buff)
{
		 //   printf("0x7\n");

		int j=1, ret = -1;
		BYTE bpack_num[4];
		BYTE sed_buff[2];
		char tmp[1024];
		/* exit update*/

		if ( buff[1] == 0xf) {
			update = 0;
			if( filefd != 0){
				close(filefd);
				remove(updatename);
			}
			goto ok;
		}
		for( j=0; j < 4; j++) {
			bpack_num[j] =buff[1+j];
	  //	  printf(" bpack_num = %x", bpack_num[j]);
		}
		j = 1;
	   // printf("\n");
		bzero(tmp, 1024);
		if( bpack_num[0] == 'S'  && update == 0) {
			bzero(updatename,1024);
			strcpy(updatename, (char *)&buff[5]);
			sprintf(tmp, "%s/%s", MUSIC_DIR,&buff[5]);
			printf("file:	 %s\n", tmp);
			filefd = open(tmp, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR );
			if( filefd <= 0) {
				update=0;
				goto err;
			}else{
				update=1;
				goto ok;
			}
		}
		if( update ) {
			if( bpack_num[0] != 0x45) {
				int  ret;
				ret = write(filefd, &buff[5], 4092);
				if( ret != 4092 ) {
					goto err;
				}
				goto ok;
			}else{
				int  ret;
				int size = bpack_num[1]*255 + bpack_num[2] ;
				printf("size = %d\n", size);
				ret = write(filefd, &buff[5], size);
				close(filefd);
				filefd = 0;
				update = 0;
				goto ok;
			}
		}

		err:
				update=0;
				sed_buff[1] = 0xf;
				j++;
		ok:
				sed_buff[0] = 0x7;
				usleep(500);
				ret = write(connfd,  sed_buff, j);
				if( ret != j) {
					printf("net_upmp3_info, err sed ok\n");
					ret = -1;
				}
				{
              #ifdef NET_DEBUG
						printf("net : %x, %x\n", ret, j);
						for( j=0 ; j < ret; j++) {
							printf(" %x", sed_buff[j]);
						}
						printf("\n");
               #endif
				}
		if( ret < j)  {
			net_debug(" sed mesg err(ret = %d)\n", ret);
		}
	return ret;
}
static int net_set_ok(int connfd, BYTE *recbuff, BYTE num)
{
    char sedbuff[32];
    int i=0;

    bzero(sedbuff, 32);
    sedbuff[i++] = num;
    {
    int ret;
    usleep(100);
    ret = write(connfd,  sedbuff, i);
           #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, i);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(100);
    return ret;
    }

}
//extern int ARM_STATE;

/*   N E T _ S E L E C T _ I N F O   */
/*-------------------------------------------------------------------------
    矩阵 0x10
-------------------------------------------------------------------------*/
static int net_select_info(int connfd, char *buff)
{
	int ret =-1, cont=2;
	BYTE type = buff[1];
	BYTE sed[10240];
	bzero(sed, 10240);
	struct st_select_amp *amp;

	sed[0] = buff[0];
	sed[1] = buff[1];
	switch(type){
	case 0x1:
		{
			int i;
			BYTE *num;
			num = &(sed[cont++]);
			for( amp = gselect_amp; amp; amp = amp->next){
				(*num)++;
				sed[cont++] = (BYTE)amp->dev.addr;
				sed[cont++] = (BYTE)amp->in;
				sed[cont++] = (BYTE)amp->out;
				for(i = 0; i < 10; i++){
					sed[cont++] = (BYTE)select_amp_get_sound_state(amp, i+1);
				}
				strcpy( (char *)&(sed[cont++]), amp->dev.des);
				cont += strlen(amp->dev.des);
			}
		}
		break;
	default:
		net_debug("select_info:err\n");
		break;
	}

    {
    int ret, i;
    usleep(500);
    ret = write(connfd,  sed, cont);
	   #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed[i]);
                }
                printf("\n");
		#endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(50000);
    }

	return ret;
}




/*   N E T _ S H O U D O N G _ I N F O   */
/*-------------------------------------------------------------------------
    手动模式下命令的操作
-------------------------------------------------------------------------*/
int net_shoudong_info(int connfd, char *buff)
{
	char sedbuff[32];
    int cont=0;
    bzero(sedbuff, 32);
    sedbuff[cont++] = buff[0];
	sedbuff[cont++] = buff[1];
	switch(buff[1]){
	case 0x1:
		/*非MP3 */
		{
			struct st_action_unit unit;
			int ret = 0;
			bzero(&unit, sizeof unit);

			unit.dev_addr = (BYTE)buff[2];
			unit.sdata[0] = (BYTE)buff[3];
			unit.sdata[1] = (BYTE)buff[4];
			unit.ssize = 2;
			ret = dispersion(&unit);
			if ( ret < 0){
				sedbuff[cont++] = 0xf;
			}else
				sedbuff[cont++] = 0x1;
		}
		break;
	case 0x2:
		{
			struct st_action_unit unit;
			int ret = -1, len;
			bzero(&unit, sizeof unit);
			unit.dev_addr = (BYTE)buff[2];
			unit.sdata[0] = (BYTE)buff[3];
			len = strlen(&buff[4]) +1;
			pthread_mutex_lock(&malloc_flag_mutex);
			unit.mp3sdata = malloc(len);
			pthread_mutex_unlock(&malloc_flag_mutex);
			if (!unit.mp3sdata){
				printf("net: shoudong malloc err\n");
				goto err;
			}
			strcpy((char *)(unit.mp3sdata), &buff[4] );
			ret = dispersion( &unit);
            free(unit.mp3sdata);
            unit.mp3sdata = NULL;
			if (ret < 0)
err:			sedbuff[cont++] = 0xf;
			else
				sedbuff[cont++] = 0x1;
		}

		break;
	}


	{
    int ret, i;
    usleep(100);
    ret = write(connfd,  sedbuff, cont);
           #ifdef NET_DEBUG
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sedbuff[i]);
                }
                printf("\n");
            #endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(100);
    return ret;
    }
}


/*   N E T _ R A D I O _ I N F O   */
/*-------------------------------------------------------------------------
     radio  收音机
-------------------------------------------------------------------------*/
static int net_radio_info(int connfd, char *buff)
{
	int ret =-1, cont=2;
	BYTE type = buff[1];
	BYTE sed[32];
	bzero(sed, 32);

	sed[0] = buff[0];
	sed[1] = buff[1];
	switch(type){
	case 0x1:
		{
			struct st_action_unit unit;
			BYTE op = buff[2];
			bzero(&unit, sizeof unit);

			unit.dev_addr = MATRIX_BASE_ADDR;
			unit.sdata[0] = 5;
			unit.sdata[1] = op;
			unit.ssize = 2;
			ret = dispersion(&unit);
			if (ret < 0){
				net_debug("net_radio_info 0x1 err\n");
				goto _0x1_err;
			}
			sed[cont++] = 0x1;
			break;
_0x1_err:
			sed[cont++] = 0xfa;
			break;
		}
	case 0x2:
		{
			struct st_action_unit unit;
			bzero (&unit, sizeof unit);

			unit.dev_addr = MATRIX_BASE_ADDR;
			unit.sdata[0] = 5;
			unit.sdata[1] = 0;
			unit.ssize = 2;
			ret = dispersion(&unit);
			if(ret < 0){
				net_debug("net_radio_info 0x2 err\n");
				goto _0x2_err;
			}
			sed[cont++] = 0x1;
			break;
_0x2_err:
			sed[cont++] = 0xfa;
			break;
		}
	default:
		net_debug("select_info:err\n");
		break;
	}

    {
    int ret, i;
    usleep(500);
    ret = write(connfd,  sed, cont);
	   #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed[i]);
                }
                printf("\n");
		#endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(500);
    }

	return ret;
}



/*   N E T _ F I R E _ I N F O   */
/*-------------------------------------------------------------------------
    区域火警 网络接口    0x15
-------------------------------------------------------------------------*/
static int net_fire_info(int connfd, char *buff)
{
	int ret =-1, cont=2;
	BYTE type = buff[1];
	BYTE sed[32];
	bzero(sed, 32);

	sed[0] = buff[0];
	sed[1] = buff[1];
	switch(type){
	case 0x1:
		{
			sed[cont++] = fire_area;

		}
		break;
	case 0x2:
		{
			if ( 1 == buff[2] )
				fire_area = 1;
			else
				fire_area = 0;

			/* 保存到 ini 里面*/
			{
				INI *ini;
				int ret;
				ini = ini_open(DEV_INI);
				if (!ini) return -1;

				ret = ini_set_int(ini, "FIRE_AREA", fire_area);
				if(!ret) ini_append_int(ini,"FIRE_AREA",fire_area);
				ini_save(ini);
				ini_close(ini);
			}
		}
	default:
		net_debug("fire_info:err\n");
		break;
	}

    {
    int ret, i;
    usleep(500);
    ret = write(connfd,  sed, cont);
	   #ifdef NET_DEBUG
                printf("net : %x, %x\n", ret, cont);
                for( i=0 ; i < ret; i++) {
                    printf(" %x", sed[i]);
                }
                printf("\n");
		#endif
    if( ret < i)  {
        net_debug(" sed mesg err(ret = %d)\n", ret);
    }
    usleep(500);
    }

	return ret;
}

/*   N E T _ D I S P E R S I O N   */
/*-------------------------------------------------------------------------
    网络 总接口
-------------------------------------------------------------------------*/
int net_dispersion(int connfd, char *buff)
{
    int ret = 0;
    BYTE type = buff[0];

    switch( type) {
    case 0x1:
        start_plan();
        net_set_ok(connfd, (BYTE *)buff,0x1);
        net_debug( "start_plan\n");
        break;
    case 0x2:
        stop_plan();
        net_set_ok(connfd, (BYTE *)buff,0x2);
        net_debug( "stop_plan\n");
        break;
    case 0x3:
     //   ret = net_get_sound_info(connfd, buff);
        break;
    case 0x4:
            /* systime */
        ret = net_time_info(connfd, buff);
        break;

    case 0x5:
        ret = net_vol_info(connfd, buff);
        break;
    case 0x6:
        ret = net_action_info(connfd, buff);
        break;
    case 0x7:
		ret = net_upmp3_info(connfd, (BYTE *)buff);
    	break;
	case 0x8:
		ret = net_mp3dir_info(connfd, (BYTE *)buff);
		break;
	case 0x9:
		ret = net_mp3del_info(connfd, (BYTE *)buff);
		break;
    case 0x10:
        ret = net_select_info(connfd, buff);
        break;
    case 0x11:
        ret = net_matrix_info(connfd, buff);
       // ret = net_get_matrix_info(connfd, buff);
        break;
    case 0x12:
        ret = net_plan_info(connfd, buff);
        //ret = net_get_plan_info(connfd, buff);
        break;
	case 0x13:
		ret = net_shoudong_info(connfd, buff);
		break;
	case 0x14:
		ret = net_radio_info(connfd, buff);
		break;
	case 0x15:
		ret = net_fire_info(connfd, buff);
		break;
	case 0xff:
		ret = net_app_up(connfd, buff);
		break;
    case 0xfe:
        ret = net_planini_up(connfd, buff);
		break;
	case 0xfd:
		ret = net_devini_up(connfd, buff);
		break;
	case 0xfc:
		ret = net_actionini_up(connfd, buff);
		break;
    default:
        net_debug("(net_dispersion): no fn\n");
    }
    return ret;
}



pthread_mutex_t select_flag_mutex = PTHREAD_MUTEX_INITIALIZER;


void *net_run_server(void *arg)
{
	int connfd = *(int *)arg,ret;
	BYTE netbuff[5];
    struct timeval tv;
	int maxfd=0;
	fd_set readfds;


	printf(" == new <-> connfd: %d,  maxfd = %d, net_run_server == \n", connfd, maxfd);
	while (1){

		FD_ZERO(&readfds);
		FD_SET(connfd, &readfds);
		maxfd = connfd;
		tv.tv_sec = 3;
    	tv.tv_usec = 0;
//		pthread_mutex_lock(&select_flag_mutex);
		ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);
//		pthread_mutex_unlock(&select_flag_mutex);

		if( ret == -1) {
			printf("err select\n");
			break;
		}
		else if ( ret == 0){
			printf(" connfd = %d\n ", connfd);
            continue;
		}else{
			int isset_ret = FD_ISSET(connfd, &readfds);
            if ( isset_ret > 0 ){
                BYTE *pbuff= NULL;
                unsigned int size;
                BYTE tmpbuff[5120];
				int rec_ret;
fix1:  		rec_ret = recv(connfd, netbuff, 1, 0) ;
       			if( rec_ret == 1 ){
                    if ( netbuff[0] != 0xf1) goto fix1;
                }else if ( rec_ret <= 0) break;
				else continue;

fix2:		   rec_ret = recv(connfd, netbuff, 1, 0) ;
        		if ( rec_ret == 1 ){
                    if ( netbuff[0] != 0xf2) goto fix2;
                }else if ( rec_ret <= 0) break;
				else continue;

fix3:		   rec_ret = recv(connfd, netbuff, 1, 0) ;
		       if (rec_ret == 1 ){
                    if ( netbuff[0] != 0xf3) goto fix3;
                }else if ( rec_ret <= 0 ) break;
			   else continue;

				rec_ret = recv(connfd, netbuff, 2, 0);
                if ( rec_ret ==  2 ){
                    size = netbuff[1];
                    size <<= 8;
                    size |= netbuff[0];
                 //   printf("size = %d netbuff[0] = %d, netbuff[1] = %d\n", size, netbuff[0], netbuff[1] );
                }else if ( rec_ret <= 0 ) break;
				else continue;

                if( size < 5120 ){
                    unsigned int real_size = 0, cont = 30;
                    int num = 0;
fix4:				num = recv(connfd, &tmpbuff[num], size, 0) ;
					if ( num <= 0 ) break;
					else if ( (real_size += num) < size && cont-- ) goto fix4;
                    if (real_size < size) continue;
                    net_dispersion(connfd,tmpbuff);
                }else{
                	//printf(", size = %d\n", size);
                    pthread_mutex_lock(&malloc_flag_mutex);
                    pbuff = ( BYTE *)malloc(size*sizeof(BYTE));
                    pthread_mutex_unlock(&malloc_flag_mutex);
                    if(!pbuff) continue;
                    bzero(pbuff, size*sizeof(char));

                    if( recv(connfd, pbuff, size, 0) < size){
                        printf("recv SIZE = %d\n", size);
                        continue;
                    }

                    net_dispersion(connfd,pbuff);
                }

			   if (pbuff){
                pthread_mutex_lock(&malloc_flag_mutex);
                free(pbuff);
                pbuff = NULL;
                pthread_mutex_unlock(&malloc_flag_mutex);
			   }
			}else if ( isset_ret == 0 ) break;
			else break;

			}
		}
	close(connfd);
	printf(" == free <->connfd: %d,  net_run_server== \n", connfd);
    return NULL;
}

void *net_run(void *arg)
{
    int listenfd, connfd=0;
    socklen_t  chilen;
	int ret;
	pthread_t run_id;
    struct sockaddr_in childaddr, servaddr;

age:
	bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family   = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(NET_PORT);
    listenfd = socket(AF_INET,  SOCK_STREAM, 0);

    ret=	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt, sizeof(sock_opt));
    if (ret < 0) {
		net_debug("(create net): setsockopt err\n ");
		return NULL;
		goto age;
	}
    ret =     bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if( ret < 0) {
		net_debug("(create net): bind err\n");
		goto age;
	}
    ret =    listen( listenfd,  5 );
    if( ret < 0) {
		net_debug("(create net): listen err \n");
		goto age;
	}
    for(;;) {
        chilen = sizeof( childaddr);
        net_debug("(start net\n)");
        connfd = accept( listenfd, ( struct sockaddr *)&childaddr, &chilen);
        if (connfd == -1){
		switch( errno){
			case ECONNABORTED:
				net_debug(" a connection has been aborted\n ");
			break;
			case EPERM:
				net_debug( "firewall rules \n");
			break;
			case ENFILE:
				net_debug("The sys limit on\n");
			break;
			case EINTR:
				net_debug("the system call was \n");
			break;
		default:
			net_debug("unknow error %d\n", errno);
			break;
		}
		continue;
	}

{

     int res= -1;
  run_id = 0;
	    res = pthread_create(&run_id, NULL, (void *)net_run_server, &connfd);
    if ( res != 0 ){
        printf("err create pthread\n");
    }
}
	sleep(3);

    }
}



void *net_timer_run(void *arg)
{
    int listenfd, connfd=0;
    socklen_t  chilen;
	int ret;
	pthread_t run_id;
    struct sockaddr_in childaddr, servaddr;

age:
	bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family   = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(NET_TIMR_PORT);
    listenfd = socket(AF_INET,  SOCK_STREAM, 0);

    ret=	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt, sizeof(sock_opt));
    if (ret < 0) {
		net_debug("(create net): setsockopt err\n ");
		return NULL;
		goto age;
	}
    ret =     bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if( ret < 0) {
		net_debug("(create net): bind err\n");
		goto age;
	}
    ret =    listen( listenfd,  5 );
    if( ret < 0) {
		net_debug("(create net): listen err \n");
		goto age;
	}
    for(;;) {
        chilen = sizeof( childaddr);
        net_debug("(start net\n)");
        connfd = accept( listenfd, ( struct sockaddr *)&childaddr, &chilen);
        if (connfd == -1){
		switch( errno){
			case ECONNABORTED:
				net_debug(" a connection has been aborted\n ");
			break;
			case EPERM:
				net_debug( "firewall rules \n");
			break;
			case ENFILE:
				net_debug("The sys limit on\n");
			break;
			case EINTR:
				net_debug("the system call was \n");
			break;
		default:
			net_debug("unknow error %d\n", errno);
			break;
		}
		continue;
	}

{

     int res= -1;
  run_id = 0;
	    res = pthread_create(&run_id, NULL, (void *)net_run_server, &connfd);
    if ( res != 0 ){
        printf("err create pthread\n");
    }
}
	sleep(3);

    }
}

void net_start()
{
    int res;
    pthread_t  runid;

	res = pthread_create(&runid, NULL, (void *)net_timer_run, NULL);
    if( res != 0 ) {
        net_debug("net_timer_run pthread is err\n");
        exit(res);
    }

    res = pthread_create(&runid, NULL, (void *)net_run, NULL);
    if( res != 0 ) {
        net_debug("net_run pthread is err\n");
        exit(res);
    }
}
