/***************************************************************************
* Name  : eval.h
* Brief : 評価値関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include "empty.h"

#pragma once

#define NEGAMIN -2500000
#define NEGAMAX 2500000
#define EVAL_ONE_STONE 1024
#define WIN 1
#define DRAW 0
#define LOSS -1

#define INDEX_NUM 6561
#define MOBILITY_NUM 36
#define PARITY_NUM 16
// 範囲外の点数
#define INF_SCORE 127
#define STAGE_NUM 61


#define FEATURE_N          12
#define EVAL_N             226234
#define EVAL_FEATURE_SIZE (EVAL_N * sizeof(INT16))

/* 各座標 */
#define A1 0			/* A1 */
#define A2 1			/* A2 */
#define A3 2			/* A3 */
#define A4 3			/* A4 */
#define A5 4			/* A5 */
#define A6 5			/* A6 */
#define A7 6			/* A7 */
#define A8 7			/* A8 */

#define B1 8			/* B1 */
#define B2 9			/* B2 */
#define B3 10			/* B3 */
#define B4 11			/* B4 */
#define B5 12			/* B5 */
#define B6 13			/* B6 */
#define B7 14			/* B7 */
#define B8 15			/* B8 */

#define C1 16			/* C1 */
#define C2 17			/* C2 */
#define C3 18			/* C3 */
#define C4 19			/* C4 */
#define C5 20			/* C5 */
#define C6 21			/* C6 */
#define C7 22			/* C7 */
#define C8 23			/* C8 */

#define D1 24			/* D1 */
#define D2 25			/* D2 */
#define D3 26			/* D3 */
#define D4 27			/* D4 */
#define D5 28			/* D5 */
#define D6 29			/* D6 */
#define D7 30			/* D7 */
#define D8 31			/* D8 */

#define E1 32			/* E1 */
#define E2 33			/* E2 */
#define E3 34			/* E3 */
#define E4 35			/* E4 */
#define E5 36			/* E5 */
#define E6 37			/* E6 */
#define E7 38			/* E7 */
#define E8 39			/* E8 */

#define F1 40			/* F1 */
#define F2 41			/* F2 */
#define F3 42			/* F3 */
#define F4 43			/* F4 */
#define F5 44			/* F5 */
#define F6 45			/* F6 */
#define F7 46			/* F7 */
#define F8 47			/* F8 */

#define G1 48			/* G1 */
#define G2 49			/* G2 */
#define G3 50			/* G3 */
#define G4 51			/* G4 */
#define G5 52			/* G5 */
#define G6 53			/* G6 */
#define G7 54			/* G7 */
#define G8 55			/* G8 */

#define H1 56			/* H1 */
#define H2 57			/* H2 */
#define H3 58			/* H3 */
#define H4 59			/* H4 */
#define H5 60			/* H5 */
#define H6 61			/* H6 */
#define H7 62			/* H7 */
#define H8 63			/* H8 */


typedef struct
{
	UINT16   size;
	INT16   *data[STAGE_NUM];
} st_feature;

typedef struct
{
	st_feature    feature[FEATURE_N];
} st_eval;

extern UINT8 posEval[64];
extern INT32 g_evaluation;

extern UINT64 a1;			/* a1 */
extern UINT64 a2;			/* a2 */
extern UINT64 a3;			/* a3 */
extern UINT64 a4;			/* a4 */
extern UINT64 a5;			/* a5 */
extern UINT64 a6;			/* a6 */
extern UINT64 a7;			/* a7 */
extern UINT64 a8;			/* a8 */

extern UINT64 b1;			/* b1 */
extern UINT64 b2;			/* b2 */
extern UINT64 b3;			/* b3 */
extern UINT64 b4;			/* b4 */
extern UINT64 b5;			/* b5 */
extern UINT64 b6;			/* b6 */
extern UINT64 b7;			/* b7 */
extern UINT64 b8;			/* b8 */

extern UINT64 c1;			/* c1 */
extern UINT64 c2;			/* c2 */
extern UINT64 c3;			/* c3 */
extern UINT64 c4;			/* c4 */
extern UINT64 c5;			/* c5 */
extern UINT64 c6;			/* c6 */
extern UINT64 c7;			/* c7 */
extern UINT64 c8;			/* c8 */

extern UINT64 d1;			/* d1 */
extern UINT64 d2;			/* d2 */
extern UINT64 d3;			/* d3 */
extern UINT64 d4;			/* d4 */
extern UINT64 d5;			/* d5 */
extern UINT64 d6;			/* d6 */
extern UINT64 d7;			/* d7 */
extern UINT64 d8;			/* d8 */

extern UINT64 e1;			/* e1 */
extern UINT64 e2;			/* e2 */
extern UINT64 e3;			/* e3 */
extern UINT64 e4;			/* e4 */
extern UINT64 e5;			/* e5 */
extern UINT64 e6;			/* e6 */
extern UINT64 e7;			/* e7 */
extern UINT64 e8;			/* e8 */

extern UINT64 f1;			/* f1 */
extern UINT64 f2;			/* f2 */
extern UINT64 f3;			/* f3 */
extern UINT64 f4;			/* f4 */
extern UINT64 f5;			/* f5 */
extern UINT64 f6;			/* f6 */
extern UINT64 f7;			/* f7 */
extern UINT64 f8;			/* f8 */

extern UINT64 g1;			/* g1 */
extern UINT64 g2;			/* g2 */
extern UINT64 g3;			/* g3 */
extern UINT64 g4;			/* g4 */
extern UINT64 g5;			/* g5 */
extern UINT64 g6;			/* g6 */
extern UINT64 g7;			/* g7 */
extern UINT64 g8;			/* g8 */

extern UINT64 h1;			/* h1 */
extern UINT64 h2;			/* h2 */
extern UINT64 h3;			/* h3 */
extern UINT64 h4;			/* h4 */
extern UINT64 h5;			/* h5 */
extern UINT64 h6;			/* h6 */
extern UINT64 h7;			/* h7 */
extern UINT64 h8;			/* h8 */

extern INT32 eval_sum;

INT32 Evaluation(UINT8 *board, UINT64 b_board, UINT64 w_board, UINT32 color, UINT32 stage);
INT32 GetExactScore(UINT64 bk, UINT64 wh, INT32 empty);
INT32 GetWinLossScore(UINT64 bk, UINT64 wh, INT32 empty);
BOOL LoadData(void);

/* function empty 0 or end leave empty */
extern INT32(*GetEndScore[])(UINT64 bk, UINT64 wh, INT32 empty);
