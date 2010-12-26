#include "trigger.h"
#include "caller.h"
#include "plan.h"
#include "matrix.h"
#include "select_amp.h"


int fire_area;
#if 0
static void sound_run(void)
{
    system("rm /test.wav");
    system("cat /dev/dsp > test.wav");
}
#endif
static void *trigger_run(void *arg)
{
	int buttons_fd;
    int flags=0, flags2=0;


    struct st_caller *caller;
    buttons_fd = open("/dev/buttons", 0);
	if (buttons_fd < 0) {
		perror("open device buttons");
		exit(1);
	}
#if 1
	for (;;) {
		char current_buttons[6];
		if (read(buttons_fd, current_buttons, sizeof current_buttons) != sizeof current_buttons) {
			perror("read buttons:");
			exit(1);
		}

		if (current_buttons[3] == '1' && flags2== 0 ){
			usleep(5000);
		}
		if (current_buttons[3] == '0' && flags2 == 1){
			usleep(5000);
		}
		if (current_buttons[3] == '1' && flags2 == 0){
			int ret;
			struct st_matrix *matrix;
			struct st_action_unit unit;
			bzero(&unit, sizeof unit);
			flags2 = 1;

			pthread_mutex_lock( &plan_mutex);
			matrix = gmatrix;
			if (!matrix){
				printf("head_caller err, matrix is NULL\n");
				pthread_mutex_unlock(&plan_mutex);
				break;
			}
			if (current_buttons[4] == '0' && flags ==1 && ARM_STATE == CALLER_SEND_BUSY){
              flags = 0;
              pthread_mutex_lock( &plan_mutex);
              caller_op(CALLER_SEND_IDEL);
              pthread_mutex_unlock( &plan_mutex);
              printf(" CALLER - BUS = %x\n", ARM_STATE);

       		}

			ARM_STATE = CALLER_SEND_BUSY;
			matrix->matrix_state[MATRIX_STATE] = MATRIX_CALLER;
			unit.dev_addr = matrix->dev.addr;
			unit.sdata[0] = 1;
			unit.sdata[1] = 0xff;
			unit.ssize 	  = 2;
			ret = dispersion(&unit);
			if (ret < 0){
				printf("head_caller err, dispersion return err\n");
				matrix->matrix_state[MATRIX_STATE] = MATRIX_IDLE;
				pthread_mutex_unlock(&plan_mutex);
				break;
			}
			matrix->matrix_state[MATRIX_STATE] = MATRIX_BUSY;

			pthread_mutex_unlock( &plan_mutex);
			}
			else if ( current_buttons[3] == '0' && flags2 == 1 ){
				struct st_matrix *matrix;

				int ret;
				matrix = gmatrix;
				printf("free\n");
                //if( matrix->matrix_state[MATRIX_STATE] == MATRIX_BUSY) {
                //    matrix->matrix_state[MATRIX_STATE] = MATRIX_CALLER;
                    ret = matrix_load_state(matrix);
                    if( ret < 0)  matrix->matrix_state[MATRIX_STATE] =  MATRIX_ERR;
                    else matrix->matrix_state[MATRIX_STATE] = MATRIX_IDLE;
                //}

            {
                struct st_action_unit new_unit;
                struct st_select_amp *amp;
                bzero(&new_unit, sizeof(new_unit));

                for( amp= gselect_amp; amp; amp = amp->next) {
                    if( amp->select_state[SELECT_AMP_STATE] == SELECT_AMP_BUSY){
                        int ret;
                        amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_CALLER;
                        ret = select_amp_load_state(amp);
                        if( ret < 0){
                               amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_ERR;
                               continue;
                        }
                        amp->select_state[SELECT_AMP_STATE] = SELECT_AMP_IDLE;
                	}
                }
            }

			ARM_STATE = CALLER_SEND_IDEL;
			flags2 = 0;
		}
        if(current_buttons[4] == '1' && flags == 0 && ARM_STATE == CALLER_SEND_IDEL) {
            usleep(5000);
        }
           if( current_buttons[4] == '1' && flags == 0 && ARM_STATE == CALLER_SEND_IDEL) {
               int ret;
               flags =1;
               pthread_mutex_lock( &plan_mutex);
               ret = caller_op( CALLER_SEND_LOOP );
               if( ret < 0 ){
                   flags = 0;  pthread_mutex_unlock( &plan_mutex);
               }
               pthread_mutex_unlock( &plan_mutex);
               for(caller=gcaller; caller; caller = caller->next) {
               printf("caller: \n\taddr=%x\n\tstate = %x\n", caller->dev.addr, caller->caller_state);
        }
        printf(" CALLER - BUS = %x\n", ARM_STATE);
        }else if (current_buttons[4] == '0' && flags ==1 && ARM_STATE == CALLER_SEND_BUSY){
              flags = 0;
              pthread_mutex_lock( &plan_mutex);
              caller_op(CALLER_SEND_IDEL);
              pthread_mutex_unlock( &plan_mutex);
              for(caller=gcaller; caller; caller = caller->next) {
              printf("caller: \n\taddr=%x\n\tstate = %x\n", caller->dev.addr, caller->caller_state);
              }
                printf(" CALLER - BUS = %x\n", ARM_STATE);
 //               system("madplay /0.mp3");
        }
        sleep(1);
    }
#endif
	close(buttons_fd);
	return NULL;
}

void trigger_start()
{
    int res;
    pthread_t  runid;
    res = pthread_create(&runid, NULL, (void *)trigger_run, NULL);
    if( res != 0 ) {
        trigger_debug("( trigger_start):  pthread_create  is err\n");
        exit(res);
    }

}


#if 0
static void fire_start()
{

}
#endif
