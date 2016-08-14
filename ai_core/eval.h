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
#define EVAL_ONE_STONE 10000
#define WIN 1
#define DRAW 0
#define LOSS -1

#define INDEX_NUM 6561
#define MOBILITY_NUM 1
#define PARITY_NUM 16
// 範囲外の点数
#define INF_SCORE 127


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

extern float eval_sum;

INT32 Evaluation(UINT8 *board, UINT64 b_board, UINT64 w_board, UINT32 color, UINT32 stage);
INT32 GetExactScore(UINT64 bk, UINT64 wh, INT32 empty);
INT32 GetWinLossScore(UINT64 bk, UINT64 wh, INT32 empty);
BOOL LoadData(void);


/* function empty 0 or end leave empty */
extern INT32(*GetEndScore[])(UINT64 bk, UINT64 wh, INT32 empty);
