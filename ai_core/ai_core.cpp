/***************************************************************************
* Name  : ai_core.cpp
* Brief : DLL�G�N�X�|�[�g�֐��֘A
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include "board.h"
#include "move.h"
#include "book.h"
#include "eval.h"
#include "bit64.h"
#include "cpu.h"
#include "rev.h"

#define KZ_EXPORT extern "C" __declspec(dllexport)

BOOL m_BookFlag;

/***************************************************************************
* Name  : KZ_LibInit
* Brief : �������������s��
* Return: TRUE/FALSE
****************************************************************************/
KZ_EXPORT BOOL KZ_LibInit()
{
	BOOL result;
	
	// DLL�̃��[�h
	result = AlocMobilityFunc();

	if (result == TRUE)
	{
		// ��΃f�[�^�ƕ]���e�[�u���̃��[�h
		result = LoadData();
	}

	return result;
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : ����\���񋓂���
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetEnumMove(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	return CreateMoves(bk_p, wh_p, p_count_p);
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : �ω�����ӏ����v�Z���A�r�b�g��ɂ��ĕԋp����
* Return: �ω�����ӏ��̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetBoardChangeInfo(UINT64 bk, UINT64 wh, INT32 move)
{
	return GetRev[move](bk, wh);
}

/***************************************************************************
* Name  : KZ_GetCpuMove
* Brief : ��΂�]���l����CPU�̒�����v�Z����
* Args  : bk ���̔Ֆʏ��
*         wh ���̔Ֆʏ��
*         cpuConfig CPU�ݒ�N���X
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetCpuMove(UINT64 bk, UINT64 wh, CPUCONFIG *cpuConfig)
{
	UINT64 move = MOVE_NONE;
	UINT32 emptyNum;
	UINT32 turn;

	emptyNum = CountBit(~(bk | wh));
	turn = 60 - emptyNum;

	if (emptyNum == 0)
	{
		return MOVE_PASS;
	}

	if (cpuConfig->bookFlag)
	{
		// ��΃f�[�^���璅��
		m_BookFlag = TRUE;
		move = GetMoveFromBooks(bk, wh, cpuConfig->color, 
			cpuConfig->bookVariability, turn);

	}

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// ��΂ɊY�����Ȃ��ǖʂ̏ꍇ
	if (move == MOVE_NONE)
	{
		if (cpuConfig->color == BLACK)
		{
			// �T���J�n
			move = GetMoveFromAI(bk, wh, emptyNum, cpuConfig);
		}
		else
		{
			// �T���J�n
			move = GetMoveFromAI(wh, bk, emptyNum, cpuConfig);
		}
	}

	return move;
}

/***************************************************************************
* Name  : KZ_GetLastEvaluation
* Brief : ���O��CPU�̒���ɑΉ�����]���l���擾����
* Return: CPU�̎Z�o�������ߕ]���l
****************************************************************************/
KZ_EXPORT INT32 KZ_GetLastEvaluation()
{
	return g_evaluation;
}

/***************************************************************************
* Name  : KZ_SendAbort
* Brief : AI�X���b�h�ɒ��f���߂𑗐M
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT void KZ_SendAbort()
{
	SetAbortFlag();
}

/***************************************************************************
* Name  : KZ_CountBit
* Brief : �P�������Ă���r�b�g���𐔂���
* Args  : bit �P�������Ă���r�b�g�𐔂���Ώۂ̃r�b�g��
* Return: �P�������Ă���r�b�g��
****************************************************************************/
KZ_EXPORT UINT32 KZ_CountBit(UINT64 bit)
{
	return CountBit(bit);
}