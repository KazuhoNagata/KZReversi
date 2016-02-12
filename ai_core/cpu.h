/***************************************************************************
* Name  : search.h
* Brief : �T���̏����S�ʂ��s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#define KEY_HASH_MACRO(b, w) (UINT32)((b ^ ((w) >> 1)) % g_casheSize)

#define ILLIGAL_ARGUMENT 0x80000001
#define MOVE_PASS 0x0

typedef struct
{
	UINT32 color;				// CPU�̐F
	UINT32 casheSize;			// �u���\�̃T�C�Y
	UINT32 searchDepth;			// ���Փǂ݂̐[��
	UINT32 winLossDepth;		// ���s�T�����J�n����[��
	UINT32 exactDepth;			// �΍��T�����J�n����[��
	BOOL   bookFlag;			// ��΂��g�p���邩�ǂ���
	UINT32 bookVariability;	    // ��΂̕ω��x
	BOOL   mpcFlag;				// MPC���g�p���邩�ǂ���
	BOOL   tableFlag;			// �u���\���g�p���邩�ǂ���

}CPUCONFIG;

/* MPC */
typedef struct
{
	int depth;
	int offset;
	int deviation;
}MPCINFO;

extern INT32 g_limitDepth;
extern UINT64 g_casheSize;
extern MPCINFO mpcInfo[22];

/***************************************************************************
* Name  : GetMoveFromAI
* Brief : CPU�̒����T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
UINT64 GetMoveFromAI(UINT64 bk, UINT64 wh, UINT32 emptyNum, CPUCONFIG *cpuConfig);



/***************************************************************************
* Name  : OrderingAlphaBeta
* Brief : ����\��̕��ёւ���󂢒T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, UINT32 depth,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 turn, UINT32 pass_cnt);