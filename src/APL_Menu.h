#ifndef	APL_MENU_H
#define	APL_MENU_H

#include <usr_define.h>

typedef struct
{
    int16_t     (*MenuFunc)(int16_t, int16_t);    /* ���s�֐� */
    int16_t     nType;                            /* ���j���[��� */
    int8_t*     pbSelect;                         /* ���j���[�I����e */
    int8_t      bSelectMin;                       /* ���j���[�I����e(�ŏ�) */
    int8_t      bSelectMax;                       /* ���j���[�I����e(�ő�) */
    int8_t*     sMenuName;                        /* ���j���[�� */
    int8_t*     sHelpMess;                        /* ���j���[�I�����̃w���v */
}   ST_MENU_DATA;

typedef struct
{
    ST_PCG      stPCG;                  /* �J�[�\�� */
	int16_t	    nSelectMenu;		    /* ���j���[�ԍ� */
	int16_t	    nSelectMenu_Old;		/* ���j���[�ԍ�(�O��l) */
	int8_t		sDispMess[64];	        /* �X�e�[�^�X */
	int16_t		nNum;		        	/* �i���o�[ */
	int16_t		nMenuX;		        	/* x���W */
	int16_t		nMenuY;	        		/* y���W */
}	ST_MENU;

extern uint8_t	g_STG68K_mode;
extern uint8_t	g_STG68K_stage;
extern uint8_t	g_STG68K_PAL_num;
extern uint8_t	g_STG68K_View;
extern uint8_t	g_STG68K_save;

extern void APL_Menu_Init(void);
extern void APL_Menu_Exit(void);
extern void APL_Menu_On(void);
extern void APL_Menu_Off(void);
extern int16_t APL_Menu_Mess(int16_t);
extern int16_t APL_Menu_Proc(void);

#endif	/* APL_MENU_H */
