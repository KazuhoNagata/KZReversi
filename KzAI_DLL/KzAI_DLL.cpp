// KzAI_DLL.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"

#define KZ_EXPORT extern "C" __declspec(dllexport)

#define MOVE_NONE 0xFF

/* ����\���v�Z�p */
typedef struct {
	unsigned long high;
	unsigned long low;
} st_bit;

typedef void(*INIT_MMX)(void);
typedef int(*BIT_MOB)(st_bit, st_bit, UINT64 *);

BIT_MOB g_bit_mob;
st_bit g_stbit_bk;
st_bit g_stbit_wh;

KZ_EXPORT BOOL LibInit()
{
	HMODULE hDLL;
	INIT_MMX init_mmx;
	BOOL result;

	if ((hDLL = LoadLibrary("mobility.dll")) == NULL)
	{
		return FALSE;
	}

	init_mmx = (INIT_MMX)GetProcAddress(hDLL, "init_mmx");
	g_bit_mob = (BIT_MOB)GetProcAddress(hDLL, "bitboard_mobility");

	(init_mmx)();

	// ��΃f�[�^�ƕ]���e�[�u���̃��[�h(TBI)
	//result = LoadData();

	return result;
}

/***************************************************************************
* Name  : CreateMoves
* Args  : bk - ���̔z�u�r�b�g��
          wh - ���̔z�u�r�b�g��
		  count - ����\���̊i�[�ϐ�
* Brief : ����\���ƒ���\�ʒu�̃r�b�g����v�Z����
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 CreateMoves(UINT64 bk_p, UINT64 wh_p, ULONG *p_count_p)
{
	UINT64 moves;

	g_stbit_bk.high = (bk_p >> 32);
	g_stbit_bk.low = (bk_p & 0xffffffff);

	g_stbit_wh.high = (wh_p >> 32);
	g_stbit_wh.low = (wh_p & 0xffffffff);

	*p_count_p = g_bit_mob(g_stbit_bk, g_stbit_wh, &moves);

	return moves;

}

/***************************************************************************
* Name  : GetCpuMove
* Brief : ��΂�]���l����CPU�̒�����v�Z����
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
KZ_EXPORT UINT64 GetCpuMove()
{
	UINT64 move = 0;

	// book(���)(TBI)
	//move = GetMoveFromBooks();

	if (move == MOVE_NONE)
	{
		// others
	}



	return move;
}