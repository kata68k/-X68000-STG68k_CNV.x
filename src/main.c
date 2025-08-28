#ifndef	MAIN_C
#define	MAIN_C

#include <iocslib.h>
#include <stdio.h>
#include <stdlib.h>
#include <doslib.h>
#include <io.h>
#include <math.h>
#include <time.h>
#include <interrupt.h>

#include <usr_macro.h>
#include <apicglib.h>

#include "main.h"

#include "BIOS_CRTC.h"
#include "BIOS_DMAC.h"
#include "BIOS_MFP.h"
#include "BIOS_PCG.h"
#include "BIOS_MPU.h"
#include "IF_Draw.h"
#include "IF_FileManager.h"
#include "IF_Graphic.h"
#include "IF_Input.h"
#include "IF_MACS.h"
#include "IF_Math.h"
#include "IF_Memory.h"
#include "IF_Mouse.h"
#include "IF_Music.h"
#include "IF_PCG.h"
#include "IF_Text.h"
#include "IF_Task.h"
#include "APL_Menu.h"
#include "APL_PCG.h"
#include "APL_Score.h"

//#define 	W_BUFFER	/* �_�u���o�b�t�@�����O���[�h */
//#define	FPS_MONI	/* FPS�v�� */
//#define	CPU_MONI	/* CPU�v�� */
#define	MEM_MONI	/* MEM�v�� */

enum
{
	STG_SM_SP,
	STG_SM_PAL,
	STG_68_PTN,
	STG_68_COL,
	STG_68_PAL,
};

enum
{
	VIEW_SP,
	VIEW_STG68K,
};

enum
{
	GR_Screen0,
	GR_Screen1,
};


/* �O���[�o���ϐ� */
uint32_t	g_unTime_cal = 0u;
uint32_t	g_unTime_cal_PH = 0u;
uint32_t	g_unTime_Pass = 0xFFFFFFFFul;
int32_t		g_nCrtmod = 0;
int32_t		g_nMaxUseMemSize;
int16_t		g_CpuTime = 0;
uint8_t		g_mode;
uint8_t		g_mode_rev;
uint8_t		g_Vwait = 1;
uint8_t		g_bFlip = FALSE;
uint8_t		g_bFlip_old = FALSE;
uint16_t	g_unFrameCount;
#ifdef FPS_MONI
uint8_t		g_bFPS_PH = 0u;
#endif
uint8_t		g_bExit = TRUE;
int16_t		g_GameSpeed;

uint8_t		g_ubDemoPlay = FALSE;
uint8_t		g_ubPhantomX = FALSE;

/* �O���[�o���\���� */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
uint16_t	g_uDebugNum = 0;
//uint16_t	g_uDebugNum = (Bit_7|Bit_4|Bit_0);
#endif
int8_t	g_sFileName[5][32] = {0};
int32_t	g_nFileSize[5] = {0};
int8_t	*g_pSP_Buf;
int8_t	*g_pPAL_Buf;
int8_t	*g_pSTG68_PTN_Buf;
int8_t	*g_pSTG68_COL_Buf;
int8_t	*g_pSTG68_PAL_Buf;

enum
{
	STG68K_SHIP,
	STG68K_ENEMTY,
	STG68K_BOSS,
	STG68K_BG,
	STG68K_SCORE,
	STG68K_MAX,
};

int32_t g_nSTG68_PTN_MAP[] = {
	0x14000,	/* ���@ */
	0x08000,	/* �G */
	0x0E000,	/* �{�X */
	0x00000,	/* �w�i */
	0x14000,	/* �X�R�A */
};

int32_t g_nSTG68_PAL_MAP[] = {
	0x280,	/* ���@ */
	0x100,	/* �G */
	0x1C0,	/* �{�X */
	0x000,	/* �w�i */
	0x280,	/* �X�R�A */
};

const int16_t nTboxTable_X[4] = { 32, 32, 16, 16 };
const int16_t nTboxTable_Y[4] = { 32, 16, 32, 16 };
int16_t nTboxTableCount = 0;
int16_t	g_nTboxCursorPosX[2] = {0};
int16_t	g_nTboxCursorPosY[2] = {0};
int16_t	g_nTboxCursorPosX_ofst[2] = {0, 256};
int16_t	g_nTboxCursorWidth = 32;
int16_t	g_nTboxCursorHeight = 32;
int16_t	g_nTboxCursorPosX_O[2] = {0};
int16_t	g_nTboxCursorPosY_O[2] = {0};
int16_t	g_nTboxCursorWidth_O = 32;
int16_t	g_nTboxCursorHeight_O = 32;

/* �֐��̃v���g�^�C�v�錾 */
int16_t main(int16_t, int8_t**);
static void App_Init(void);
static void App_exit(void);
int16_t	BG_main(uint32_t);
void App_TimerProc( void );
int16_t App_RasterProc( uint16_t * );
void App_VsyncProc( void );
void App_HsyncProc( void );
int16_t	App_FlipProc( void );
int16_t	SetFlip(uint8_t);
int16_t	GetGameMode(uint8_t *);
int16_t	SetGameMode(uint8_t);

int16_t STG68K_SPPAL_to_PTNCOLPAL_Cnv(int16_t);

void (*usr_abort)(void);	/* ���[�U�̃A�{�[�g�����֐� */

/* �֐� */
/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t main_Task(void)
{
	int16_t	ret = 0;
	uint16_t	uFreeRunCount=0u;
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	uint32_t	unTime_cal = 0u;
	uint32_t	unTime_cal_PH = 0u;
#endif

	int16_t	loop;
	int16_t	pushCount = 0;

	uint8_t	bMode;
	uint8_t	bViewMode = VIEW_SP;
	
	ST_TASK		stTask = {0}; 
	ST_CRT		stCRT;
	
	/* �ϐ��̏����� */
#ifdef W_BUFFER
	SetGameMode(1);
#else
	SetGameMode(0);
#endif	
	loop = 1;
	g_unFrameCount = 0;
	
	/* ���� */
	srandom(TIMEGET());	/* �����̏����� */
	
	do	/* ���C�����[�v���� */
	{
		uint32_t time_st;
#if 0
		ST_CRT	stCRT;
#endif
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
		uint32_t time_now;
		GetNowTime(&time_now);
#endif

#ifdef DEBUG
//		Draw_Box(	stCRT.hide_offset_x + X_POS_MIN - 1,
//					stCRT.hide_offset_y + Y_POS_BD + 1,
//					stCRT.hide_offset_x + X_POS_MAX - 1,
//					stCRT.hide_offset_y + g_TB_GameLevel[g_nGameLevel] + 1,
//					G_PURPLE, 0xFFFF);
#endif
		PCG_START_SYNC(g_Vwait);	/* �����J�n */

		/* �I������ */
		if(loop == 0)
		{
			break;
		}
		
		/* �����ݒ� */
		GetNowTime(&time_st);	/* ���C�������̊J�n�������擾 */
		SetStartTime(time_st);	/* ���C�������̊J�n�������L�� */
		
		/* �^�X�N���� */
		TaskManage();				/* �^�X�N���Ǘ����� */
		GetTaskInfo(&stTask);		/* �^�X�N�̏��𓾂� */

		/* ���[�h�����n�� */
		GetGameMode(&bMode);
		GetCRT(&stCRT, bMode);	/* ��ʍ��W�擾 */

#if CNF_VDISP
		/* V-Sync�œ��� */
#else
		if(Input_Main(g_ubDemoPlay) != 0u) 	/* ���͏��� */
		{
		}
#endif

#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
//		DirectInputKeyNum(&g_uDebugNum, 3);	/* �L�[�{�[�h���琔������� */
#endif

		if((GetInput() & KEY_b_ESC ) != 0u)	/* �d�r�b�L�[ */
		{
			/* �I�� */
			pushCount = Minc(pushCount, 1);
			if(pushCount >= 5)
			{
				SetTaskInfo(SCENE_EXIT);	/* �I���V�[���֐ݒ� */
			}
		}
		else if((GetInput() & KEY_b_HELP ) != 0u)	/* HELP�L�[ */
		{
			if( (stTask.bScene != SCENE_GAME_OVER_S) && (stTask.bScene != SCENE_GAME_OVER) )
			{
				/* ���Z�b�g */
				pushCount = Minc(pushCount, 1);
				if(pushCount >= 6)
				{

				}
				else if(pushCount >= 5)
				{
					SetTaskInfo(SCENE_INIT);	/* �I���V�[���֐ݒ� */
				}
			}
		}		
		else if((GetInput() & KEY_b_Q ) != 0u)	/* Q�L�[ */
		{
			if( (stTask.bScene != SCENE_GAME_OVER_S) && (stTask.bScene != SCENE_GAME_OVER) )
			{
				/* ���Z�b�g */
				pushCount = Minc(pushCount, 1);
				if(pushCount >= 6)
				{

				}
				else if(pushCount >= 5)
				{
					SetTaskInfo(SCENE_EXIT);	/* �I���V�[���֐ݒ� */
				}
			}
		}		
		else
		{
			pushCount = 0;
		}

		switch(stTask.bScene)
		{
			case SCENE_INIT:	/* �������V�[�� */
			{
				T_Clear();		/* �e�L�X�g�N���A */

				g_STG68K_mode = STG68K_SHIP;
				g_STG68K_stage = 0;	
				g_STG68K_PAL_num = 0;
				g_STG68K_View = 0;
				g_STG68K_save = 0;

				G_SP_to_GR_Load( g_pSP_Buf, g_nFileSize[STG_SM_SP], (int16_t *)((uint32_t)g_pPAL_Buf + (g_STG68K_PAL_num * sizeof(int16_t) * 16)), g_nFileSize[STG_SM_PAL], 0, 0, GR_Screen0);

				G_STG68K_to_GR_Load((int8_t *)((int32_t)(g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PTN], 
									(int16_t *)((int32_t)(g_pSTG68_COL_Buf)), g_nFileSize[STG_68_COL], 
									(int8_t *)((int32_t)(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PAL], 
									256, 0, GR_Screen0, g_STG68K_mode );

				SetTaskInfo(SCENE_TITLE_S);	/* �V�[��(�J�n����)�֐ݒ� */
				break;
			}
			/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
			case SCENE_TITLE_S:
			{
				G_PaletteSetZero();

				APL_Menu_Init();	/* ���j���[������ */
			    APL_Menu_On();	/* ���j���[�\�� */

				SetTaskInfo(SCENE_TITLE);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			case SCENE_TITLE:
			{
				int16_t sw = 0;

				sw = APL_Menu_Proc();	/* ���j���[ */
				switch(sw)
				{
					case 0:	/* �ҏW�Ώۂ̕ύX */
					{
						SetTaskInfo(SCENE_TITLE_E);	/* �V�[���ݒ� */
						break;
					}
					case 1:	/* �p���b�g�ԍ��̕ύX */
					{
						g_STG68K_View = 0;
						SetTaskInfo(SCENE_TITLE_E);	/* �V�[���ݒ� */
						break;
					}
					case 2:	/* �I�� */
					{
						g_STG68K_View = 0;
						SetTaskInfo(SCENE_GAME1_S);	/* �V�[���ݒ� */
						break;
					}
					case 3:	/* �I��(�ۑ�����) */
					{
						g_STG68K_View = 1;
						SetTaskInfo(SCENE_GAME_OVER_S);	/* �V�[���ݒ� */
						break;
					}
					case 4:	/* �I��(�ۑ��Ȃ�) */
					{
						g_STG68K_View = 1;
						SetTaskInfo(SCENE_GAME_OVER_E);	/* �V�[���ݒ� */
						break;
					}
					default:
					{
						/* �������Ȃ� */
						break;
					}
				}
				break;
			}
			case SCENE_TITLE_E:
			{
				SetTaskInfo(SCENE_START_S);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
			case SCENE_START_S:
			{
				APL_Menu_Off();	/* ���j���[������ */
				G_PaletteSetZero();

				/* ���͕\�� */
				{
//					puts("G_SP_to_GR_Load");
					G_SP_to_GR_Load( g_pSP_Buf, g_nFileSize[STG_SM_SP], (int16_t *)((uint32_t)g_pPAL_Buf + (g_STG68K_PAL_num * sizeof(int16_t) * 16)), g_nFileSize[STG_SM_PAL], 0, 0, GR_Screen0);
				}
				/* �o�͕\�� */
				{
					G_STG68K_to_GR_Load((int8_t *)((int32_t)(g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PTN], 
										(int16_t *)((int32_t)(g_pSTG68_COL_Buf)), g_nFileSize[STG_68_COL], 
										(int8_t *)((int32_t)(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PAL], 
										256, 0, GR_Screen0, g_STG68K_mode );
				}

				SetTaskInfo(SCENE_START);	/* �V�[���ݒ� */
				break;
			}
			case SCENE_START:
			{
				SetTaskInfo(SCENE_START_E);	/* �V�[���ݒ� */
				break;
			}
			case SCENE_START_E:
			{
				SetTaskInfo(SCENE_TITLE_S);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
			case SCENE_GAME1_S:
			{
				APL_Menu_Off();	/* ���j���[������ */
				bViewMode = VIEW_SP;

				G_SP_to_GR_Load( g_pSP_Buf, g_nFileSize[STG_SM_SP], (int16_t *)((uint32_t)g_pPAL_Buf + (g_STG68K_PAL_num * sizeof(int16_t) * 16)), g_nFileSize[STG_SM_PAL], 0, 0, GR_Screen0);
				
				T_Box(g_nTboxCursorPosX_O[VIEW_SP] + g_nTboxCursorPosX_ofst[VIEW_SP], g_nTboxCursorPosY_O[VIEW_SP], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0xFFFF, T_BLACK1);
				T_Box(g_nTboxCursorPosX[VIEW_SP] + g_nTboxCursorPosX_ofst[VIEW_SP], g_nTboxCursorPosY[VIEW_SP], g_nTboxCursorWidth, g_nTboxCursorHeight, 0xFFFF, T_YELLOW);
				
				SetTaskInfo(SCENE_GAME1);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			case SCENE_GAME2_S:
			{
				APL_Menu_Off();	/* ���j���[������ */
				bViewMode = VIEW_STG68K;

				G_STG68K_to_GR_Load((int8_t *)((int32_t)(g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PTN], 
									(int16_t *)((int32_t)(g_pSTG68_COL_Buf)), g_nFileSize[STG_68_COL], 
									(int8_t *)((int32_t)(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PAL], 
									256, 0, GR_Screen0, g_STG68K_mode );
				
				T_Box(g_nTboxCursorPosX_O[VIEW_STG68K] + g_nTboxCursorPosX_ofst[VIEW_STG68K], g_nTboxCursorPosY_O[VIEW_STG68K], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0xFFFF, T_BLACK1);
				T_Box(g_nTboxCursorPosX[VIEW_STG68K] + g_nTboxCursorPosX_ofst[VIEW_STG68K], g_nTboxCursorPosY[VIEW_STG68K], g_nTboxCursorWidth, g_nTboxCursorHeight, 0xFFFF, T_YELLOW);
				
				SetTaskInfo(SCENE_GAME2);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			case SCENE_GAME1:
			case SCENE_GAME2:
			{
				static int8_t s_b_UP;
				static int8_t s_b_DOWN;
				static int8_t s_b_RIGHT;
				static int8_t s_b_LEFT;
				static int8_t s_b_A;
				static int8_t s_b_B;
				static int8_t s_b_Updata;

				int8_t ms_x;
				int8_t ms_y;
				int8_t ms_left;
				int8_t ms_right;
				int32_t ms_pos_x;
				int32_t ms_pos_y;

				/* �}�E�X���� */
				Mouse_GetDataPos(&ms_x, &ms_y, &ms_left, &ms_right);

				Mouse_GetPos(&ms_pos_x, &ms_pos_y);

				if(ms_left != 0)
				{
					SetInput(KEY_b_Z);
				}
				if(ms_right != 0)
				{
					SetInput(KEY_b_X);
				}

				/* �W���C�X�e�B�b�N���� */

				/* ���j���[�ԍ��̎��s */
				if(	((GetInput_P1() & JOY_A ) != 0u)	||		/* A */
					((GetInput() & KEY_b_Z) != 0u)		||		/* A(z) */
					((GetInput() & KEY_b_SP ) != 0u)		)	/* �X�y�[�X�L�[ */
				{
					if(s_b_A == FALSE)
					{
						s_b_A = TRUE;
					}
					else
					{
					}
				}
				else
				{
					if(s_b_A == TRUE)
					{
						if(stTask.bScene == SCENE_GAME1)SetTaskInfo(SCENE_GAME1_E);	/* �V�[��(����)�֐ݒ� */
						if(stTask.bScene == SCENE_GAME2)SetTaskInfo(SCENE_GAME2_E);	/* �V�[��(����)�֐ݒ� */
					}
					s_b_A = FALSE;
				}

				/* �L�����Z�� */
				if(	((GetInput_P1() & JOY_B ) != 0u)	||		/* B */
					((GetInput() & KEY_b_X) != 0u)	)			/* B(z) */
				{
					if(s_b_B == FALSE)
					{
						s_b_B = TRUE;
						
						if(s_b_A == TRUE)
						{
							if(stTask.bScene == SCENE_GAME1)SetTaskInfo(SCENE_GAME1_E);	/* �V�[��(����)�֐ݒ� */
							if(stTask.bScene == SCENE_GAME2)SetTaskInfo(SCENE_GAME2_E);	/* �V�[��(����)�֐ݒ� */
						}
						else
						{
							if(g_STG68K_mode == STG68K_SHIP)
							{
								nTboxTableCount = 0;
							}
							else
							{
								nTboxTableCount++;
								if(nTboxTableCount >= 4)
								{
									nTboxTableCount = 0;
								}
							}
							g_nTboxCursorWidth  = nTboxTable_X[nTboxTableCount];
							g_nTboxCursorHeight = nTboxTable_Y[nTboxTableCount];
						}
					}
				}
				else
				{
					s_b_B = FALSE;
				}

				/* ���j���[�ԍ��̎��s�i�e���j���[�̔ԍ��ύX�����s�j */
				if(	((GetInput_P1() & JOY_LEFT ) != 0u )	||	/* LEFT */
					((GetInput() & KEY_LEFT) != 0 )		)	/* �� */
				{
					if(s_b_LEFT == FALSE)
					{
						s_b_LEFT = TRUE;

						g_nTboxCursorPosX[bViewMode] -= 16;
						if(g_nTboxCursorPosX[bViewMode] <= 0)
						{
							g_nTboxCursorPosX[bViewMode] = 0;
						}
					}
				}
				else if( ((GetInput_P1() & JOY_RIGHT ) != 0u )	||	/* RIGHT */
						((GetInput() & KEY_RIGHT) != 0 )			)	/* �E */
				{
					if(s_b_RIGHT == FALSE)
					{
						s_b_RIGHT = TRUE;

						g_nTboxCursorPosX[bViewMode] += 16;
						if(g_nTboxCursorPosX[bViewMode] >= g_nTboxCursorPosX[bViewMode] + g_nTboxCursorWidth)
						{
							g_nTboxCursorPosX[bViewMode] = g_nTboxCursorPosX[bViewMode] + g_nTboxCursorWidth;
						}
					}
				}
				else
				{
					s_b_LEFT = FALSE;
					s_b_RIGHT = FALSE;
				}

				/* ���j���[�ԍ��ύX */
				if(	((GetInput_P1() & JOY_UP ) != 0u )	||	/* UP */
					((GetInput() & KEY_UPPER) != 0 )	)	/* �� */
				{
					if(s_b_UP == FALSE)
					{
						s_b_UP = TRUE;

						g_nTboxCursorPosY[bViewMode] -= 16;
						if(g_nTboxCursorPosY[bViewMode] <= 0)
						{
							g_nTboxCursorPosY[bViewMode] = 0;
						}
					}
				}
				else if(((GetInput_P1() & JOY_DOWN ) != 0u )	||	/* UP */
						((GetInput() & KEY_LOWER) != 0 )	)	    /* �� */
				{
					if(s_b_DOWN == FALSE)
					{
						s_b_DOWN = TRUE;

						g_nTboxCursorPosY[bViewMode] += 16;
						if(g_nTboxCursorPosY[bViewMode] >= g_nTboxCursorPosY[bViewMode] + g_nTboxCursorHeight)
						{
							g_nTboxCursorPosY[bViewMode] = g_nTboxCursorPosY[bViewMode] + g_nTboxCursorHeight;
						}
					}
				}
				else
				{
					s_b_UP = FALSE;
					s_b_DOWN = FALSE;

					if((ms_x != 0) || (ms_y != 0))
					{
//						T_Box(g_nTboxCursorPosX_O[bViewMode], g_nTboxCursorPosY_O[bViewMode], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0x5A5A, T_BLACK1);
//						T_Box(g_nTboxCursorPosX[bViewMode], g_nTboxCursorPosY[bViewMode], g_nTboxCursorWidth, g_nTboxCursorHeight, 0x5A5A, T_YELLOW);
					}
				}
				if( (s_b_UP == TRUE)    ||
					(s_b_DOWN == TRUE)  ||
					(s_b_RIGHT == TRUE) ||
					(s_b_LEFT == TRUE)  ||
					(s_b_A == TRUE)     ||
					(s_b_B == TRUE)     )
				{
					s_b_Updata = TRUE;
				}

				if( s_b_Updata == TRUE )
				{
					T_Box(g_nTboxCursorPosX_O[bViewMode] + g_nTboxCursorPosX_ofst[bViewMode], g_nTboxCursorPosY_O[bViewMode], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0xFFFF, T_BLACK1);
					T_Box(g_nTboxCursorPosX[bViewMode] + g_nTboxCursorPosX_ofst[bViewMode], g_nTboxCursorPosY[bViewMode], g_nTboxCursorWidth, g_nTboxCursorHeight, 0xFFFF, T_YELLOW);
					g_nTboxCursorPosX_O[bViewMode] = g_nTboxCursorPosX[bViewMode];
					g_nTboxCursorPosY_O[bViewMode] = g_nTboxCursorPosY[bViewMode];
					g_nTboxCursorWidth_O = g_nTboxCursorWidth;
					g_nTboxCursorHeight_O = g_nTboxCursorHeight;
					s_b_Updata = FALSE;
				}
				break;
			}
			case SCENE_GAME1_E:
			{
				T_Box(g_nTboxCursorPosX_O[VIEW_SP] + g_nTboxCursorPosX_ofst[VIEW_SP], g_nTboxCursorPosY_O[VIEW_SP], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0xFFFF, T_BLACK1);

				g_STG68K_View = 1;

				SetTaskInfo(SCENE_GAME2_S);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			case SCENE_GAME2_E:
			{
				T_Box(g_nTboxCursorPosX_O[VIEW_STG68K] + g_nTboxCursorPosX_ofst[VIEW_STG68K], g_nTboxCursorPosY_O[VIEW_STG68K], g_nTboxCursorWidth_O, g_nTboxCursorHeight_O, 0xFFFF, T_BLACK1);

				STG68K_SPPAL_to_PTNCOLPAL_Cnv(g_STG68K_mode);

				g_STG68K_View = 1;

				SetTaskInfo(SCENE_START_S);	/* �V�[��(����)�֐ݒ� */
				break;
			}
			/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
			case SCENE_GAME_OVER_S:	
			{
				APL_Menu_Exit();	/* ���j���[������ */

				G_PaletteSetZero();

				G_STG68K_to_GR_Load((int8_t *)((int32_t)(g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PTN], 
									(int16_t *)((int32_t)(g_pSTG68_COL_Buf)), g_nFileSize[STG_68_COL], 
									(int8_t *)((int32_t)(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[g_STG68K_mode])), g_nFileSize[STG_68_PAL], 
									256, 0, GR_Screen0, g_STG68K_mode );

				SetTaskInfo(SCENE_GAME_OVER);	/* �V�[���ݒ� */
				break;
			}
			case SCENE_GAME_OVER:
			{
				File_Save_OverWrite(g_sFileName[STG_68_PTN], g_pSTG68_PTN_Buf, sizeof(int8_t), g_nFileSize[STG_68_PTN] );

				File_Save_OverWrite(g_sFileName[STG_68_PAL], g_pSTG68_PAL_Buf, sizeof(int8_t), g_nFileSize[STG_68_PAL] );

				File_Save_OverWrite(g_sFileName[STG_68_COL], g_pSTG68_COL_Buf, sizeof(int8_t), g_nFileSize[STG_68_COL] );

				SetTaskInfo(SCENE_GAME_OVER_E);	/* �V�[���ݒ� */
				break;
			}
			case SCENE_GAME_OVER_E:
			{
				SetTaskInfo(SCENE_EXIT);	/* �I���V�[���֐ݒ� */
				break;
			}
			/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
			default:	/* �ُ�V�[�� */
			{
				loop = 0;	/* ���[�v�I�� */
				break;
			}
		}
		SetFlip(TRUE);	/* �t���b�v���� */

		PCG_END_SYNC(g_Vwait);	/* �X�v���C�g�̏o�� */

		uFreeRunCount++;	/* 16bit �t���[�����J�E���^�X�V */
		g_unFrameCount++;	/* 16bit �t���[�����J�E���^�X�V */

#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
		/* �������Ԍv�� */
		GetNowTime(&time_now);
		unTime_cal = time_now - time_st;	/* LSB:1 UNIT:ms */
		g_unTime_cal = unTime_cal;
		if( stTask.bScene == SCENE_GAME1 )
		{
			unTime_cal_PH = Mmax(unTime_cal, unTime_cal_PH);
			g_unTime_cal_PH = unTime_cal_PH;
		}
#endif

#ifdef MEM_MONI	/* �f�o�b�O�R�[�i�[ */
		GetFreeMem();	/* �󂫃������T�C�Y�m�F */
#endif

	}
	while( loop );

	g_bExit = FALSE;

	return ret;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
static void App_Init(void)
{
	uint32_t nNum;
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init �J�n");
#endif
#ifdef MEM_MONI	/* �f�o�b�O�R�[�i�[ */
	g_nMaxUseMemSize = GetFreeMem();
	printf("FreeMem(%d[kb])\n", g_nMaxUseMemSize);	/* �󂫃������T�C�Y�m�F */
	puts("App_Init ������");
#endif

	Set_SupervisorMode();	/* �X�[�p�[�o�C�U�[���[�h */
	/* MFP */
	MFP_INIT();	/* V-Sync���荞�ݓ��̏��������� */
	Set_UserMode();			/* ���[�U�[���[�h */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	printf("App_Init MFP(%d)\n", Get_CPU_Time());
#endif
	if(SetNowTime(0) == FALSE)
	{
		/*  */
	}
	nNum = Get_CPU_Time();	/* 300 = 10MHz� */
	if(nNum  < 400)
	{
		g_GameSpeed = 0;
		printf("Normal Speed(%d)\n", g_GameSpeed);
	}
	else if(nNum < 640)
	{
		g_GameSpeed = 2;
		printf("XVI Speed(%d)\n", g_GameSpeed);
	}
	else if(nNum < 800)
	{
		g_GameSpeed = 4;
		printf("RedZone Speed(%d)\n", g_GameSpeed);
	}
	else if(nNum < 2000)
	{
		g_GameSpeed = 7;
		printf("030 Speed(%d)\n", g_GameSpeed);
	}
	else
	{
		g_GameSpeed = 10;
		printf("Another Speed(%d)\n", g_GameSpeed);
	}
	puts("App_Init Input");
	Input_Init();			/* ���͏����� */
	/* �X�[�p�[�o�C�U�[���[�h�J�n */
	Set_SupervisorMode();


	/* Task */
	TaskManage_Init();
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init Task");
#endif
	
	/* �}�E�X������ */
	Mouse_Init();	/* �}�E�X������ */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init �}�E�X");
#endif
	/* ��� */
//	g_nCrtmod = CRTC_INIT(10);	/* mode=10 256x256 col:16/256 31kHz */
	g_nCrtmod = CRTC_INIT(12);	/* mode=14 512x512 col:65536 31kHz */
//	g_nCrtmod = CRTC_INIT(14);	/* mode=14 256x256 col:65536 31kHz */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init CRTC(���)");
//	KeyHitESC();	/* �f�o�b�O�p */
#endif

	/* �O���t�B�b�N */
	G_INIT();			/* ��ʂ̏����ݒ� */
	G_CLR();			/* �N���b�s���O�G���A�S�J������ */
	G_HOME(0);			/* �z�[���ʒu�ݒ� */
	G_VIDEO_INIT();		/* �r�f�I�R���g���[���[�̏����� */
	G_PaletteSetZero();
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init �O���t�B�b�N");
#endif

	/* �e�L�X�g */
	T_INIT();	/* �e�L�X�g�u�q�`�l������ */
	T_PALET();	/* �e�L�X�g�p���b�g������ */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init T_INIT");
#endif
	g_Vwait = 1;

#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_Init �I��");
#endif
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
static void App_exit(void)
{
	int16_t	ret = 0;

#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit �J�n");
#endif
	
	if(g_bExit == TRUE)
	{
		puts("�G���[���L���b�`�I ESC to skip");
		KeyHitESC();	/* �f�o�b�O�p */
	}
	g_bExit = TRUE;

	/* �O���t�B�b�N */
	G_CLR();			/* �N���b�s���O�G���A�S�J������ */


	/* MFP */
	ret = MFP_EXIT();				/* MFP�֘A�̉��� */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	printf("App_exit MFP(%d)\n", ret);
#endif

	/* ��� */
	CRTC_EXIT(0x100 + g_nCrtmod);	/* ���[�h�����Ƃɖ߂� */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit ���");
#endif

	Mouse_Exit();	/* �}�E�X��\�� */

	/* �e�L�X�g */
	T_EXIT();				/* �e�L�X�g�I������ */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit �e�L�X�g");
#endif

	MyMfree(0);				/* �S�Ẵ���������� */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	printf("MaxUseMem(%d[kb])\n", g_nMaxUseMemSize - GetMaxFreeMem());
	puts("App_exit ������");
#endif

	_dos_kflushio(0xFF);	/* �L�[�o�b�t�@���N���A */
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit �L�[�o�b�t�@�N���A");
#endif

	/*�X�[�p�[�o�C�U�[���[�h�I��*/
	Set_UserMode();
#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit �X�[�p�[�o�C�U�[���[�h�I��");
#endif

#ifdef DEBUG	/* �f�o�b�O�R�[�i�[ */
	puts("App_exit �I��");
#endif
}


/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t	App_FlipProc(void)
{
	int16_t	ret = 0;
	
#ifdef FPS_MONI	/* �f�o�b�O�R�[�i�[ */
	uint32_t time_now;
	static uint8_t	bFPS = 0u;
	static uint32_t	unTime_FPS = 0u;
#endif

	if(g_bFlip == FALSE)	/* �`�撆�Ȃ̂Ńt���b�v���Ȃ� */
	{
		return ret;
	}
	else
	{
#ifdef W_BUFFER
		ST_CRT		stCRT;
		GetCRT(&stCRT, g_mode);	/* ��ʍ��W�擾 */
#endif
		SetFlip(FALSE);			/* �t���b�v�֎~ */
					
#ifdef W_BUFFER
		/* ���[�h�`�F���W */
		if(g_mode == 1u)		/* �㑤���� */
		{
			SetGameMode(2);
		}
		else if(g_mode == 2u)	/* �������� */
		{
			SetGameMode(1);
		}
		else					/* ���̑� */
		{
			SetGameMode(0);
		}

		/* ��\����ʂ�\����ʂ֐؂�ւ� */
		G_HOME(g_mode);

		/* �����N���A */
		G_CLR_AREA(	stCRT.hide_offset_x, WIDTH,
					stCRT.hide_offset_y, HEIGHT, 0);
#endif
		
#ifdef FPS_MONI	/* �f�o�b�O�R�[�i�[ */
		bFPS++;
#endif
		ret = 1;
	}

#ifdef FPS_MONI	/* �f�o�b�O�R�[�i�[ */
	GetNowTime(&time_now);
	if( (time_now - unTime_FPS) >= 1000ul )
	{
		g_bFPS_PH = bFPS;
		unTime_FPS = time_now;
		bFPS = 0;
	}
#endif

	return ret;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t	SetFlip(uint8_t bFlag)
{
	int16_t	ret = 0;

	Set_DI();	/* ���荞�݋֎~ */
	g_bFlip_old = g_bFlip;	/* �t���b�v�O��l�X�V */
	g_bFlip = bFlag;
	Set_EI();	/* ���荞�݋��� */

	return ret;
}
/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
void App_TimerProc( void )
{
	ST_TASK stTask;

	TaskManage();
	GetTaskInfo(&stTask);	/* �^�X�N�擾 */

	/* ������ ���̊Ԃɏ��������� ������ */
	if(stTask.b96ms == TRUE)	/* 96ms���� */
	{
		if(GetJoyAnalogMode() == TRUE)	/* �A�i���O���� */
		{
			if(Input_Main(g_ubDemoPlay) != 0u) 	/* ���͏��� */
			{
				g_ubDemoPlay = FALSE;	/* �f������ */

				SetTaskInfo(SCENE_INIT);	/* �V�[��(����������)�֐ݒ� */
			}
		}
		else	/* �f�W�^������ */
		{
		}
#if 0
		switch(stTask.bScene)
		{
			case SCENE_GAME1:
			case SCENE_GAME2:
			{
#ifdef FPS_MONI	/* �f�o�b�O�R�[�i�[ */
//				int8_t	sBuf[8];
#endif
#ifdef FPS_MONI	/* �f�o�b�O�R�[�i�[ */
				memset(sBuf, 0, sizeof(sBuf));
				sprintf(sBuf, "%3d", g_bFPS_PH);
				Draw_Message_To_Graphic(sBuf, 24, 24, F_MOJI, F_MOJI_BAK);	/* �f�o�b�O */
#endif
				break;
			}
			default:
				break;
		}
#endif
	}
	/* ������ ���̊Ԃɏ��������� ������ */

	/* �^�X�N���� */
	UpdateTaskInfo();		/* �^�X�N�̏����X�V */

}
/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t App_RasterProc( uint16_t *pRaster_cnt )
{
	int16_t	ret = 0;
#if CNF_RASTER
	RasterProc(pRaster_cnt);	/* ���X�^�[���荞�ݏ��� */
#endif /* CNF_RASTER */
	return ret;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
void App_VsyncProc( void )
{
#if CNF_VDISP
	ST_TASK stTask;
	int8_t x;
	int8_t y;
	int8_t left;
	int8_t right;

//	puts("App_VsyncProc");
	Timer_D_Less_NowTime();

	GetTaskInfo(&stTask);	/* �^�X�N�擾 */
	/* ������ ���̊Ԃɏ��������� ������ */
	if(GetJoyAnalogMode() == TRUE)	/* �A�i���O���� */
	{
	}
	else	/* �f�W�^������ */
	{
		if(Input_Main(g_ubDemoPlay) != 0u) 	/* ���͏��� */
		{
			g_ubDemoPlay = FALSE;	/* �f������ */

			SetTaskInfo(SCENE_INIT);	/* �V�[��(����������)�֐ݒ� */
		}
	}

	switch(stTask.bScene)
	{
		case SCENE_TITLE:
		case SCENE_GAME_OVER:
		{
			Mouse_GetDataPos(&x, &y, &left, &right);

			if(x == 0)
			{

			}
			else if(x > 0)
			{
				SetInput(KEY_RIGHT);
			}
			else{
				SetInput(KEY_LEFT);
			}

			if(y == 0)
			{

			}
			else if(y > 0)
			{
				SetInput(KEY_LOWER);
			}
			else{
				SetInput(KEY_UPPER);
			}

			if(left != 0)
			{
				SetInput(KEY_b_Z);
			}

			if(right != 0)
			{
				SetInput(KEY_b_X);
			}
			break;
		}
		default:	/* �ُ�V�[�� */
		{
			break;
		}
	}

	/* ������ ���̊Ԃɏ��������� ������ */
	
	App_FlipProc();	/* ��ʐ؂�ւ� */

#endif /* CNF_VDISP */
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t	GetGameMode(uint8_t *bMode)
{
	int16_t	ret = 0;
	
	*bMode = g_mode;
	
	return ret;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t	SetGameMode(uint8_t bMode)
{
	int16_t	ret = 0;
	
	g_mode = bMode;
	if(bMode == 1)
	{
		g_mode_rev = 2;
	}
	else
	{
		g_mode_rev = 1;
	}
	
	return ret;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t SerchNumber(int8_t *pData, int32_t data_size, int8_t min, int8_t max)
{
    // �͈͓��̎g�p�󋵂��L�^����t���O
    int8_t *pFlag;
	int32_t i;
	int16_t range = max - min + 1;

	pFlag = MyMalloc(range *  sizeof(int8_t));
    for ( i = 0; i < range; i++) {
        pFlag[i] = FALSE;
    }
    // �g�p����Ă��鐔�l�Ƀt���O�𗧂Ă�
    for ( i = 0; i < data_size; i++) {
        if ((pData[i] >= min) && (pData[i] <= max)) {
 			pFlag[pData[i] - min] = TRUE;  // �C���f�b�N�X�␳
        }
    }

    // �g�p����Ă��Ȃ����l���o��
    for ( i = 0; i < range; i++) {
        if(pFlag[i] == 0){
			MyMfree(pFlag);
            return (i + min);
        }
    }
	MyMfree(pFlag);
    return -1;
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t STG68K_SPPAL_to_PTNCOLPAL_Cnv(int16_t nMode)
{
	int16_t ret = 0;
	int16_t x, y, z, u;
	int16_t dx, dy;
	int16_t src_pal, dst_pal;
	int8_t *pSrc, *pDst, *pDst2;
	int16_t *pSrc16, *pDst16;
	int16_t count_x, count_y;
	int16_t pal_min, pal_max;
	int8_t bMuch = FALSE;

	if(nMode >= STG68K_MAX)
	{
		return ret -1;
	}

	switch(nMode)
	{
		case STG68K_SHIP:
		case STG68K_ENEMTY:
		case STG68K_BOSS:
		{
			pal_min = 17;
			pal_max = 31;
			break;
		}
		case STG68K_SCORE:
		{
			pal_min = 16;
			pal_max = 16;
			break;
		}
		case STG68K_BG:
		{
			pal_min = 0;
			pal_max = 15;
			break;
		}
		default:
		{
			pal_min = 0;
			pal_max = 31;
			break;
		}
	}

	count_x = g_nTboxCursorWidth / 16;
	count_y = g_nTboxCursorHeight / 16;

	for(dy = 0; dy < count_y; dy++)	/* �cSP */
	{
		for(dx = 0; dx < count_x; dx++)	/* ��SP  */
		{
			/*===<PTN>===============================================================================================================*/

			pSrc = g_pSP_Buf + (dy * 0x800) + (dx * 0x80) + ((g_nTboxCursorPosY[VIEW_SP] / 16) * 0x800) + ((g_nTboxCursorPosX[VIEW_SP] / 16) * 0x80);

			for(z = 0; z < 4; z++)	/* 8x8��4�Z�b�g */
			{
				for(y = 0; y < 4; y++)	/* �c8�h�b�g�� */
				{
					int32_t nOffsetXY[] = {0x00, 0x40, 0x04, 0x44};

					pDst = g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[nMode] + ((g_nTboxCursorPosY[VIEW_STG68K] / 16) * 0x800) + ((g_nTboxCursorPosX[VIEW_STG68K] / 16) * 0x80) + (dy * 0x800) + (dx * 0x80) + nOffsetXY[z] + (y * 0x10);
					pDst2= g_pSTG68_PTN_Buf + g_nSTG68_PTN_MAP[nMode] + ((g_nTboxCursorPosY[VIEW_STG68K] / 16) * 0x800) + ((g_nTboxCursorPosX[VIEW_STG68K] / 16) * 0x80) + (dy * 0x800) + (dx * 0x80) + nOffsetXY[z] + (y * 0x10) + 8;

					for(x = 0; x < 4; x++)	/* ��4�h�b�g��1 */
					{
						*pDst = swap_nibbles(*pSrc);
						pDst++;
						pSrc++;
					}
					for(x = 0; x < 4; x++)	/* ��4�h�b�g��2 */
					{
						*pDst2 = swap_nibbles(*pSrc);
						pDst2++;
						pSrc++;
					}
				}
			}
		}
	}

	/*===<PAL>===============================================================================================================*/
	dst_pal = SerchNumber(g_pSTG68_PAL_Buf, g_nFileSize[STG_68_PAL], pal_min, pal_max);	/* �󂫃p���b�g�T�� */
	for(z = pal_min; z <= pal_max; z++)	/* �T�� */
	{
		bMuch = TRUE;
		pSrc16 = (int16_t *)((uint32_t)g_pPAL_Buf + (g_STG68K_PAL_num * sizeof(int16_t) * 16));
		pDst16 = (int16_t *)((uint32_t)g_pSTG68_COL_Buf + (sizeof(int16_t) * 16 * z));
		for(u=0; u < 16; u++)
		{
			int16_t dst_col;
			// �\�[�X�̂Obit���珇�Ԃɂǂ��̃r�b�g�Ɉړ����������i0 �� , 1 �� , ..., 15 �� �j
			uint8_t reverse_order[16] = {
					9, 10,				/* R */
					11, 12, 13, 14, 15,	/* B */
					0,					/* I */
					1,  2,  3,  4,  5,	/* G */
					6,  7,  8			/* R */
			};

			dst_col = reorder_bits(*pSrc16, reverse_order);
//			printf("(%d,%d)*pDst16 0x%X dst_col 0x%X, *pSrc16 0x%X\n", z, u, (int16_t)*pDst16, dst_col, (int16_t)*pSrc16);
			if((int16_t)*pDst16 != dst_col)
			{
				bMuch = FALSE;
				break;
			}
			pSrc16++;
			pDst16++;
		}
		if(bMuch == TRUE)
		{
//			printf("much %d\n", z);
			dst_pal = z;
			break;
		}
	}

	for(dy = 0; dy < count_y; dy++)	/* �cSP */
	{
		for(dx = 0; dx < count_x; dx++)	/* ��SP  */
		{
			if(dst_pal < 0)
			{
				src_pal = *(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[nMode] + ((g_nTboxCursorPosY[VIEW_STG68K] / 16) * 0x10) + (g_nTboxCursorPosX[VIEW_STG68K] / 16) + (dy * 0x10) + count_x);
//				printf("src_pal %d dst_pal%d\n", src_pal, dst_pal);
				dst_pal = src_pal;
			}
			*(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[nMode] + ((g_nTboxCursorPosY[VIEW_STG68K] / 16) * 0x10) + (g_nTboxCursorPosX[VIEW_STG68K] / 16) + (dy * 0x10) + dx) = dst_pal;
//			printf("%d,", *(g_pSTG68_PAL_Buf + g_nSTG68_PAL_MAP[nMode] + (dy * 0x10) + dx));
		}
	}
	
	/*===<COL>===============================================================================================================*/
	pSrc16 = (int16_t *)((uint32_t)g_pPAL_Buf + (g_STG68K_PAL_num * sizeof(int16_t) * 16));
	pDst16 = (int16_t *)((uint32_t)g_pSTG68_COL_Buf + (sizeof(int16_t) * 16 * dst_pal));
	for(u=0; u < 16; u++)
	{
		// �\�[�X�̂Obit���珇�Ԃɂǂ��̃r�b�g�Ɉړ����������i0 �� , 1 �� , ..., 15 �� �j
		uint8_t reverse_order[16] = {
				9, 10,				/* R */
				11, 12, 13, 14, 15,	/* B */
				0,					/* I */
				1,  2,  3,  4,  5,	/* G */
				6,  7,  8			/* R */
		};
//							*pDst16 = swap_nibbles8(*pSrc16);
		*pDst16 = reorder_bits(*pSrc16, reverse_order);
		pSrc16++;
		pDst16++;
	}
	return	ret;
}
/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
void HelpMessage(void)
{
	printf("=<Help Message>=============================================================\n");
	printf("  STG68K_CNV.X -i <SP/PAL�t�@�C����> -o <�o��STAGE�ԍ�> -m <�����������>\n");
	printf("   ex. >STG68K_CNV.x -i hogehoge -o 2 -m 0\n");
	printf("        ���@�݂̂��f�[�^�R���o�[�g���܂�\n");
	printf("        hogehoge.sp hogehoge.pal =>  STAGE2.PTN STAGE2.COL STAGE2.PAL \n");
	printf("============================================================================\n");
	printf("<�����������(bit)>\n");
	printf("  bit0:���@�̂�\n");
	printf("============================================================================\n");
}

/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
int16_t main(int16_t argc, int8_t** argv)
{
	int16_t	ret = 0;

	/* COPY�L�[������ */
	init_trap12();
	/* ��O�n���h�����O���� */
	usr_abort = App_exit;	/* ��O�����ŏI�����������{���� */
	init_trap14();			/* �f�o�b�O�p�v���I�G���[�n���h�����O */
#if 0	/* �A�h���X�G���[���� */
	{
		char buf[10];
		int *A = (int *)&buf[1];
		int B = *A;
		return B;
	}
#endif

	printf("SHOOTING 68K�f�[�^�쐬�⏕�c�[���uSTG68K_CNV.X�vVer%d.%d.%d (c)2025 �J�^.\n", MAJOR_VER, MINOR_VER, PATCH_VER);

	App_Init();		/* ������ */

	if(argc > 1)	/* �I�v�V�����`�F�b�N */
	{
		int16_t i, j;
		
		for(i = 1; i < argc; i++)
		{
			int8_t	bOption = FALSE;
			int8_t	bFlag;
			
			bOption	= ((argv[i][0] == '-') || (argv[i][0] == '/')) ? TRUE : FALSE;

			if(bOption == TRUE)
			{
				/* �w���v */
				bFlag	= ((argv[i][1] == '?') || (argv[i][1] == 'h') || (argv[i][1] == 'H')) ? TRUE : FALSE;
				if(bFlag == TRUE)
				{
					HelpMessage();	/* �w���v */
					ret = -1;
					break;
				}

				/* ���̓t�@�C�� */
				bFlag	= ((argv[i][1] == 'i') || (argv[i][1] == 'I')) ? TRUE : FALSE;
				if(bFlag == TRUE)
				{
					for(j = 0; j < 2; j++)
					{
						int8_t sStr[128] = {0};
						int8_t sExt[2][8] = {".SP",".PAL"};

						strcpy(sStr, argv[i+1]);
						strcat(sStr, sExt[j]);
						ret = Get_FileAlive(sStr);
						if(ret < 0)
						{
							HelpMessage();	/* �w���v */
							ret = -1;
							break;
						}
						else
						{
							int32_t nLength = 0;
							GetFileLength(sStr, &nLength);

							switch(j)
							{
								case STG_SM_SP:
								{
									strcpy(g_sFileName[STG_SM_SP], sStr);
									g_nFileSize[STG_SM_SP] = nLength;
									g_pSP_Buf = MyMalloc(nLength);
									File_Load(sStr, g_pSP_Buf, sizeof(int8_t), nLength);
									break;
								}
								case STG_SM_PAL:
								{
									strcpy(g_sFileName[STG_SM_PAL], sStr);
									g_nFileSize[STG_SM_PAL] = nLength;
									g_pPAL_Buf = MyMalloc(nLength);
									File_Load(sStr, g_pPAL_Buf, sizeof(int8_t), nLength);
									break;
								}
							}
							printf("%s(%d)\n",g_sFileName[j], g_nFileSize[j]);
						}
					}

					continue;
				}
				
				/* �o�̓t�@�C�� */
				bFlag	= ((argv[i][1] == 'o') || (argv[i][1] == 'O')) ? TRUE : FALSE;
				if(bFlag == TRUE)
				{
					int16_t nStageNum;
					nStageNum = atoi(argv[i+1]);
					g_STG68K_stage = (uint8_t)nStageNum;

					for(j = 0; j < 3; j++)
					{
						int8_t sStr[128] = {0};
						int8_t sExt[3][8] = {".PTN",".COL",".PAL"};

						sprintf(sStr, "STAGE%d%s", nStageNum, sExt[j]);
						ret = Get_FileAlive(sStr);
						if(ret < 0)
						{
							HelpMessage();	/* �w���v */
							ret = -1;
							break;
						}
						else
						{
							int32_t nLength = 0;
							GetFileLength(sStr, &nLength);
							//printf("%s\n",sStr);

							switch(j)
							{
								case 0:
								{
									strcpy(g_sFileName[STG_68_PTN], sStr);
									g_nFileSize[STG_68_PTN] = nLength;
									g_pSTG68_PTN_Buf = MyMalloc(nLength);
									File_Load(sStr, g_pSTG68_PTN_Buf, sizeof(int8_t), nLength);
									break;
								}
								case 1:
								{
									strcpy(g_sFileName[STG_68_COL], sStr);
									g_nFileSize[STG_68_COL] = nLength;
									g_pSTG68_COL_Buf = MyMalloc(nLength);
									File_Load(sStr, g_pSTG68_COL_Buf, sizeof(int8_t), nLength);
									break;
								}
								case 2:
								{
									strcpy(g_sFileName[STG_68_PAL], sStr);
									g_nFileSize[STG_68_PAL] = nLength;
									g_pSTG68_PAL_Buf = MyMalloc(nLength);
									File_Load(sStr, g_pSTG68_PAL_Buf, sizeof(int8_t), nLength);
									break;
								}
							}
						}
						printf("%s(%d)\n",g_sFileName[STG_68_PTN+j], g_nFileSize[STG_68_PTN+j]);
					}
					continue;
				}

				/* ����������� */
				bFlag	= ((argv[i][1] == 'm') || (argv[i][1] == 'M')) ? TRUE : FALSE;
				if(bFlag == TRUE)
				{
					g_STG68K_mode = atoi(argv[i+1]);
					continue;
				}

				/* �w���v */
				if(bFlag == FALSE)
				{
					HelpMessage();	/* �w���v */
					ret = -1;
					break;
				}
			}
		}
	}
	else
	{
		HelpMessage();	/* �w���v */
		ret = -1;
	}

	if(ret == 0)
	{
		main_Task();	/* ���C������ */

		App_exit();		/* �I������ */
	}
	
	return ret;
}

#endif	/* MAIN_C */
