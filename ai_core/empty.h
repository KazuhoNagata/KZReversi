#include "stdafx.h"

#include "cpu.h"

INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline);
INT32 SearchEmpty_2(UINT64 bk, UINT64 wh, INT32 x1, INT32 x2,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline);
INT32 SearchEmpty_3(UINT64 bk, UINT64 wh, UINT64 blank, UINT32 parity,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline);
INT32 SearchEmpty_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);
INT32 SearchEmpty_5(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);