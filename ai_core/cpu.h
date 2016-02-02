/***************************************************************************
* Name  : search.h
* Brief : �T���̏����S�ʂ��s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#define KEY_HASH_MACRO(b, w) (UINT32)((b ^ ((w) >> 1)) % g_casheSize)

#define ILLIGAL_ARGUMENT 0x80000001

typedef struct
{
	UINT32 color;				// CPU�̐F
	UINT32 casheSize;			// �u���\�̃T�C�Y
	UINT32 searchDepth;			// ���Փǂ݂̐[��
	UINT32 winLossDepth;		// ���s�T�����J�n����[��
	UINT32 exactDepth;			// �΍��T�����J�n����[��
	BOOL   bookFlag;			// ��΂��g�p���邩�ǂ���
	BOOL   bookVariability;		// ��΂̕ω��x
	BOOL   mpcFlag;				// MPC���g�p���邩�ǂ���
	BOOL   tableFlag;			// �u���\���g�p���邩�ǂ���

}CPUCONFIG;

extern INT32 g_limitDepth;
extern UINT64 g_casheSize;

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