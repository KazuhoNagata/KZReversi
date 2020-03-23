#include "stdafx.h"

#pragma once

#include "cpu_root.h"

/** Loop over all empty squares */
#define foreach_empty(empty, list)\
	for ((empty) = (list)->next; (empty)->next; (empty) = (empty)->next)

/** Loop over all empty squares on even quadrants */
#define foreach_even_empty(empty, list, parity)\
	for ((empty) = (list)->next; (empty)->next; (empty) = (empty)->next) if ((parity & empty->quadrant) == 0)

/** Loop over all empty squares on odd quadrants */
#define foreach_odd_empty(empty, list, parity)\
	for ((empty) = (list)->next; (empty)->next; (empty) = (empty)->next) if (parity & empty->quadrant)

/** double linked list of squares */
typedef struct SquareList {
	unsigned long long b;         /*!< bit representation of the square location */
	int x;                        /*!< square location */
	int quadrant;                 /*!< parity quadrant */
	struct SquareList *previous;  /*!< link to previous square */
	struct SquareList *next;      /*!< link to next square */
} SquareList;


INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline);
INT32 SearchEmpty_2(UINT64 bk, UINT64 wh, INT32 x1, INT32 x2, INT32 empty,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline);
INT32 SearchEmpty_3(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 parity,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline);
INT32 SearchEmpty_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);
INT32 SearchEmpty_5(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);