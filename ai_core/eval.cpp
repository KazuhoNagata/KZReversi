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

/* 評価パターンテーブル(現在のステージにより内容が変わるポインタ) */
float *hori_ver1;
float *hori_ver2;
float *hori_ver3;
float *dia_ver1;
float *dia_ver2;
float *dia_ver3;
float *dia_ver4;
float *edge;
float *corner5_2;
float *corner3_3;
float *triangle;
float *mobility;
float *parity;

/* 評価パターンテーブル(おおもと) */
float hori_ver1_data[2][60][INDEX_NUM];
float hori_ver2_data[2][60][INDEX_NUM];
float hori_ver3_data[2][60][INDEX_NUM];
float dia_ver1_data[2][60][INDEX_NUM];
float dia_ver2_data[2][60][INDEX_NUM / 3];
float dia_ver3_data[2][60][INDEX_NUM / 9];
float dia_ver4_data[2][60][INDEX_NUM / 27];
float edge_data[2][60][INDEX_NUM * 9];
float corner5_2_data[2][60][INDEX_NUM * 9];
float corner3_3_data[2][60][INDEX_NUM * 3];
float triangle_data[2][60][INDEX_NUM * 9];
float mobility_data[2][60][MOBILITY_NUM];
float parity_data[2][60][PARITY_NUM];
float constant_data[2][60];

int pow_table[10] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

double key_hori_ver1[4];
double key_hori_ver2[4];
double key_hori_ver3[4];
double key_dia_ver1[2];
double key_dia_ver2[4];
double key_dia_ver3[4];
double key_dia_ver4[4];
double key_edge[4];
double key_corner5_2[8];
double key_corner3_3[4];
double key_triangle[4];
double key_mobility;
double key_parity;
double key_constant;
double eval_sum;

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

double check_h_ver1(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[A7];
	key += 3 * board[B7];
	key += 9 * board[C7];
	key += 27 * board[D7];
	key += 81 * board[E7];
	key += 243 * board[F7];
	key += 729 * board[G7];
	key += 2187 * board[H7];

	eval += hori_ver1[key];
	key_hori_ver1[1] = hori_ver1[key];

	key = board[B1];
	key += 3 * board[B2];
	key += 9 * board[B3];
	key += 27 * board[B4];
	key += 81 * board[B5];
	key += 243 * board[B6];
	key += 729 * board[B7];
	key += 2187 * board[B8];

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

double check_h_ver2(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[A6];
	key += 3 * board[B6];
	key += 9 * board[C6];
	key += 27 * board[D6];
	key += 81 * board[E6];
	key += 243 * board[F6];
	key += 729 * board[G6];
	key += 2187 * board[H6];

	eval += hori_ver2[key];
	key_hori_ver2[1] = hori_ver2[key];

	key = board[C1];
	key += 3 * board[C2];
	key += 9 * board[C3];
	key += 27 * board[C4];
	key += 81 * board[C5];
	key += 243 * board[C6];
	key += 729 * board[C7];
	key += 2187 * board[C8];

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

double check_h_ver3(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[A5];
	key += 3 * board[B5];
	key += 9 * board[C5];
	key += 27 * board[D5];
	key += 81 * board[E5];
	key += 243 * board[F5];
	key += 729 * board[G5];
	key += 2187 * board[H5];

	eval += hori_ver3[key];
	key_hori_ver3[1] = hori_ver3[key];

	key = board[D1];
	key += 3 * board[D2];
	key += 9 * board[D3];
	key += 27 * board[D4];
	key += 81 * board[D5];
	key += 243 * board[D6];
	key += 729 * board[D7];
	key += 2187 * board[D8];

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

double check_dia_ver1(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[A8];
	key += 3 * board[B7];
	key += 9 * board[C6];
	key += 27 * board[D5];
	key += 81 * board[E4];
	key += 243 * board[F3];
	key += 729 * board[G2];
	key += 2187 * board[H1];

	eval += dia_ver1[key];
	key_dia_ver1[1] = dia_ver1[key];

	return eval;
}

double check_dia_ver2(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[B1];
	key += 3 * board[C2];
	key += 9 * board[D3];
	key += 27 * board[E4];
	key += 81 * board[F5];
	key += 243 * board[G6];
	key += 729 * board[H7];

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

	key = board[A7];
	key += 3 * board[B6];
	key += 9 * board[C5];
	key += 27 * board[D4];
	key += 81 * board[E3];
	key += 243 * board[F2];
	key += 729 * board[G1];

	eval += dia_ver2[key];
	key_dia_ver2[3] = dia_ver2[key];

	return eval;
}

double check_dia_ver3(UINT8 *board)
{
	int key;
	double eval;

	key = board[A3];
	key += 3 * board[B4];
	key += 9 * board[C5];
	key += 27 * board[D6];
	key += 81 * board[E7];
	key += 243 * board[F8];

	eval = dia_ver3[key];
	key_dia_ver3[0] = dia_ver3[key];

	key = board[C1];
	key += 3 * board[D2];
	key += 9 * board[E3];
	key += 27 * board[F4];
	key += 81 * board[G5];
	key += 243 * board[H6];

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

	key = board[A6];
	key += 3 * board[B5];
	key += 9 * board[C4];
	key += 27 * board[D3];
	key += 81 * board[E2];
	key += 243 * board[F1];

	eval += dia_ver3[key];
	key_dia_ver3[3] = dia_ver3[key];

	return eval;
}

double check_dia_ver4(UINT8 *board)
{
	int key;
	double eval;

	key = board[A4];
	key += 3 * board[B5];
	key += 9 * board[C6];
	key += 27 * board[D7];
	key += 81 * board[E8];

	eval = dia_ver4[key];
	key_dia_ver4[0] = dia_ver4[key];

	key = board[D1];
	key += 3 * board[E2];
	key += 9 * board[F3];
	key += 27 * board[G4];
	key += 81 * board[H5];

	eval += dia_ver4[key];
	key_dia_ver4[1] = dia_ver4[key];

	key = board[D8];
	key += 3 * board[E7];
	key += 9 * board[F6];
	key += 27 * board[G5];
	key += 81 * board[H4];

	eval += dia_ver4[key];
	key_dia_ver4[2] = dia_ver4[key];

	key = board[A5];
	key += 3 * board[B4];
	key += 9 * board[C3];
	key += 27 * board[D2];
	key += 81 * board[E1];

	eval += dia_ver4[key];
	key_dia_ver4[3] = dia_ver4[key];

	return eval;
}

double check_edge(UINT8 *board)
{
	int key;
	double eval;

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

	key = board[H1];
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[H5];
	key += 243 * board[H6];
	key += 729 * board[H7];
	key += 2187 * board[H8];
	key += 6561 * board[G2];
	key += 19683 * board[G7];

	eval += edge[key];
	key_edge[1] = edge[key];

	key = board[A1];
	key += 3 * board[B1];
	key += 9 * board[C1];
	key += 27 * board[D1];
	key += 81 * board[E1];
	key += 243 * board[F1];
	key += 729 * board[G1];
	key += 2187 * board[H1];
	key += 6561 * board[B2];
	key += 19683 * board[G2];

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

double check_corner3_3(UINT8 *board)
{
	int key;
	double eval;

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
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[G1];
	key += 81 * board[G2];
	key += 243 * board[G3];
	key += 729 * board[F1];
	key += 2187 * board[F2];
	key += 6561 * board[F3];

	eval += corner3_3[key];
	key_corner3_3[1] = corner3_3[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[B8];
	key += 81 * board[B7];
	key += 243 * board[B6];
	key += 729 * board[C8];
	key += 2187 * board[C7];
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

double check_corner5_2(UINT8 *board)
{
	int key;
	double eval;

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

double check_triangle(UINT8 *board)
{
	int key;
	double eval;

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
	key += 3 * board[H2];
	key += 9 * board[H3];
	key += 27 * board[H4];
	key += 81 * board[G1];
	key += 243 * board[G2];
	key += 729 * board[G3];
	key += 2187 * board[F1];
	key += 6561 * board[F2];
	key += 19683 * board[E1];

	eval += triangle[key];
	key_triangle[1] = triangle[key];

	key = board[A8];
	key += 3 * board[A7];
	key += 9 * board[A6];
	key += 27 * board[A5];
	key += 81 * board[B8];
	key += 243 * board[B7];
	key += 729 * board[B6];
	key += 2187 * board[C8];
	key += 6561 * board[C7];
	key += 19683 * board[D8];

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

double check_parity(UINT64 blank, UINT32 color)
{
	int p;

	p = (CountBit(blank) % 2) + (color * 2);
	key_parity = parity[p];

	return parity[p];
}

double check_mobility(UINT64 b_board, UINT64 w_board)
{
	UINT32 mob1, mob2;

	CreateMoves(b_board, w_board, &mob1);
	CreateMoves(w_board, b_board, &mob2);
	key_mobility = mobility[mob1 - mob2 + 32];

	return key_mobility;
}

INT32 Evaluation(UINT8 *board, UINT64 b_board, UINT64 w_board, UINT32 color, UINT32 stage)
{
	double eval;

	/* 現在の色とステージでポインタを指定 */
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
	mobility = mobility_data[0][stage];
	parity = parity_data[0][stage];

	eval = check_h_ver1(board);
	eval += check_h_ver2(board);
	eval += check_h_ver3(board);

	eval += check_dia_ver1(board);
	eval += check_dia_ver2(board);
	eval += check_dia_ver3(board);
	eval += check_dia_ver4(board);

	eval += check_edge(board);
	eval += check_corner5_2(board);
	eval += check_corner3_3(board);
	eval += check_triangle(board);

	eval += check_mobility(b_board, w_board);
	eval += constant_data[color][stage];

	eval_sum = eval;
	eval *= EVAL_ONE_STONE;

	return (INT32)eval;

}

int opponent_feature(int l, int d)
{
	const int o[] = { 0, 2, 1 };
	int f = o[l % 3];

	if (d > 1) f += opponent_feature(l / 3, d - 1) * 3;

	return f;
}

BOOL OpenEvalData(char *filename)
{
	int stage = 0;
	int i, evalSize;
	UCHAR *buf;
	char *line, *ctr;
	float *p_table, *p_table_op;

	buf = DecodeEvalData(&evalSize, filename);

	if (buf == NULL)
	{
		return FALSE;
	}

	line = strtok_s((char *)buf, "\n", &ctr);

	while (stage < 60)
	{
		/* horizon_ver1 */
		p_table = hori_ver1_data[0][stage];
		p_table_op = hori_ver1_data[1][stage];
		for (i = 0; i < 6561; i++)
		{
			p_table[i] = (float)atof(line);
			/* opponent */
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* horizon_ver2 */
		p_table = hori_ver2_data[0][stage];
		p_table_op = hori_ver2_data[1][stage];
		for (i = 0; i < 6561; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* horizon_ver3 */
		p_table = hori_ver3_data[0][stage];
		p_table_op = hori_ver3_data[1][stage];
		for (i = 0; i < 6561; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver1 */
		p_table = dia_ver1_data[0][stage];
		p_table_op = dia_ver1_data[1][stage];
		for (i = 0; i < 6561; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 8)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver2 */
		p_table = dia_ver2_data[0][stage];
		p_table_op = dia_ver2_data[1][stage];
		for (i = 0; i < 2187; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 7)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver3 */
		p_table = dia_ver3_data[0][stage];
		p_table_op = dia_ver3_data[1][stage];
		for (i = 0; i < 729; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 6)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* diagram_ver4 */
		p_table = dia_ver4_data[0][stage];
		p_table_op = dia_ver4_data[1][stage];
		for (i = 0; i < 243; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 5)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* edge */
		p_table = edge_data[0][stage];
		p_table_op = edge_data[1][stage];
		for (i = 0; i < 59049; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* corner5 + 2X */
		p_table = corner5_2_data[0][stage];
		p_table_op = corner5_2_data[1][stage];
		for (i = 0; i < 59049; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* corner3_3 */
		p_table = corner3_3_data[0][stage];
		p_table_op = corner3_3_data[1][stage];
		for (i = 0; i < 19683; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 9)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* triangle */
		p_table = triangle_data[0][stage];
		p_table_op = triangle_data[1][stage];
		for (i = 0; i < 59049; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[opponent_feature(i, 10)] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* mobility */
		p_table = mobility_data[0][stage];
		p_table_op = mobility_data[1][stage];
		for (i = 0; i < MOBILITY_NUM; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[i] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}

		/* parity */
		p_table = parity_data[0][stage];
		p_table_op = parity_data[1][stage];
		for (i = 0; i < PARITY_NUM; i++)
		{
			p_table[i] = (float)atof(line);
			p_table_op[i] = -p_table[i];
			line = strtok_s(NULL, "\n", &ctr);
		}
		
		/* constant */
		constant_data[0][stage] = (float)atof(line);
		line = strtok_s(NULL, "\n", &ctr);

		stage++;
	}

	free(buf);

	return TRUE;
}

BOOL LoadData()
{
	/* 定石データの読み込み */
	BOOL result = OpenBook("src\\books.bin");
	if (result == FALSE)
	{
		return result;
	}

	/* 評価テーブルの読み込み */
	result = OpenEvalData("src\\eval.bin");
	if (result == FALSE)
	{
		return result;
	}

	///* MPCテーブルの読み込み */
	result = OpenMpcInfoData("src\\mpc.dat");

	return result;
}