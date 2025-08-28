#ifndef	APL_PCG_C
#define	APL_PCG_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>

#include <usr_macro.h>
#include "BIOS_PCG.h"
#include "IF_PCG.h"
#include "IF_FileManager.h"
#include "IF_Math.h"
#include "APL_PCG.h"

/* define��` */

/* �O���[�o���ϐ� */

/* �֐��̃v���g�^�C�v�錾 */
void PCG_INIT_CHAR(void);

/* �֐� */
/*===========================================================================================*/
/* �֐���	�F	*/
/* ����		�F	*/
/* �߂�l	�F	*/
/*-------------------------------------------------------------------------------------------*/
/* �@�\		�F	*/
/*===========================================================================================*/
void PCG_INIT_CHAR(void)
{
	uint32_t	i, j;
	uint32_t	uPCG_list;
	uint32_t	uPCG_num;
	uint16_t	uPCG_SP_offset;
	uint16_t	uPCG_SP_next = 0x0;	/* 0=BG */
	uint8_t		ubOK;
	uint8_t		ubPri = 0x00;
	uint8_t		ubPal = 0x00;

#if  CNF_XSP
	uPCG_SP_offset = PCG_16x16_AREA;	/* PCG_16x16_AREA������BG */
#else
	uPCG_SP_offset = 0;	/* XSP�ȊO�͊֌W�Ȃ� */
#endif

	puts("SP PCG_INIT_CHAR");
	
	/* ���ԃX�v���C�g */
	for(uPCG_num = 0; uPCG_num < PCG_NUM_MAX; uPCG_num++)
	{
		ST_PCG stPCG;
		
		/* �����܂��������͕s�v */
		stPCG.x			= 0;	/* x���W */
		stPCG.y			= 0;	/* y���W */
		stPCG.dx		= 0;	/* �ړ���x */
		stPCG.dy		= 0;	/* �ړ���y */
		stPCG.Anime		= 0;	/* ���݂̃A�j�� */
		stPCG.Anime_old	= 0;	/* �O��̃A�j�� */
		
		switch(uPCG_num)
		{
#if 0
			case	BG_DATA:
			{
				uPCG_list = 0;
				uPCG_SP_next = 0;
				ubOK = FALSE;
				break;
			}
#endif
			case	SP_CURSOR1:
			case	SP_CURSOR2:
			case	SP_CURSOR3:
			case	SP_CURSOR4:
			{
				uPCG_list = 1;
				stPCG.Pat_w			= g_stST_PCG_LIST[uPCG_list].Pat_w = 1;
				stPCG.Pat_h			= g_stST_PCG_LIST[uPCG_list].Pat_h = 1;
				stPCG.Pat_AnimeMax	= g_stST_PCG_LIST[uPCG_list].Pat_AnimeMax = 1;;
				stPCG.Pat_DataMax	= stPCG.Pat_w * stPCG.Pat_h * stPCG.Pat_AnimeMax;
				ubPal 				= g_stST_PCG_LIST[uPCG_list].Pal;				/* �p���b�g�ԍ� */
#if  CNF_XSP
				PCG_Load_Data(NULL, 0x00, stPCG, uPCG_num, 3);
				g_stPCG_DATA[uPCG_num].Plane	= uPCG_SP_offset + uPCG_SP_next;	/* �X�v���C�gNo. */
				ubPri = 0x1F;														/* �v���C�I���e�B */
				if(uPCG_num == SP_CURSOR2)	/* �Ō�͎��֐\������ */
				{
					uPCG_SP_next += stPCG.Pat_DataMax;
				}
#else
				PCG_Load_Data(g_sp_list[uPCG_list], 0x00, stPCG, uPCG_num, 0);
				ubPri = 0xFF;
#endif
				ubOK = TRUE;
				break;
			}
			default:
			{

				ubOK = FALSE;
				break;
			}
		}
		
		if(ubOK == TRUE)
		{
			/* �X�v���C�g��`�ݒ� */
			for(j=0; j < stPCG.Pat_DataMax; j++)
			{
#if  CNF_XSP
				*(g_stPCG_DATA[uPCG_num].pPatCodeTbl + j)	= SetXSPinfo(0, 0, ubPal, ubPri);	/* �p�^�[���R�[�h�e�[�u�� */
#else
				*(g_stPCG_DATA[uPCG_num].pPatCodeTbl + j)	= SetBGcode(0, 0, ubPal, 0xFF);	/* �p�^�[���R�[�h�e�[�u�� */
//				*(g_stPCG_DATA[uPCG_num].pPatCodeTbl + j)	= SetBGcode2(0, 0, ubPal);	/* �p�^�[���R�[�h�e�[�u�� */
#endif
			}
			g_stPCG_DATA[uPCG_num].update	= FALSE;
			g_stPCG_DATA[uPCG_num].validty	= TRUE;
		}
		else
		{
			g_stPCG_DATA[uPCG_num].Pat_w		= 0;
			g_stPCG_DATA[uPCG_num].Pat_h		= 0;
			g_stPCG_DATA[uPCG_num].update	= FALSE;
			g_stPCG_DATA[uPCG_num].validty	= FALSE;
		}
#if  CNF_XSP
		/* �Ȃ� */
#else
		g_stPCG_DATA[uPCG_num].Plane		= 0xFFFF;		/* �v���[��No. */
#endif
#ifdef DEBUG
//		printf("%d, %d, %d, %d, %d, %d\n", uPCG_num, stPCG.Pat_w, stPCG.Pat_h, stPCG.Pat_AnimeMax, stPCG.Pat_DataMax, ubPal);
#endif
#if 1
		if((uPCG_num % 10) == 0)
		{
			printf("\n");
		}
		printf("..........");
		printf("\033[10D");	/* $1B ESC[10D ����10�������ړ� */
		for(i=0; i<(uPCG_num % 10)+1; i++)
		{
			printf("*");
		}
		printf("\n");
		printf("\033[1A");	/* $1B ESC[1A 1�s��Ɉړ� */
#endif

#ifdef DEBUG
//		printf("g_stPCG_DATA[%d]=0x%p\n", uPCG_num, &g_stPCG_DATA[uPCG_num]);
//		KeyHitESC();	/* �f�o�b�O�p */
#endif
	}

	if((uPCG_SP_offset + uPCG_SP_next) >= PCG_MAX)
	{
		printf("\nERROR:PCG��`OVER(%d >= %d)\n", (uPCG_SP_offset + uPCG_SP_next), PCG_MAX);
	}
	else
	{
		printf("\n(%d)...Ok!\n", (uPCG_SP_offset + uPCG_SP_next));
	}
}

#endif	/* APL_PCG_C */

