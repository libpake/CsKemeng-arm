/**
 * matrix.h 
 */
 #ifndef _MATRIX_H_

 #define _MXTRIX_H_

#include "rs232.h"
#include "dev.h"
#include "ini.h"
#include "debug.h"
#include "action.h"

#define MATRIX_BASE_ADDR 0x60
#define MATRIX_TYPE    MATRIX_BASE_ADDR



/* config */
#define MATRIX_CONFIG_MAX "MATRIX.MAX"
#define MATRIX_CONFIG_DES "MATRIX.DES"
#define MATRIX_CONFIG_POWER "MATRIX.POWER"
#define MATRIX_CONFIG_UART "MATRIX.UART"
#define MATRIX_CONFIG_ADDR "MATRIX.ADDR"
#define MATRIX_CONFIG_IN1_DES "MATRIX.INONE"
#define MATRIX_CONFIG_IN2_DES "MATRIX.INTOW"
#define MATRIX_CONFIG_IN3_DES "MATRIX.INTHREE"
#define MATRIX_CONFIG_IN4_DES "MATRIX.INFOUR"
#define MATRIX_CONFIG_IN5_DES "MATRIX.INFIVE"
#define MATRIX_CONFIG_IN6_DES "MATRIX.INSIX"

#define MATRIX_LINE_MAX 6
#define MATRIX_STATE_MAX (MATRIX_LINE_MAX+1)

#define MATRIX_STATE (MATRIX_STATE_MAX -1)

#define MATRIX_STRING_MAXLEN 64


#define CALLER_LINE_SET_ON 2 /* 6x6 input */
#define ARM_SOUND_SET_ON 3   /* 6x6 input */
enum{
    MATRIX_IDLE,
    MATRIX_CALLER,
    MATRIX_BUSY,
    MATRIX_ERR=-1
};

struct st_matrix{
	/**
		state [0] = BYTE 0 0 0 0 0 0  LINE 1 ‰»Î   BYTE 0 0 0 0 0 0  ‰≥ˆ
	*/
    WORD matrix_state[MATRIX_STATE_MAX];
    struct st_dev   dev;
    struct st_matrix *next;

    char indes[MATRIX_LINE_MAX][MATRIX_STRING_MAXLEN];
};



extern int matrix_load_cfg(INI *ini);
extern int matrix_rec_op(struct st_action_unit *unit);
extern struct  st_matrix *seach_matrix( int addr);
extern int matrix_load_state(struct st_matrix *matrix);
extern int matrix_load_state_ex(struct st_matrix *matrix, WORD *pdate, int len);


extern void save_matrix_ex( WORD *pdata, struct st_action_unit *unit);
extern void save_matrix(struct st_action_unit *unit);

extern char *matrix_get_des(int line);
extern int load_matrix_out_state( int outNum);
extern int matrix_set_des(int line, char *des);
extern int matrix_get_state(WORD *pdate,int len, int flag);

extern int matrix_get_inaddr(int num);


extern struct st_matrix *gmatrix;
extern WORD matrix_caller[MATRIX_LINE_MAX];

 #endif
