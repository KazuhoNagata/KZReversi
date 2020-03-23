/***************************************************************************
* Name  : eval.cpp
* Brief : 評価値関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include <stdlib.h>
#include "bit64.h"
#include "book.h"
#include "move.h"
#include "eval.h"
#include "board.h"
#include "fio.h"
#include "cpu.h"
#include "zlib.h"

// inner structure
typedef struct
{
	UINT32 size;
	UINT32 decompSize;
	UINT8  compData[1024 * 512];
} st_eval_data;
st_eval_data g_evalData;

// テーブル本体
st_eval  g_st_eval[2];

/* 評価パターンテーブル(現在のステージにより内容が変わるポインタ) */
INT16 *hori_ver1;
INT16 *hori_ver2;
INT16 *hori_ver3;
INT16 *dia_ver1;
INT16 *dia_ver2;
INT16 *dia_ver3;
INT16 *dia_ver4;
INT16 *edge;
INT16 *corner5_2;
INT16 *corner3_3;
INT16 *triangle;
INT16 *constant;

/* 評価パターンテーブル */
INT16 hori_ver1_data[2][STAGE_NUM][INDEX_NUM];
INT16 hori_ver2_data[2][STAGE_NUM][INDEX_NUM];
INT16 hori_ver3_data[2][STAGE_NUM][INDEX_NUM];
INT16 dia_ver1_data[2][STAGE_NUM][INDEX_NUM];
INT16 dia_ver2_data[2][STAGE_NUM][INDEX_NUM / 3];
INT16 dia_ver3_data[2][STAGE_NUM][INDEX_NUM / 9];
INT16 dia_ver4_data[2][STAGE_NUM][INDEX_NUM / 27];
INT16 edge_data[2][STAGE_NUM][INDEX_NUM * 9];
INT16 corner5_2_data[2][STAGE_NUM][INDEX_NUM * 9];
INT16 corner3_3_data[2][STAGE_NUM][INDEX_NUM * 3];
INT16 triangle_data[2][STAGE_NUM][INDEX_NUM * 9];
INT16 constant_data[2][STAGE_NUM][1];

// idx  : 254664 * 2 * sizeof(INT16) ≒ RAM 994KB
UINT16 idx_op_5[2][INDEX_NUM / 27];
UINT16 idx_op_6[2][INDEX_NUM / 9];
UINT16 idx_op_7[2][INDEX_NUM / 3];
UINT16 idx_op_8[2][INDEX_NUM];
UINT16 idx_op_9[2][INDEX_NUM * 3];
UINT16 idx_op_10[2][INDEX_NUM * 9];

int pow_table[10] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

INT32 key_hori_ver1[4];
INT32 key_hori_ver2[4];
INT32 key_hori_ver3[4];
INT32 key_dia_ver1[2];
INT32 key_dia_ver2[4];
INT32 key_dia_ver3[4];
INT32 key_dia_ver4[4];
INT32 key_edge[4];
INT32 key_corner5_2[8];
INT32 key_corner3_3[4];
INT32 key_triangle[4];
INT32 key_mobility;
INT32 key_parity;
//float key_constant;
INT32 eval_sum;

UINT64 a1 = 1ULL;					/* a1 */
UINT64 a2 = (1ULL << 1);			/* a2 */
UINT64 a3 = (1ULL << 2);			/* a3 */
UINT64 a4 = (1ULL << 3);			/* a4 */
UINT64 a5 = (1ULL << 4);			/* a5 */
UINT64 a6 = (1ULL << 5);			/* a6 */
UINT64 a7 = (1ULL << 6);			/* a7 */
UINT64 a8 = (1ULL << 7);			/* a8 */

UINT64 b1 = (1ULL << 8);			/* b1 */
UINT64 b2 = (1ULL << 9);			/* b2 */
UINT64 b3 = (1ULL << 10);			/* b3 */
UINT64 b4 = (1ULL << 11);			/* b4 */
UINT64 b5 = (1ULL << 12);			/* b5 */
UINT64 b6 = (1ULL << 13);			/* b6 */
UINT64 b7 = (1ULL << 14);			/* b7 */
UINT64 b8 = (1ULL << 15);			/* b8 */

UINT64 c1 = (1ULL << 16);			/* c1 */
UINT64 c2 = (1ULL << 17);			/* c2 */
UINT64 c3 = (1ULL << 18);			/* c3 */
UINT64 c4 = (1ULL << 19);			/* c4 */
UINT64 c5 = (1ULL << 20);			/* c5 */
UINT64 c6 = (1ULL << 21);			/* c6 */
UINT64 c7 = (1ULL << 22);			/* c7 */
UINT64 c8 = (1ULL << 23);			/* c8 */

UINT64 d1 = (1ULL << 24);			/* d1 */
UINT64 d2 = (1ULL << 25);			/* d2 */
UINT64 d3 = (1ULL << 26);			/* d3 */
UINT64 d4 = (1ULL << 27);			/* d4 */
UINT64 d5 = (1ULL << 28);			/* d5 */
UINT64 d6 = (1ULL << 29);			/* d6 */
UINT64 d7 = (1ULL << 30);			/* d7 */
UINT64 d8 = (1ULL << 31);			/* d8 */

UINT64 e1 = (1ULL << 32);			/* e1 */
UINT64 e2 = (1ULL << 33);			/* e2 */
UINT64 e3 = (1ULL << 34);			/* e3 */
UINT64 e4 = (1ULL << 35);			/* e4 */
UINT64 e5 = (1ULL << 36);			/* e5 */
UINT64 e6 = (1ULL << 37);			/* e6 */
UINT64 e7 = (1ULL << 38);			/* e7 */
UINT64 e8 = (1ULL << 39);			/* e8 */

UINT64 f1 = (1ULL << 40);			/* f1 */
UINT64 f2 = (1ULL << 41);			/* f2 */
UINT64 f3 = (1ULL << 42);			/* f3 */
UINT64 f4 = (1ULL << 43);			/* f4 */
UINT64 f5 = (1ULL << 44);			/* f5 */
UINT64 f6 = (1ULL << 45);			/* f6 */
UINT64 f7 = (1ULL << 46);			/* f7 */
UINT64 f8 = (1ULL << 47);			/* f8 */

UINT64 g1 = (1ULL << 48);			/* g1 */
UINT64 g2 = (1ULL << 49);			/* g2 */
UINT64 g3 = (1ULL << 50);			/* g3 */
UINT64 g4 = (1ULL << 51);			/* g4 */
UINT64 g5 = (1ULL << 52);			/* g5 */
UINT64 g6 = (1ULL << 53);			/* g6 */
UINT64 g7 = (1ULL << 54);			/* g7 */
UINT64 g8 = (1ULL << 55);			/* g8 */

UINT64 h1 = (1ULL << 56);			/* h1 */
UINT64 h2 = (1ULL << 57);			/* h2 */
UINT64 h3 = (1ULL << 58);			/* h3 */
UINT64 h4 = (1ULL << 59);			/* h4 */
UINT64 h5 = (1ULL << 60);			/* h5 */
UINT64 h6 = (1ULL << 61);			/* h6 */
UINT64 h7 = (1ULL << 62);			/* h7 */
UINT64 h8 = (1ULL << 63);			/* h8 */


UINT8 posEval[64] =
{
	8, 1, 5, 6, 6, 5, 1, 8,
	1, 0, 2, 3, 3, 2, 0, 1,
	5, 2, 7, 4, 4, 7, 2, 5,
	6, 3, 4, 0, 0, 4, 3, 6,
	6, 3, 4, 0, 0, 4, 3, 6,
	5, 2, 7, 4, 4, 7, 2, 5,
	1, 0, 2, 3, 3, 2, 0, 1,
	8, 1, 5, 6, 6, 5, 1, 8
};

INT32 g_evaluation;

/* function empty 0 or end leave empty */
INT32(*GetEndScore[])(UINT64 bk, UINT64 wh, INT32 empty) = {
	GetWinLossScore,
	GetExactScore
};



static int opponent_feature(int l, int d)
{
	const int o[] = { 0, 2, 1 };
	int f = o[l % 3];

	if (d > 1) f += opponent_feature(l / 3, d - 1) * 3;

	return f;
}

static void setEvalIndex()
{
	INT32 i;

	// idx8(hori, diag1)
	for (i = 0; i < INDEX_NUM; i++)
	{
		idx_op_8[0][i] = i;
		idx_op_8[1][opponent_feature(i, 8)] = i;
	}
	// idx7(diag2)
	for (i = 0; i < INDEX_NUM / 3; i++)
	{
		idx_op_7[0][i] = i;
		idx_op_7[1][opponent_feature(i, 7)] = i;
	}
	// idx6(diag3)
	for (i = 0; i < INDEX_NUM / 9; i++)
	{
		idx_op_6[0][i] = i;
		idx_op_6[1][opponent_feature(i, 6)] = i;
	}
	// idx6(diag4)
	for (i = 0; i < INDEX_NUM / 27; i++)
	{
		idx_op_5[0][i] = i;
		idx_op_5[1][opponent_feature(i, 5)] = i;
	}
	// idx9(corner33)
	for (i = 0; i < INDEX_NUM * 3; i++)
	{
		idx_op_9[0][i] = i;
		idx_op_9[1][opponent_feature(i, 9)] = i;
	}
	// idx10(edge, corner52, triangle)
	for (i = 0; i < INDEX_NUM * 9; i++)
	{
		idx_op_10[0][i] = i;
		idx_op_10[1][opponent_feature(i, 10)] = i;
	}
}



INT16   **edata;
BOOL allocEvalMemory(st_eval *eval)
{
	INT32   i, j;
	UINT32  offset;

	// allocate for st_feature.data
	edata = (INT16 **)malloc(sizeof(INT16 *) * 2);
	if (edata == NULL)
	{
		return FALSE;
	}
	edata[0] = (INT16 *)malloc(STAGE_NUM * EVAL_FEATURE_SIZE); // 13800274
	if (edata[0] == NULL)
	{
		free(edata);
		return FALSE;
	}
	edata[1] = (INT16 *)malloc(STAGE_NUM * EVAL_FEATURE_SIZE); // 13800274
	if (edata[1] == NULL)
	{
		free(edata[0]);
		free(edata);
		return FALSE;
	}

	// setting st_feature structure
	for (i = 0; i < 2; i++)
	{
		offset = 0;
		for (j = 0; j < STAGE_NUM; j++)
		{
			// hori1
			eval[i].feature[0].size = INDEX_NUM;
			eval[i].feature[0].data[j] = &edata[i][offset];
			offset += eval[i].feature[0].size;
			// hori2
			eval[i].feature[1].size = INDEX_NUM;
			eval[i].feature[1].data[j] = &edata[i][offset];
			offset += eval[i].feature[1].size;
			// hori3
			eval[i].feature[2].size = INDEX_NUM;
			eval[i].feature[2].data[j] = &edata[i][offset];
			offset += eval[i].feature[2].size;
			// diag1
			eval[i].feature[3].size = INDEX_NUM;
			eval[i].feature[3].data[j] = &edata[i][offset];
			offset += eval[i].feature[3].size;
			// diag2
			eval[i].feature[4].size = INDEX_NUM / 3;
			eval[i].feature[4].data[j] = &edata[i][offset];
			offset += eval[i].feature[4].size;
			// diag3
			eval[i].feature[5].size = INDEX_NUM / 9;
			eval[i].feature[5].data[j] = &edata[i][offset];
			offset += eval[i].feature[5].size;
			// diag4
			eval[i].feature[6].size = INDEX_NUM / 27;
			eval[i].feature[6].data[j] = &edata[i][offset];
			offset += eval[i].feature[6].size;
			// edge
			eval[i].feature[7].size = INDEX_NUM * 9;
			eval[i].feature[7].data[j] = &edata[i][offset];
			offset += eval[i].feature[7].size;
			// corner52
			eval[i].feature[8].size = INDEX_NUM * 9;
			eval[i].feature[8].data[j] = &edata[i][offset];
			offset += eval[i].feature[8].size;
			// corner33
			eval[i].feature[9].size = INDEX_NUM * 3;
			eval[i].feature[9].data[j] = &edata[i][offset];
			offset += eval[i].feature[9].size;
			// triangle
			eval[i].feature[10].size = INDEX_NUM * 9;
			eval[i].feature[10].data[j] = &edata[i][offset];
			offset += eval[i].feature[10].size;
			// constant
			eval[i].feature[11].size = 1;
			eval[i].feature[11].data[j] = &edata[i][offset];
			offset += eval[i].feature[11].size;

			//printf("offset = %d\n", offset);
		}
	}

	return TRUE;
}



INT32 check_h_ver1(UINT8 *board)
{
	int key;
	INT32 eval;

	/* a2 b2 c2 d2 e2 f2 g2 h2 */
	/* a7 b7 c7 d7 e7 f7 g7 h7 */
	/* b1 b2 b3 b4 b5 b6 b7 b8 */
	/* g1 g2 g3 g4 g5 g6 g7 g8 */

	key = board[A2];
	key += 3 * board[B2];
	key += 9 * board[C2];
	key += 27 * board[D2];
	key += 81 * board[E2];
	key += 243 * board[F2];
	key += 729 * board[G2];
	key += 2187 * board[H2];

	eval = hori_ver1[key];
	key_hori_ver1[0] = hori_ver1[key];

	key = board[H7];
	key += 3 * board[G7];
	key += 9 * board[F7];
	key += 27 * board[E7];
	key += 81 * board[D7];
	key += 243 * board[C7];
	key += 729 * board[B7];
	key += 2187 * board[A7];

	eval += hori_ver1[key];
	key_hori_ver1[1] = hori_ver1[key];

	key = board[B8];
	key += 3 * board[B7];
	key += 9 * board[B6];
	key += 27 * board[B5];
	key += 81 * board[B4];
	key += 243 * board[B3];
	key += 729 * board[B2];
	key += 2187 * board[B1];

	eval += hori_ver1[key];
	key_hori_ver1[2] = hori_ver1[key];


	key = board[G1];
	key += 3 * board[G2];
	key += 9 * board[G3];
	key += 27 * board[G4];
	key += 81 * board[G5];
	key += 243 * board[G6];
	key += 729 * board[G7];
	key += 2187 * board[G8];

	eval += hori_ver1[key];
	key_hori_ver1[3] = hori_ver1[key];

	return eval;
}

INT32 check_h_ver2(UINT8 *board)
{
	int key;
	INT32 eval;

	/* a3 b3 c3 d3 e3 f3 g3 h3 */
	/* a6 b6 c6 d6 e6 f6 g6 h6 */
	/* c1 c2 c3 c4 c5 c6 c7 c8 */
	/* f1 f2 f3 f4 f5 f6 f7 f8 */

	key = board[A3];
	key += 3 * board[B3];
	key += 9 * board[C3];
	key += 27 * board[D3];
	key += 81 * board[E3];
	key += 243 * board[F3];
	key += 729 * board[G3];
	key += 2187 * board[H3];

	eval = hori_ver2[key];
	key_hori_ver2[0] = hori_ver2[key];

	key = board[H6];
	key += 3 * board[G6];
	key += 9 * board[F6];
	key += 27 * board[E6];
	key += 81 * board[D6];
	key += 243 * board[C6];
	key += 729 * board[B6];
	key += 2187 * board[A6];

	eval += hori_ver2[key];
	key_hori_ver2[1] = hori_ver2[key];

	key = board[C8];
	key += 3 * board[C7];
	key += 9 * board[C6];
	key += 27 * board[C5];
	key += 81 * board[C4];
	key += 243 * board[C3];
	key += 729 * board[C2];
	key += 2187 * board[C1];

	eval += hori_ver2[key];
	key_hori_ver2[2] = hori_ver2[key];

	key = board[F1];
	key += 3 * board[F2];
	key += 9 * board[F3];
	key += 27 * board[F4];
	key += 81 * board[F5];
	key += 243 * board[F6];
	key += 729 * board[F7];
	key += 2187 * board[F8];

	eval += hori_ver2[key];
	key_hori_ver2[3] = hori_ver2[key];

	return eval;

}

INT32 check_h_ver3(UINT8 *board)
{
	int key;
	INT32 eval;

	/* a4 b4 c4 d4 e4 f4 g4 h4 */
	/* a5 b5 c5 d5 e5 f5 g5 h5 */
	/* d1 d2 d3 d4 d5 d6 d7 d8 */
	/* e1 e2 e3 e4 e5 e6 e7 e8 */

	key = board[A4];
	key += 3 * board[B4];
	key += 9 * board[C4];
	key += 27 * board[D4];
	key += 81 * board[E4];
	key += 243 * board[F4];
	key += 729 * board[G4];
	key += 2187 * board[H4];

	eval = hori_ver3[key];
	key_hori_ver3[0] = hori_ver3[key];

	key = board[H5];
	key += 3 * board[G5];
	key += 9 * board[F5];
	key += 27 * board[E5];
	key += 81 * board[D5];
	key += 243 * board[C5];
	key += 729 * board[B5];
	key += 2187 * board[A5];

	eval += hori_ver3[key];
	key_hori_ver3[1] = hori_ver3[key];

	key = board[D8];
	key += 3 * board[D7];
	key += 9 * board[D6];
	key += 27 * board[D5];
	key += 81 * board[D4];
	key += 243 * board[D3];
	key += 729 * board[D2];
	key += 2187 * board[D1];

	eval += hori_ver3[key];
	key_hori_ver3[2] = hori_ver3[key];

	key = board[E1];
	key += 3 * board[E2];
	key += 9 * board[E3];
	key += 27 * board[E4];
	key += 81 * board[E5];
	key += 243 * board[E6];
	key += 729 * board[E7];
	key += 2187 * board[E8];

	eval += hori_ver3[key];
	key_hori_ver3[3] = hori_ver3[key];

	return eval;

}

INT32 check_dia_ver1(UINT8 *board)
{
	int key;
	INT32 eval;

	/* a1 b2 c3 d4 e5 f6 g7 h8 */
	/* h1 g2 f3 e4 d5 c6 b7 a8  */

	key = board[A1];
	key += 3 * board[B2];
	key += 9 * board[C3];
	key += 27 * board[D4];
	key += 81 * board[E5];
	key += 243 * board[F6];
	key += 729 * board[G7];
	key += 2187 * board[H8];

	eval = dia_ver1[key];
	key_dia_ver1[0] = dia_ver1[key];

	key = board[H1];
	key += 3 * board[G2];
	key += 9 * board[F3];
	key += 27 * board[E4];
	key += 81 * board[D5];
	key += 243 * board[C6];
	key += 729 * board[B7];
	key += 2187 * board[A8];

	eval += dia_ver1[key];
	key_dia_ver1[1] = dia_ver1[key];

	return eval;
}

INT32 check_dia_ver2(UINT8 *board)
{
	int key;
	INT32 eval;

	/* a2 b3 c4 d5 e6 f7 g8 */
	/* b1 c2 d3 e4 f5 g6 h7 */
	/* h2 g3 f4 e5 d6 c7 b8 */
	/* g1 f2 e3 d4 c5 b6 a7 */

	key = board[A2];
	key += 3 * board[B3];
	key += 9 * board[C4];
	key += 27 * board[D5];
	key += 81 * board[E6];
	key += 243 * board[F7];
	key += 729 * board[G8];

	eval = dia_ver2[key];
	key_dia_ver2[0] = dia_ver2[key];

	key = board[H7];
	key += 3 * board[G6];
	key += 9 * board[F5];
	key += 27 * board[E4];
	key += 81 * board[D3];
	key += 243 * board[C2];
	key += 729 * board[B1];

	eval += dia_ver2[key];
	key_dia_ver2[1] = dia_ver2[key];

	key = board[B8];
	key += 3 * board[C7];
	key += 9 * board[D6];
	key += 27 * board[E5];
	key += 81 * board[F4];
	key += 243 * board[G3];
	key += 729 * board[H2];

	eval += dia_ver2[key];
	key_dia_ver2[2] = dia_ver2[key];

	key = board[G1];
	key += 3 * board[F2];
	key += 9 * board[E3];
	key += 27 * board[D4];
	key += 81 * board[C5];
	key += 243 * board[B6];
	key += 729 * board[A7];

	eval += dia_ver2[key];
	key_dia_ver2[3] = dia_ver2[key];

	return eval;
}

INT32 check_dia_ver3(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A3];
	key += 3 * board[B4];
	key += 9 * board[C5];
	key += 27 * board[D6];
	key += 81 * board[E7];
	key += 243 * board[F8];

	eval = dia_ver3[key];
	key_dia_ver3[0] = dia_ver3[key];

	key = board[H6];
	key += 3 * board[G5];
	key += 9 * board[F4];
	key += 27 * board[E3];
	key += 81 * board[D2];
	key += 243 * board[C1];

	eval += dia_ver3[key];
	key_dia_ver3[1] = dia_ver3[key];

	key = board[C8];
	key += 3 * board[D7];
	key += 9 * board[E6];
	key += 27 * board[F5];
	key += 81 * board[G4];
	key += 243 * board[H3];

	eval += dia_ver3[key];
	key_dia_ver3[2] = dia_ver3[key];

	key = board[F1];
	key += 3 * board[E2];
	key += 9 * board[D3];
	key += 27 * board[C4];
	key += 81 * board[B5];
	key += 243 * board[A6];

	eval += dia_ver3[key];
	key_dia_ver3[3] = dia_ver3[key];

	return eval;
}

INT32 check_dia_ver4(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A4];
	key += 3 * board[B5];
	key += 9 * board[C6];
	key += 27 * board[D7];
	key += 81 * board[E8];

	eval = dia_ver4[key];
	key_dia_ver4[0] = dia_ver4[key];

	key = board[H5];
	key += 3 * board[G4];
	key += 9 * board[F3];
	key += 27 * board[E2];
	key += 81 * board[D1];

	eval += dia_ver4[key];
	key_dia_ver4[1] = dia_ver4[key];

	key = board[D8];
	key += 3 * board[E7];
	key += 9 * board[F6];
	key += 27 * board[G5];
	key += 81 * board[H4];

	eval += dia_ver4[key];
	key_dia_ver4[2] = dia_ver4[key];

	key = board[E1];
	key += 3 * board[D2];
	key += 9 * board[C3];
	key += 27 * board[B4];
	key += 81 * board[A5];

	eval += dia_ver4[key];
	key_dia_ver4[3] = dia_ver4[key];

	return eval;
}

INT32 check_edge(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[A5];
	key += 243 * board[A6];
	key += 729 * board[A7];
	key += 2187 * board[A8];
	key += 6561 * board[B2];
	key += 19683 * board[B7];

	eval = edge[key];
	key_edge[0] = edge[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[H4];
	key += 243 * board[H3];
	key += 729 * board[H2];
	key += 2187 * board[H1];
	key += 6561 * board[G7];
	key += 19683 * board[G2];

	eval += edge[key];
	key_edge[1] = edge[key];

	key = board[H1];
	key += 3 * board[G1];
	key += 9 * board[F1];
	key += 27 * board[E1];
	key += 81 * board[D1];
	key += 243 * board[C1];
	key += 729 * board[B1];
	key += 2187 * board[A1];
	key += 6561 * board[G2];
	key += 19683 * board[B2];

	eval += edge[key];
	key_edge[2] = edge[key];

	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[D8];
	key += 81 * board[E8];
	key += 243 * board[F8];
	key += 729 * board[G8];
	key += 2187 * board[H8];
	key += 6561 * board[B7];
	key += 19683 * board[G7];

	eval += edge[key];
	key_edge[3] = edge[key];

	return eval;
}

INT32 check_corner3_3(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[B1];
	key += 81 * board[B2];
	key += 243 * board[B3];
	key += 729 * board[C1];
	key += 2187 * board[C2];
	key += 6561 * board[C3];

	eval = corner3_3[key];
	key_corner3_3[0] = corner3_3[key];

	key = board[H1];
	key += 3 * board[G1];
	key += 9 * board[F1];
	key += 27 * board[H2];
	key += 81 * board[G2];
	key += 243 * board[F2];
	key += 729 * board[H3];
	key += 2187 * board[G3];
	key += 6561 * board[F3];

	eval += corner3_3[key];
	key_corner3_3[1] = corner3_3[key];


	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[A7];
	key += 81 * board[B7];
	key += 243 * board[C7];
	key += 729 * board[A6];
	key += 2187 * board[B6];
	key += 6561 * board[C6];

	eval += corner3_3[key];
	key_corner3_3[2] = corner3_3[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[G8];
	key += 81 * board[G7];
	key += 243 * board[G6];
	key += 729 * board[F8];
	key += 2187 * board[F7];
	key += 6561 * board[F6];

	eval += corner3_3[key];
	key_corner3_3[3] = corner3_3[key];

	return eval;
}

INT32 check_corner5_2(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A1];
	key += 3 * board[B1];
	key += 9 * board[C1];
	key += 27 * board[D1];
	key += 81 * board[E1];
	key += 243 * board[A2];
	key += 729 * board[B2];
	key += 2187 * board[C2];
	key += 6561 * board[D2];
	key += 19683 * board[E2];

	eval = corner5_2[key];
	key_corner5_2[0] = corner5_2[key];

	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[D8];
	key += 81 * board[E8];
	key += 243 * board[A7];
	key += 729 * board[B7];
	key += 2187 * board[C7];
	key += 6561 * board[D7];
	key += 19683 * board[E7];

	eval += corner5_2[key];
	key_corner5_2[1] = corner5_2[key];

	key = board[H1];
	key += 3 * board[G1];
	key += 9 * board[F1];
	key += 27 * board[E1];
	key += 81 * board[D1];
	key += 243 * board[H2];
	key += 729 * board[G2];
	key += 2187 * board[F2];
	key += 6561 * board[E2];
	key += 19683 * board[D2];

	eval += corner5_2[key];
	key_corner5_2[2] = corner5_2[key];

	key = board[H8];
	key += 3 * board[G8];
	key += 9 * board[F8];
	key += 27 * board[E8];
	key += 81 * board[D8];
	key += 243 * board[H7];
	key += 729 * board[G7];
	key += 2187 * board[F7];
	key += 6561 * board[E7];
	key += 19683 * board[D7];

	eval += corner5_2[key];
	key_corner5_2[3] = corner5_2[key];

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[A5];
	key += 243 * board[B1];
	key += 729 * board[B2];
	key += 2187 * board[B3];
	key += 6561 * board[B4];
	key += 19683 * board[B5];

	eval += corner5_2[key];
	key_corner5_2[4] = corner5_2[key];

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[H5];
	key += 243 * board[G1];
	key += 729 * board[G2];
	key += 2187 * board[G3];
	key += 6561 * board[G4];
	key += 19683 * board[G5];

	eval += corner5_2[key];
	key_corner5_2[5] = corner5_2[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[A5];
	key += 81 * board[A4];
	key += 243 * board[B8];
	key += 729 * board[B7];
	key += 2187 * board[B6];
	key += 6561 * board[B5];
	key += 19683 * board[B4];

	eval += corner5_2[key];
	key_corner5_2[6] = corner5_2[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[H4];
	key += 243 * board[G8];
	key += 729 * board[G7];
	key += 2187 * board[G6];
	key += 6561 * board[G5];
	key += 19683 * board[G4];

	eval += corner5_2[key];
	key_corner5_2[7] = corner5_2[key];

	return eval;
}

INT32 check_triangle(UINT8 *board)
{
	int key;
	INT32 eval;

	key = board[A1];
	key += 3 * board[A2];
	key += 9 * board[A3];
	key += 27 * board[A4];
	key += 81 * board[B1];
	key += 243 * board[B2];
	key += 729 * board[B3];
	key += 2187 * board[C1];
	key += 6561 * board[C2];
	key += 19683 * board[D1];

	eval = triangle[key];
	key_triangle[0] = triangle[key];

	key = board[H1];
	key += 3 * board[G1];
	key += 9 * board[F1];
	key += 27 * board[E1];
	key += 81 * board[H2];
	key += 243 * board[G2];
	key += 729 * board[F2];
	key += 2187 * board[H3];
	key += 6561 * board[G3];
	key += 19683 * board[H4];

	eval += triangle[key];
	key_triangle[1] = triangle[key];

	key = board[A8];
	key += 3 * board[B8];
	key += 9 * board[C8];
	key += 27 * board[D8];
	key += 81 * board[A7];
	key += 243 * board[B7];
	key += 729 * board[C7];
	key += 2187 * board[A6];
	key += 6561 * board[B6];
	key += 19683 * board[A5];

	eval += triangle[key];
	key_triangle[2] = triangle[key];

	key = board[H8];
	key += 3 * board[H7];
	key += 9 * board[H6];
	key += 27 * board[H5];
	key += 81 * board[G8];
	key += 243 * board[G7];
	key += 729 * board[G6];
	key += 2187 * board[F8];
	key += 6561 * board[F7];
	key += 19683 * board[E8];

	eval += triangle[key];
	key_triangle[3] = triangle[key];

	return eval;
}

#if 0
INT32 check_parity(UINT64 blank, UINT32 color)
{
	int p;

	p = CountBit(blank & 0x0f0f0f0f) % 2;
	p |= (CountBit(blank & 0xf0f0f0f0) % 2) << 1;
	p |= (CountBit(blank & 0x0f0f0f0f00000000) % 2) << 2;
	p |= (CountBit(blank & 0xf0f0f0f000000000) % 2) << 3;

	return parity[p];
}

INT32 check_mobility(UINT64 b_board, UINT64 w_board)
{
	UINT32 mob1;

	CreateMoves(b_board, w_board, &mob1);
	//CreateMoves(w_board, b_board, &mob2);
	key_mobility = mobility[mob1];

	return key_mobility;
}

#endif

/**
* @brief Get the final score.
*
* Get the final score, when no move can be made.
*
* @param board Board.
* @param n_empties Number of empty squares remaining on the board.
* @return The final score, as a disc difference.
*/
INT32 GetExactScore(UINT64 bk, UINT64 wh, INT32 empty)
{
	const int n_discs_p = CountBit(bk);
	const int n_discs_o = 64 - empty - n_discs_p;

	int score = n_discs_p - n_discs_o;

	if (score > 0) score += empty;
	else if (score < 0) score -= empty;
#ifdef LOSSGAME
	return -score;
#else
	return score;
#endif
}


/**
* @brief Get the final score.
*
* Get the final score, when no move can be made.
*
* @param board Board.
* @param n_empties Number of empty squares remaining on the board.
* @return The final score, as a disc difference.
*/
INT32 GetWinLossScore(UINT64 bk, UINT64 wh, INT32 empty)
{
	const int n_discs_p = CountBit(bk);
	const int n_discs_o = 64 - empty - n_discs_p;
	int score = n_discs_p - n_discs_o;

	if (score > 0) score += empty;
	else if (score < 0) score -= empty;
#ifdef LOSSGAME
	if (n_discs_p < n_discs_o)
	{
		score = WIN;
	}
	else if (n_discs_p > n_discs_o)
	{
		score = LOSS;
	}
	else
	{
		score = DRAW;
	}
#else
	if (score > 0)
	{
		score = WIN;
	}
	else if (score < 0)
	{
		score = LOSS;
	}
	else
	{
		score = DRAW;
	}
#endif
	return score;

}

INT32 Evaluation(UINT8 *board, UINT64 bk, UINT64 wh, UINT32 color, UINT32 stage)
{
	INT32 score;

	/* 現在の色とステージでポインタを指定 */
#if 1
	color = 0;
	hori_ver1 = hori_ver1_data[color][stage];
	hori_ver2 = hori_ver2_data[color][stage];
	hori_ver3 = hori_ver3_data[color][stage];
	dia_ver1 = dia_ver1_data[color][stage];
	dia_ver2 = dia_ver2_data[color][stage];
	dia_ver3 = dia_ver3_data[color][stage];
	dia_ver4 = dia_ver4_data[color][stage];
	edge = edge_data[color][stage];
	corner5_2 = corner5_2_data[color][stage];
	corner3_3 = corner3_3_data[color][stage];
	triangle = triangle_data[color][stage];
	constant = constant_data[color][stage];
#else
	color = 0;
	hori_ver1 = g_st_eval[color].feature[0].data[stage];
	hori_ver2 = g_st_eval[color].feature[1].data[stage];
	hori_ver3 = g_st_eval[color].feature[2].data[stage];
	dia_ver1  = g_st_eval[color].feature[3].data[stage];
	dia_ver2  = g_st_eval[color].feature[4].data[stage];
	dia_ver3  = g_st_eval[color].feature[5].data[stage];
	dia_ver4  = g_st_eval[color].feature[6].data[stage];
	edge      = g_st_eval[color].feature[7].data[stage];
	corner5_2 = g_st_eval[color].feature[8].data[stage];
	corner3_3 = g_st_eval[color].feature[9].data[stage];
	triangle  = g_st_eval[color].feature[10].data[stage];
	constant  = g_st_eval[color].feature[11].data[stage];
#endif
	score = check_h_ver1(board);
	score += check_h_ver2(board);
	score += check_h_ver3(board);

	score += check_dia_ver1(board);
	score += check_dia_ver2(board);
	score += check_dia_ver3(board);
	score += check_dia_ver4(board);

	score += check_edge(board);
	score += check_corner5_2(board);
	score += check_corner3_3(board);
	score += check_triangle(board);

	score += constant[0];

	return score;

}


#if 1

static BOOL storeEvalData(UCHAR *buf, INT32 stage)
{
	INT32     i;
	INT32     offset;
	INT16    *p_table;
	INT16    *p_table_op;
	UINT32    size;

	size = ((UINT32)buf[0] << 24) + ((UINT32)buf[1] << 16) + ((UINT32)buf[2] << 8) + buf[3];

	offset = 4; // skip length data
#if 1
	/* horizon_ver1 */
	p_table = hori_ver1_data[0][stage];
	p_table_op = hori_ver1_data[1][stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[idx_op_8[1][i]] = -p_table[i];

	/* horizon_ver2 */
	p_table = hori_ver2_data[0][stage];
	p_table_op = hori_ver2_data[1][stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[idx_op_8[1][i]] = -p_table[i];

	/* horizon_ver3 */
	p_table = hori_ver3_data[0][stage];
	p_table_op = hori_ver3_data[1][stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[idx_op_8[1][i]] = -p_table[i];

	/* diagram_ver1 */
	p_table = dia_ver1_data[0][stage];
	p_table_op = dia_ver1_data[1][stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[idx_op_8[1][i]] = -p_table[i];

	/* diagram_ver2 */
	p_table = dia_ver2_data[0][stage];
	p_table_op = dia_ver2_data[1][stage];
	for (i = 0; i < INDEX_NUM / 3; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 3; i++) p_table_op[idx_op_7[1][i]] = -p_table[i];
	
	/* diagram_ver3 */
	p_table = dia_ver3_data[0][stage];
	p_table_op = dia_ver3_data[1][stage];
	for (i = 0; i < INDEX_NUM / 9; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 9; i++) p_table_op[idx_op_6[1][i]] = -p_table[i];

	/* diagram_ver4 */
	p_table = dia_ver4_data[0][stage];
	p_table_op = dia_ver4_data[1][stage];
	for (i = 0; i < INDEX_NUM / 27; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 27; i++) p_table_op[idx_op_5[1][i]] = -p_table[i];

	/* edge */
	p_table = edge_data[0][stage];
	p_table_op = edge_data[1][stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[idx_op_10[1][i]] = -p_table[i];

	/* corner5 + 2X */
	p_table = corner5_2_data[0][stage];
	p_table_op = corner5_2_data[1][stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[idx_op_10[1][i]] = -p_table[i];

	/* corner3_3 */
	p_table = corner3_3_data[0][stage];
	p_table_op = corner3_3_data[1][stage];
	for (i = 0; i < INDEX_NUM * 3; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 3; i++) p_table_op[idx_op_9[1][i]] = -p_table[i];

	/* triangle */
	p_table = triangle_data[0][stage];
	p_table_op = triangle_data[1][stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[idx_op_10[1][i]] = -p_table[i];

	p_table = constant_data[0][stage];
	p_table_op = constant_data[1][stage];
	p_table[0] = ((INT16)buf[offset + 1] << 8) + buf[offset];
	p_table_op[0] = -p_table[0];
	offset += 2;

#else
	/* horizon_ver1 */
	p_table = g_st_eval[0].feature[0].data[stage];
	p_table_op = g_st_eval[1].feature[0].data[stage];
	for (i = 0, offset = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset] << 8) + buf[offset + 1];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[i] = -p_table[idx_op_8[1][i]];

	/* horizon_ver2 */
	p_table = g_st_eval[0].feature[1].data[stage];
	p_table_op = g_st_eval[1].feature[1].data[stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[i] = -p_table[idx_op_8[1][i]];

	/* horizon_ver3 */
	p_table = g_st_eval[0].feature[2].data[stage];
	p_table_op = g_st_eval[1].feature[2].data[stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[i] = -p_table[idx_op_8[1][i]];

	/* diagram_ver1 */
	p_table = g_st_eval[0].feature[3].data[stage];
	p_table_op = g_st_eval[1].feature[3].data[stage];
	for (i = 0; i < INDEX_NUM; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM; i++) p_table_op[i] = -p_table[idx_op_8[1][i]];

	/* diagram_ver2 */
	p_table = g_st_eval[0].feature[4].data[stage];
	p_table_op = g_st_eval[1].feature[4].data[stage];
	for (i = 0; i < INDEX_NUM / 3; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 3; i++) p_table_op[i] = -p_table[idx_op_7[1][i]];

	/* diagram_ver3 */
	p_table = g_st_eval[0].feature[5].data[stage];
	p_table_op = g_st_eval[1].feature[5].data[stage];
	for (i = 0; i < INDEX_NUM / 9; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 9; i++) p_table_op[i] = -p_table[idx_op_6[1][i]];

	/* diagram_ver4 */
	p_table = g_st_eval[0].feature[6].data[stage];
	p_table_op = g_st_eval[1].feature[6].data[stage];
	for (i = 0; i < INDEX_NUM / 27; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM / 27; i++) p_table_op[i] = -p_table[idx_op_5[1][i]];

	/* edge */
	p_table = g_st_eval[0].feature[7].data[stage];
	p_table_op = g_st_eval[1].feature[7].data[stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[i] = -p_table[idx_op_10[1][i]];


	/* corner5_2 */
	p_table = g_st_eval[0].feature[8].data[stage];
	p_table_op = g_st_eval[1].feature[8].data[stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[i] = -p_table[idx_op_10[1][i]];

	/* corner3_3 */
	p_table = g_st_eval[0].feature[9].data[stage];
	p_table_op = g_st_eval[1].feature[9].data[stage];
	for (i = 0; i < INDEX_NUM * 3; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 3; i++) p_table_op[i] = -p_table[idx_op_9[1][i]];

	/* triangle */
	p_table = g_st_eval[0].feature[10].data[stage];
	p_table_op = g_st_eval[1].feature[10].data[stage];
	for (i = 0; i < INDEX_NUM * 9; i++, offset += 2)
	{
		p_table[i] = ((UINT16)buf[offset + 1] << 8) + buf[offset];
	}
	for (i = 0; i < INDEX_NUM * 9; i++) p_table_op[i] = -p_table[idx_op_10[1][i]];

	/* constant */
	p_table = g_st_eval[0].feature[10].data[stage];
	p_table_op = g_st_eval[1].feature[10].data[stage];
	p_table[0] = ((UINT16)buf[offset] << 8) + buf[offset + 1];
	p_table_op[0] = p_table[0];
	offset += 2;
#endif
	// 長さチェック
	if (offset != size + 4) return FALSE;

	return TRUE;
}



static BOOL unmarshalEvalData(UINT8 *in, UINT32 inSize, st_eval_data *evalData)
{
	BOOL   ret;
	UINT32 cnt;

	ret = TRUE;
	cnt = 0;

	evalData->size  = (UINT32)in[cnt++] << 24;
	evalData->size |= (UINT32)in[cnt++] << 16;
	evalData->size |= (UINT32)in[cnt++] << 8;
	evalData->size |= (UINT32)in[cnt++];

	evalData->decompSize = (UINT32)in[cnt++] << 24;
	evalData->decompSize |= (UINT32)in[cnt++] << 16;
	evalData->decompSize |= (UINT32)in[cnt++] << 8;
	evalData->decompSize |= (UINT32)in[cnt++];

	memcpy_s(evalData->compData, sizeof(evalData->compData), &in[cnt], inSize - cnt);
	cnt += evalData->size;

	return ret;
}


INT32 do_decompress(INT32 stage, UINT8 *in, UINT32 inSize, UINT8 *out, UINT32 outSize)        /* 展開（復元） */
{
	UINT32 ret;
	int count, status;
	z_stream z;

	// 構造体のアンマーシャル
	ret = unmarshalEvalData(in, inSize, &g_evalData);
	if (ret == FALSE) return -1;

	/* すべてのメモリ管理をライブラリに任せる */
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	/* 初期化 */
	if (inflateInit(&z) != Z_OK)
	{
		fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
		return -1;
	}

	z.next_in = Z_NULL;
	z.avail_in = 0;
	z.next_out = out;
	z.avail_out = outSize;
	status = Z_OK;

	if (z.avail_in == 0)
	{
		z.avail_in = g_evalData.size;
		memcpy_s(in, inSize, g_evalData.compData, g_evalData.size);
		z.next_in = in;
	}
	status = inflate(&z, Z_FINISH); /* 展開 */
	if (status == Z_STREAM_END)
	{
		count = outSize - z.avail_out;
		// 評価値をメモリに書き込み
		if (storeEvalData(out, stage) == FALSE) return -1;
	}

	/* 後始末 */
	if (inflateEnd(&z) != Z_OK) {
		fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
		return -1;
	}

	return 0;
}


BOOL OpenEvalData(char *filename)
{
	FILE *fp;
	INT32 result;
	INT32 stage;
	Bytef *ibuffer, *obuffer;
	INT32 inSize, outSize;

	result = fopen_s(&fp, filename, "rb");
	if (result != 0 || fp == NULL) return FALSE;

	//fopen_s(&g_wfp, "error_log.txt", "w");

	// インデックス値を設定
	setEvalIndex();

	// データアロケーション
	//if (allocEvalMemory(g_st_eval) == FALSE) return FALSE;

	inSize = 1024 * 1024;
	outSize = EVAL_FEATURE_SIZE + 4; // eval data + len data(4byte)
	ibuffer = malloc(inSize);
	if (ibuffer == NULL) return FALSE;
	obuffer = malloc(outSize);
	if (obuffer == NULL) return FALSE;

	for(stage = 0; stage < STAGE_NUM; stage++)
	{
		// ヘッダ読み込み(4byte)
		fread(&inSize, sizeof(UINT8), 4, fp);
		inSize = (INT32)fread(ibuffer, sizeof(UINT8), inSize, fp); // about 530KB
		result = do_decompress(stage, ibuffer, inSize, obuffer, outSize);
		if (result == -1) break;
	}

	free(ibuffer);
	free(obuffer);
	fclose(fp);

	if (result == -1) return FALSE;

	return TRUE;
}

#endif

BOOL LoadData()
{
	BOOL result;
	char filename[32];

	/* 定石データの読み込み */
#if 1
	result = OpenBook("src\\books.bin");
	if (result == FALSE)
	{
		return result;
	}
#endif

	/* 評価テーブルの読み込み */
	result = OpenEvalData("src\\eval.bin");
	if (result == FALSE)
	{
		return result;
	}

	/* MPCテーブルの読み込み */
	for(int i = 0; i < 56; i++)
	{
		sprintf_s(filename, sizeof(filename), "src\\mpc%d.dat", i);
		result = OpenMpcInfoData(mpcInfo[i], filename);
		if (result == FALSE)
		{
			return result;
		}
	}
	

	result = OpenMpcInfoData(mpcInfo_end, "src\\mpc_end.dat");

	return result;
}