// KzAI_DLL.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"
#include "move.h"
#include "book.h"
#include "bit64.h"
#include "cpu.h"

#define KZ_EXPORT extern "C" __declspec(dllexport)

#define MOVE_NONE 0xFF

BOOL m_BookFlag;

KZ_EXPORT BOOL LibInit()
{
	BOOL result;
	
	// DLL�̃��[�h
	result = AlocMobilityFunc();

	// ��΃f�[�^�ƕ]���e�[�u���̃��[�h(TBI)
	//result = LoadData();

	return result;
}

/***************************************************************************
* Name  : EnumGetCpuMove
* Brief : ����\���񋓂���
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 EnumGetMove(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	return CreateMoves(bk_p, wh_p, p_count_p);
}

/***************************************************************************
* Name  : GetCpuMove
* Brief : ��΂�]���l����CPU�̒�����v�Z����
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 GetCpuMove(UINT64 bk, UINT64 wh, CPUCONFIG *cpuConfig)
{
	UINT64 move;
	UINT32 emptyNum;

	emptyNum = CountBit(~(bk | wh));

	if (cpuConfig->bookFlag)
	{
		// ��΃f�[�^���璅��
		m_BookFlag = TRUE;
		move = GetMoveFromBooks(bk, wh, cpuConfig->color, 
			cpuConfig->bookVariability, emptyNum);

	}

	// ��΂ɊY�����Ȃ��ǖʂ̏ꍇ
	if (move == MOVE_NONE)
	{
		// others
		move = GetMoveFromAI(bk, wh, emptyNum, cpuConfig);
	}

	return move;
}