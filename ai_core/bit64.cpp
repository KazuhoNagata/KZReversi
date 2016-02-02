/***************************************************************************
* Name  : bit64.cpp
* Brief : �ǖʂƒ�΃f�[�^���Ƃ炵���킹�Ē���
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include <intrin.h>
#include "bit64.h"

/***************************************************************************
* Name  : CountBit
* Brief : �r�b�g�񂩂�P�������Ă���r�b�g�̐��𐔂���
* Return: �P�������Ă���r�b�g��
****************************************************************************/
UINT32 CountBit(UINT64 bit)
{
	int l_moves = bit & 0x00000000FFFFFFFF;
	int h_moves = (bit & 0xFFFFFFFF00000000) >> 32;

	int count = _mm_popcnt_u32(l_moves);
	count += _mm_popcnt_u32(h_moves);

	return count;
}