/***************************************************************************
* Name  : rev.h
* Brief : ���Ԃ��̏������s��
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#if 0 // Ryzen zen2 ���Ɛ��\�ቺ�����̂ł�������߂�

UINT64 get_rev(const UINT64 bk, const UINT64 wh, const int x);

#else

extern UINT64(*GetRev[64])(UINT64, UINT64);
#define get_rev(bk, wh, pos) GetRev[pos](bk, wh) 

#endif
