/***************************************************************************
* Name  : rev.h
* Brief : 裏返しの処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#if 0 // Ryzen zen2 だと性能低下したのでいったん戻し

UINT64 get_rev(const UINT64 bk, const UINT64 wh, const int x);

#else

extern UINT64(*GetRev[64])(UINT64, UINT64);
#define get_rev(bk, wh, pos) GetRev[pos](bk, wh) 

#endif
