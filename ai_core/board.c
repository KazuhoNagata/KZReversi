/***************************************************************************
* Name  : board.cpp
* Brief : 盤面関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include "board.h"
#include "eval.h"
#include "bit64.h"

#ifdef HAS_CPU_64
#define	packA1A8(X)	((((X) & 0x0101010101010101ULL) * 0x0102040810204080ULL) >> 56)
#define	packH1H8(X)	((((X) & 0x8080808080808080ULL) * 0x0002040810204081ULL) >> 56)
#else
#define	packA1A8(X)	(((((unsigned int)(X) & 0x01010101u) + (((unsigned int)((X) >> 32) & 0x01010101u) << 4)) * 0x01020408u) >> 24)
#define	packH1H8(X)	(((((unsigned int)((X) >> 32) & 0x80808080u) + (((unsigned int)(X) & 0x80808080u) >> 4)) * 0x00204081u) >> 24)
#endif

UCHAR g_board[64];

UINT8 board_parity[] =
{
	0, 0, 0, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 1, 1, 1, 1,
	2, 2, 2, 2, 3, 3, 3, 3,
	2, 2, 2, 2, 3, 3, 3, 3,
	2, 2, 2, 2, 3, 3, 3, 3,
	2, 2, 2, 2, 3, 3, 3, 3
};

UINT8 board_parity_bit[] =
{
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8
};

UINT64 quad_parity_bitmask[] = 
{
	0x000000000F0F0F0FULL,
	0x00000000F0F0F0F0ULL,
	0x0F0F0F0F00000000ULL,
	0xF0F0F0F000000000ULL
};

/** threshold values to try stability cutoff during NWS search */
// TODO: better values may exist.
const INT32 NWS_STABILITY_THRESHOLD[] = { // 99 = unused value...
	99, 99, 99, 99, 6, 8, 10, 12,
	14, 16, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46,
	48, 48, 50, 50, 52, 52, 54, 54,
	56, 56, 58, 58, 60, 60, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64,
	99, 99, 99, 99, 99, 99, 99, 99, // no stable square at those depths
};

/** threshold values to try stability cutoff during PVS search */
// TODO: better values may exist.
const INT32 PVS_STABILITY_THRESHOLD[] = { // 99 = unused value...
	99, 99, 99, 99, -2, 0, 2, 4,
	6, 8, 12, 14, 16, 18, 20, 22,
	24, 26, 28, 30, 32, 34, 36, 38,
	40, 40, 42, 42, 44, 44, 46, 46,
	48, 48, 50, 50, 52, 52, 54, 54,
	56, 56, 58, 58, 60, 60, 62, 62,
	99, 99, 99, 99, 99, 99, 99, 99, // no stable square at those depths
};

/** edge stability global data */
static unsigned char edge_stability[256][256];

/** conversion from an 8-bit line to the A1-A8 line */
static const unsigned long long A1_A8[256] = {
	0x0000000000000000ULL, 0x0000000000000001ULL, 0x0000000000000100ULL, 0x0000000000000101ULL, 0x0000000000010000ULL, 0x0000000000010001ULL, 0x0000000000010100ULL, 0x0000000000010101ULL,
	0x0000000001000000ULL, 0x0000000001000001ULL, 0x0000000001000100ULL, 0x0000000001000101ULL, 0x0000000001010000ULL, 0x0000000001010001ULL, 0x0000000001010100ULL, 0x0000000001010101ULL,
	0x0000000100000000ULL, 0x0000000100000001ULL, 0x0000000100000100ULL, 0x0000000100000101ULL, 0x0000000100010000ULL, 0x0000000100010001ULL, 0x0000000100010100ULL, 0x0000000100010101ULL,
	0x0000000101000000ULL, 0x0000000101000001ULL, 0x0000000101000100ULL, 0x0000000101000101ULL, 0x0000000101010000ULL, 0x0000000101010001ULL, 0x0000000101010100ULL, 0x0000000101010101ULL,
	0x0000010000000000ULL, 0x0000010000000001ULL, 0x0000010000000100ULL, 0x0000010000000101ULL, 0x0000010000010000ULL, 0x0000010000010001ULL, 0x0000010000010100ULL, 0x0000010000010101ULL,
	0x0000010001000000ULL, 0x0000010001000001ULL, 0x0000010001000100ULL, 0x0000010001000101ULL, 0x0000010001010000ULL, 0x0000010001010001ULL, 0x0000010001010100ULL, 0x0000010001010101ULL,
	0x0000010100000000ULL, 0x0000010100000001ULL, 0x0000010100000100ULL, 0x0000010100000101ULL, 0x0000010100010000ULL, 0x0000010100010001ULL, 0x0000010100010100ULL, 0x0000010100010101ULL,
	0x0000010101000000ULL, 0x0000010101000001ULL, 0x0000010101000100ULL, 0x0000010101000101ULL, 0x0000010101010000ULL, 0x0000010101010001ULL, 0x0000010101010100ULL, 0x0000010101010101ULL,
	0x0001000000000000ULL, 0x0001000000000001ULL, 0x0001000000000100ULL, 0x0001000000000101ULL, 0x0001000000010000ULL, 0x0001000000010001ULL, 0x0001000000010100ULL, 0x0001000000010101ULL,
	0x0001000001000000ULL, 0x0001000001000001ULL, 0x0001000001000100ULL, 0x0001000001000101ULL, 0x0001000001010000ULL, 0x0001000001010001ULL, 0x0001000001010100ULL, 0x0001000001010101ULL,
	0x0001000100000000ULL, 0x0001000100000001ULL, 0x0001000100000100ULL, 0x0001000100000101ULL, 0x0001000100010000ULL, 0x0001000100010001ULL, 0x0001000100010100ULL, 0x0001000100010101ULL,
	0x0001000101000000ULL, 0x0001000101000001ULL, 0x0001000101000100ULL, 0x0001000101000101ULL, 0x0001000101010000ULL, 0x0001000101010001ULL, 0x0001000101010100ULL, 0x0001000101010101ULL,
	0x0001010000000000ULL, 0x0001010000000001ULL, 0x0001010000000100ULL, 0x0001010000000101ULL, 0x0001010000010000ULL, 0x0001010000010001ULL, 0x0001010000010100ULL, 0x0001010000010101ULL,
	0x0001010001000000ULL, 0x0001010001000001ULL, 0x0001010001000100ULL, 0x0001010001000101ULL, 0x0001010001010000ULL, 0x0001010001010001ULL, 0x0001010001010100ULL, 0x0001010001010101ULL,
	0x0001010100000000ULL, 0x0001010100000001ULL, 0x0001010100000100ULL, 0x0001010100000101ULL, 0x0001010100010000ULL, 0x0001010100010001ULL, 0x0001010100010100ULL, 0x0001010100010101ULL,
	0x0001010101000000ULL, 0x0001010101000001ULL, 0x0001010101000100ULL, 0x0001010101000101ULL, 0x0001010101010000ULL, 0x0001010101010001ULL, 0x0001010101010100ULL, 0x0001010101010101ULL,
	0x0100000000000000ULL, 0x0100000000000001ULL, 0x0100000000000100ULL, 0x0100000000000101ULL, 0x0100000000010000ULL, 0x0100000000010001ULL, 0x0100000000010100ULL, 0x0100000000010101ULL,
	0x0100000001000000ULL, 0x0100000001000001ULL, 0x0100000001000100ULL, 0x0100000001000101ULL, 0x0100000001010000ULL, 0x0100000001010001ULL, 0x0100000001010100ULL, 0x0100000001010101ULL,
	0x0100000100000000ULL, 0x0100000100000001ULL, 0x0100000100000100ULL, 0x0100000100000101ULL, 0x0100000100010000ULL, 0x0100000100010001ULL, 0x0100000100010100ULL, 0x0100000100010101ULL,
	0x0100000101000000ULL, 0x0100000101000001ULL, 0x0100000101000100ULL, 0x0100000101000101ULL, 0x0100000101010000ULL, 0x0100000101010001ULL, 0x0100000101010100ULL, 0x0100000101010101ULL,
	0x0100010000000000ULL, 0x0100010000000001ULL, 0x0100010000000100ULL, 0x0100010000000101ULL, 0x0100010000010000ULL, 0x0100010000010001ULL, 0x0100010000010100ULL, 0x0100010000010101ULL,
	0x0100010001000000ULL, 0x0100010001000001ULL, 0x0100010001000100ULL, 0x0100010001000101ULL, 0x0100010001010000ULL, 0x0100010001010001ULL, 0x0100010001010100ULL, 0x0100010001010101ULL,
	0x0100010100000000ULL, 0x0100010100000001ULL, 0x0100010100000100ULL, 0x0100010100000101ULL, 0x0100010100010000ULL, 0x0100010100010001ULL, 0x0100010100010100ULL, 0x0100010100010101ULL,
	0x0100010101000000ULL, 0x0100010101000001ULL, 0x0100010101000100ULL, 0x0100010101000101ULL, 0x0100010101010000ULL, 0x0100010101010001ULL, 0x0100010101010100ULL, 0x0100010101010101ULL,
	0x0101000000000000ULL, 0x0101000000000001ULL, 0x0101000000000100ULL, 0x0101000000000101ULL, 0x0101000000010000ULL, 0x0101000000010001ULL, 0x0101000000010100ULL, 0x0101000000010101ULL,
	0x0101000001000000ULL, 0x0101000001000001ULL, 0x0101000001000100ULL, 0x0101000001000101ULL, 0x0101000001010000ULL, 0x0101000001010001ULL, 0x0101000001010100ULL, 0x0101000001010101ULL,
	0x0101000100000000ULL, 0x0101000100000001ULL, 0x0101000100000100ULL, 0x0101000100000101ULL, 0x0101000100010000ULL, 0x0101000100010001ULL, 0x0101000100010100ULL, 0x0101000100010101ULL,
	0x0101000101000000ULL, 0x0101000101000001ULL, 0x0101000101000100ULL, 0x0101000101000101ULL, 0x0101000101010000ULL, 0x0101000101010001ULL, 0x0101000101010100ULL, 0x0101000101010101ULL,
	0x0101010000000000ULL, 0x0101010000000001ULL, 0x0101010000000100ULL, 0x0101010000000101ULL, 0x0101010000010000ULL, 0x0101010000010001ULL, 0x0101010000010100ULL, 0x0101010000010101ULL,
	0x0101010001000000ULL, 0x0101010001000001ULL, 0x0101010001000100ULL, 0x0101010001000101ULL, 0x0101010001010000ULL, 0x0101010001010001ULL, 0x0101010001010100ULL, 0x0101010001010101ULL,
	0x0101010100000000ULL, 0x0101010100000001ULL, 0x0101010100000100ULL, 0x0101010100000101ULL, 0x0101010100010000ULL, 0x0101010100010001ULL, 0x0101010100010100ULL, 0x0101010100010101ULL,
	0x0101010101000000ULL, 0x0101010101000001ULL, 0x0101010101000100ULL, 0x0101010101000101ULL, 0x0101010101010000ULL, 0x0101010101010001ULL, 0x0101010101010100ULL, 0x0101010101010101ULL,
};

/** conversion from an 8-bit line to the H1-H8 line */
static const unsigned long long H1_H8[256] = {
	0x0000000000000000ULL, 0x0000000000000080ULL, 0x0000000000008000ULL, 0x0000000000008080ULL, 0x0000000000800000ULL, 0x0000000000800080ULL, 0x0000000000808000ULL, 0x0000000000808080ULL,
	0x0000000080000000ULL, 0x0000000080000080ULL, 0x0000000080008000ULL, 0x0000000080008080ULL, 0x0000000080800000ULL, 0x0000000080800080ULL, 0x0000000080808000ULL, 0x0000000080808080ULL,
	0x0000008000000000ULL, 0x0000008000000080ULL, 0x0000008000008000ULL, 0x0000008000008080ULL, 0x0000008000800000ULL, 0x0000008000800080ULL, 0x0000008000808000ULL, 0x0000008000808080ULL,
	0x0000008080000000ULL, 0x0000008080000080ULL, 0x0000008080008000ULL, 0x0000008080008080ULL, 0x0000008080800000ULL, 0x0000008080800080ULL, 0x0000008080808000ULL, 0x0000008080808080ULL,
	0x0000800000000000ULL, 0x0000800000000080ULL, 0x0000800000008000ULL, 0x0000800000008080ULL, 0x0000800000800000ULL, 0x0000800000800080ULL, 0x0000800000808000ULL, 0x0000800000808080ULL,
	0x0000800080000000ULL, 0x0000800080000080ULL, 0x0000800080008000ULL, 0x0000800080008080ULL, 0x0000800080800000ULL, 0x0000800080800080ULL, 0x0000800080808000ULL, 0x0000800080808080ULL,
	0x0000808000000000ULL, 0x0000808000000080ULL, 0x0000808000008000ULL, 0x0000808000008080ULL, 0x0000808000800000ULL, 0x0000808000800080ULL, 0x0000808000808000ULL, 0x0000808000808080ULL,
	0x0000808080000000ULL, 0x0000808080000080ULL, 0x0000808080008000ULL, 0x0000808080008080ULL, 0x0000808080800000ULL, 0x0000808080800080ULL, 0x0000808080808000ULL, 0x0000808080808080ULL,
	0x0080000000000000ULL, 0x0080000000000080ULL, 0x0080000000008000ULL, 0x0080000000008080ULL, 0x0080000000800000ULL, 0x0080000000800080ULL, 0x0080000000808000ULL, 0x0080000000808080ULL,
	0x0080000080000000ULL, 0x0080000080000080ULL, 0x0080000080008000ULL, 0x0080000080008080ULL, 0x0080000080800000ULL, 0x0080000080800080ULL, 0x0080000080808000ULL, 0x0080000080808080ULL,
	0x0080008000000000ULL, 0x0080008000000080ULL, 0x0080008000008000ULL, 0x0080008000008080ULL, 0x0080008000800000ULL, 0x0080008000800080ULL, 0x0080008000808000ULL, 0x0080008000808080ULL,
	0x0080008080000000ULL, 0x0080008080000080ULL, 0x0080008080008000ULL, 0x0080008080008080ULL, 0x0080008080800000ULL, 0x0080008080800080ULL, 0x0080008080808000ULL, 0x0080008080808080ULL,
	0x0080800000000000ULL, 0x0080800000000080ULL, 0x0080800000008000ULL, 0x0080800000008080ULL, 0x0080800000800000ULL, 0x0080800000800080ULL, 0x0080800000808000ULL, 0x0080800000808080ULL,
	0x0080800080000000ULL, 0x0080800080000080ULL, 0x0080800080008000ULL, 0x0080800080008080ULL, 0x0080800080800000ULL, 0x0080800080800080ULL, 0x0080800080808000ULL, 0x0080800080808080ULL,
	0x0080808000000000ULL, 0x0080808000000080ULL, 0x0080808000008000ULL, 0x0080808000008080ULL, 0x0080808000800000ULL, 0x0080808000800080ULL, 0x0080808000808000ULL, 0x0080808000808080ULL,
	0x0080808080000000ULL, 0x0080808080000080ULL, 0x0080808080008000ULL, 0x0080808080008080ULL, 0x0080808080800000ULL, 0x0080808080800080ULL, 0x0080808080808000ULL, 0x0080808080808080ULL,
	0x8000000000000000ULL, 0x8000000000000080ULL, 0x8000000000008000ULL, 0x8000000000008080ULL, 0x8000000000800000ULL, 0x8000000000800080ULL, 0x8000000000808000ULL, 0x8000000000808080ULL,
	0x8000000080000000ULL, 0x8000000080000080ULL, 0x8000000080008000ULL, 0x8000000080008080ULL, 0x8000000080800000ULL, 0x8000000080800080ULL, 0x8000000080808000ULL, 0x8000000080808080ULL,
	0x8000008000000000ULL, 0x8000008000000080ULL, 0x8000008000008000ULL, 0x8000008000008080ULL, 0x8000008000800000ULL, 0x8000008000800080ULL, 0x8000008000808000ULL, 0x8000008000808080ULL,
	0x8000008080000000ULL, 0x8000008080000080ULL, 0x8000008080008000ULL, 0x8000008080008080ULL, 0x8000008080800000ULL, 0x8000008080800080ULL, 0x8000008080808000ULL, 0x8000008080808080ULL,
	0x8000800000000000ULL, 0x8000800000000080ULL, 0x8000800000008000ULL, 0x8000800000008080ULL, 0x8000800000800000ULL, 0x8000800000800080ULL, 0x8000800000808000ULL, 0x8000800000808080ULL,
	0x8000800080000000ULL, 0x8000800080000080ULL, 0x8000800080008000ULL, 0x8000800080008080ULL, 0x8000800080800000ULL, 0x8000800080800080ULL, 0x8000800080808000ULL, 0x8000800080808080ULL,
	0x8000808000000000ULL, 0x8000808000000080ULL, 0x8000808000008000ULL, 0x8000808000008080ULL, 0x8000808000800000ULL, 0x8000808000800080ULL, 0x8000808000808000ULL, 0x8000808000808080ULL,
	0x8000808080000000ULL, 0x8000808080000080ULL, 0x8000808080008000ULL, 0x8000808080008080ULL, 0x8000808080800000ULL, 0x8000808080800080ULL, 0x8000808080808000ULL, 0x8000808080808080ULL,
	0x8080000000000000ULL, 0x8080000000000080ULL, 0x8080000000008000ULL, 0x8080000000008080ULL, 0x8080000000800000ULL, 0x8080000000800080ULL, 0x8080000000808000ULL, 0x8080000000808080ULL,
	0x8080000080000000ULL, 0x8080000080000080ULL, 0x8080000080008000ULL, 0x8080000080008080ULL, 0x8080000080800000ULL, 0x8080000080800080ULL, 0x8080000080808000ULL, 0x8080000080808080ULL,
	0x8080008000000000ULL, 0x8080008000000080ULL, 0x8080008000008000ULL, 0x8080008000008080ULL, 0x8080008000800000ULL, 0x8080008000800080ULL, 0x8080008000808000ULL, 0x8080008000808080ULL,
	0x8080008080000000ULL, 0x8080008080000080ULL, 0x8080008080008000ULL, 0x8080008080008080ULL, 0x8080008080800000ULL, 0x8080008080800080ULL, 0x8080008080808000ULL, 0x8080008080808080ULL,
	0x8080800000000000ULL, 0x8080800000000080ULL, 0x8080800000008000ULL, 0x8080800000008080ULL, 0x8080800000800000ULL, 0x8080800000800080ULL, 0x8080800000808000ULL, 0x8080800000808080ULL,
	0x8080800080000000ULL, 0x8080800080000080ULL, 0x8080800080008000ULL, 0x8080800080008080ULL, 0x8080800080800000ULL, 0x8080800080800080ULL, 0x8080800080808000ULL, 0x8080800080808080ULL,
	0x8080808000000000ULL, 0x8080808000000080ULL, 0x8080808000008000ULL, 0x8080808000008080ULL, 0x8080808000800000ULL, 0x8080808000800080ULL, 0x8080808000808000ULL, 0x8080808000808080ULL,
	0x8080808080000000ULL, 0x8080808080000080ULL, 0x8080808080008000ULL, 0x8080808080008080ULL, 0x8080808080800000ULL, 0x8080808080800080ULL, 0x8080808080808000ULL, 0x8080808080808080ULL,
};


/**
* @brief search stable edge patterns.
*
* Compute a 8-bit bitboard where each stable square is set to one
*
* @param old_P previous player edge discs.
* @param old_O previous opponent edge discs.
* @param stable 8-bit bitboard with stable edge squares.
*/
static int find_edge_stable(const long long old_P, const long long old_O, int stable)
{
	register long long P, O, x, y;
	const long long E = ~(old_P | old_O); // empties

	stable &= old_P; // mask stable squares with remaining player squares.
	if (!stable || E == 0) return stable;

	for (x = 0; x < 8; ++x) {
		if (E & x_to_bit(x)) { //is x an empty square ?
			O = old_O;
			P = old_P | x_to_bit(x); // player plays on it
			if (x > 1) { // flip left discs
				for (y = x - 1; y > 0 && (O & x_to_bit(y)); --y);
				if (P & x_to_bit(y)) {
					for (y = x - 1; y > 0 && (O & x_to_bit(y)); --y) {
						O ^= x_to_bit(y); P ^= x_to_bit(y);
					}
				}
			}
			if (x < 6) { // flip right discs
				for (y = x + 1; y < 8 && (O & x_to_bit(y)); ++y);
				if (P & x_to_bit(y)) {
					for (y = x + 1; y < 8 && (O & x_to_bit(y)); ++y) {
						O ^= x_to_bit(y); P ^= x_to_bit(y);
					}
				}
			}
			stable = find_edge_stable(P, O, stable); // next move
			if (!stable) return stable;

			P = old_P;
			O = old_O | x_to_bit(x); // opponent plays on it
			if (x > 1) {
				for (y = x - 1; y > 0 && (P & x_to_bit(y)); --y);
				if (O & x_to_bit(y)) {
					for (y = x - 1; y > 0 && (P & x_to_bit(y)); --y) {
						O ^= x_to_bit(y); P ^= x_to_bit(y);
					}
				}
			}
			if (x < 6) {
				for (y = x + 1; y < 8 && (P & x_to_bit(y)); ++y);
				if (O & x_to_bit(y)) {
					for (y = x + 1; y < 8 && (P & x_to_bit(y)); ++y) {
						O ^= x_to_bit(y); P ^= x_to_bit(y);
					}
				}
			}
			stable = find_edge_stable(P, O, stable); // next move
			if (!stable) return stable;
		}
	}

	return stable;
}

/**
* @brief Initialize the edge stability tables.
*/
void edge_stability_init(void)
{
	int P, O;

	for (P = 0; P < 256; ++P)
		for (O = 0; O < 256; ++O) {
			if (P & O) { // illegal positions
				edge_stability[P][O] = 0;
			}
			else {
				edge_stability[P][O] = find_edge_stable(P, O, P);
			}
		}
}

/***************************************************************************
* Name  : Swap
* Brief : 盤面情報（ビット列）の白黒反転
* Args  : *bk        : 黒のビット列
*         *wh        : 白のビット列
****************************************************************************/
void Swap(UINT64 *bk, UINT64 *wh)
{
	UINT64 temp = *bk;
	*bk = *wh;
	*wh = temp;
}

/***************************************************************************
* Name  : InitIndexBoard
* Brief : 盤面情報（ビット列）から配列に変換
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
****************************************************************************/
void InitIndexBoard(UINT64 bk, UINT64 wh)
{
	g_board[0] = (int)(bk & a1) + (int)((wh & a1) * 2);
	g_board[1] = (int)((bk & a2) >> 1) + (int)((wh & a2));
	g_board[2] = (int)((bk & a3) >> 2) + (int)((wh & a3) >> 1);
	g_board[3] = (int)((bk & a4) >> 3) + (int)((wh & a4) >> 2);
	g_board[4] = (int)((bk & a5) >> 4) + (int)((wh & a5) >> 3);
	g_board[5] = (int)((bk & a6) >> 5) + (int)((wh & a6) >> 4);
	g_board[6] = (int)((bk & a7) >> 6) + (int)((wh & a7) >> 5);
	g_board[7] = (int)((bk & a8) >> 7) + (int)((wh & a8) >> 6);
	g_board[8] = (int)((bk & b1) >> 8) + (int)((wh & b1) >> 7);
	g_board[9] = (int)((bk & b2) >> 9) + (int)((wh & b2) >> 8);
	g_board[10] = (int)((bk & b3) >> 10) + (int)((wh & b3) >> 9);
	g_board[11] = (int)((bk & b4) >> 11) + (int)((wh & b4) >> 10);
	g_board[12] = (int)((bk & b5) >> 12) + (int)((wh & b5) >> 11);
	g_board[13] = (int)((bk & b6) >> 13) + (int)((wh & b6) >> 12);
	g_board[14] = (int)((bk & b7) >> 14) + (int)((wh & b7) >> 13);
	g_board[15] = (int)((bk & b8) >> 15) + (int)((wh & b8) >> 14);
	g_board[16] = (int)((bk & c1) >> 16) + (int)((wh & c1) >> 15);
	g_board[17] = (int)((bk & c2) >> 17) + (int)((wh & c2) >> 16);
	g_board[18] = (int)((bk & c3) >> 18) + (int)((wh & c3) >> 17);
	g_board[19] = (int)((bk & c4) >> 19) + (int)((wh & c4) >> 18);
	g_board[20] = (int)((bk & c5) >> 20) + (int)((wh & c5) >> 19);
	g_board[21] = (int)((bk & c6) >> 21) + (int)((wh & c6) >> 20);
	g_board[22] = (int)((bk & c7) >> 22) + (int)((wh & c7) >> 21);
	g_board[23] = (int)((bk & c8) >> 23) + (int)((wh & c8) >> 22);
	g_board[24] = (int)((bk & d1) >> 24) + (int)((wh & d1) >> 23);
	g_board[25] = (int)((bk & d2) >> 25) + (int)((wh & d2) >> 24);
	g_board[26] = (int)((bk & d3) >> 26) + (int)((wh & d3) >> 25);
	g_board[27] = (int)((bk & d4) >> 27) + (int)((wh & d4) >> 26);
	g_board[28] = (int)((bk & d5) >> 28) + (int)((wh & d5) >> 27);
	g_board[29] = (int)((bk & d6) >> 29) + (int)((wh & d6) >> 28);
	g_board[30] = (int)((bk & d7) >> 30) + (int)((wh & d7) >> 29);
	g_board[31] = (int)((bk & d8) >> 31) + (int)((wh & d8) >> 30);
	g_board[32] = (int)((bk & e1) >> 32) + (int)((wh & e1) >> 31);
	g_board[33] = (int)((bk & e2) >> 33) + (int)((wh & e2) >> 32);
	g_board[34] = (int)((bk & e3) >> 34) + (int)((wh & e3) >> 33);
	g_board[35] = (int)((bk & e4) >> 35) + (int)((wh & e4) >> 34);
	g_board[36] = (int)((bk & e5) >> 36) + (int)((wh & e5) >> 35);
	g_board[37] = (int)((bk & e6) >> 37) + (int)((wh & e6) >> 36);
	g_board[38] = (int)((bk & e7) >> 38) + (int)((wh & e7) >> 37);
	g_board[39] = (int)((bk & e8) >> 39) + (int)((wh & e8) >> 38);
	g_board[40] = (int)((bk & f1) >> 40) + (int)((wh & f1) >> 39);
	g_board[41] = (int)((bk & f2) >> 41) + (int)((wh & f2) >> 40);
	g_board[42] = (int)((bk & f3) >> 42) + (int)((wh & f3) >> 41);
	g_board[43] = (int)((bk & f4) >> 43) + (int)((wh & f4) >> 42);
	g_board[44] = (int)((bk & f5) >> 44) + (int)((wh & f5) >> 43);
	g_board[45] = (int)((bk & f6) >> 45) + (int)((wh & f6) >> 44);
	g_board[46] = (int)((bk & f7) >> 46) + (int)((wh & f7) >> 45);
	g_board[47] = (int)((bk & f8) >> 47) + (int)((wh & f8) >> 46);
	g_board[48] = (int)((bk & g1) >> 48) + (int)((wh & g1) >> 47);
	g_board[49] = (int)((bk & g2) >> 49) + (int)((wh & g2) >> 48);
	g_board[50] = (int)((bk & g3) >> 50) + (int)((wh & g3) >> 49);
	g_board[51] = (int)((bk & g4) >> 51) + (int)((wh & g4) >> 50);
	g_board[52] = (int)((bk & g5) >> 52) + (int)((wh & g5) >> 51);
	g_board[53] = (int)((bk & g6) >> 53) + (int)((wh & g6) >> 52);
	g_board[54] = (int)((bk & g7) >> 54) + (int)((wh & g7) >> 53);
	g_board[55] = (int)((bk & g8) >> 55) + (int)((wh & g8) >> 54);
	g_board[56] = (int)((bk & h1) >> 56) + (int)((wh & h1) >> 55);
	g_board[57] = (int)((bk & h2) >> 57) + (int)((wh & h2) >> 56);
	g_board[58] = (int)((bk & h3) >> 58) + (int)((wh & h3) >> 57);
	g_board[59] = (int)((bk & h4) >> 59) + (int)((wh & h4) >> 58);
	g_board[60] = (int)((bk & h5) >> 60) + (int)((wh & h5) >> 59);
	g_board[61] = (int)((bk & h6) >> 61) + (int)((wh & h6) >> 60);
	g_board[62] = (int)((bk & h7) >> 62) + (int)((wh & h7) >> 61);
	g_board[63] = (int)((bk & h8) >> 63) + (int)((wh & h8) >> 62);
}

/***************************************************************************
* Name  : create_empty_list
* Brief : 空きマス＋偶奇情報のリストを作成する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
****************************************************************************/
void create_empty_list(EmptyList *start, UINT64 blank)
{
	EmptyList *list = start + 1, *previous = start;

	while (blank)
	{
		list->empty.position = CountBit((~blank) & (blank - 1));
		list->empty.parity = board_parity[list->empty.position];
		previous = previous->next = list;
		blank ^= 1ULL << list->empty.position;
		list++;
	}
	previous->next = NULL;
}

/**
* @brief Get stable edge.
*
* @param P bitboard with player's discs.
* @param O bitboard with opponent's discs.
* @return a bitboard with (some of) player's stable discs.
*
*/
static unsigned long long get_stable_edge(const unsigned long long P, const unsigned long long O)
{
	// compute the exact stable edges (from precomputed tables)
	return edge_stability[P & 0xff][O & 0xff]
		| ((unsigned long long)edge_stability[P >> 56][O >> 56]) << 56
		| A1_A8[edge_stability[packA1A8(P)][packA1A8(O)]]
		| H1_H8[edge_stability[packH1H8(P)][packH1H8(O)]];
}

/**
* @brief Estimate the stability of edges.
*
* Count the number (in fact a lower estimate) of stable discs on the edges.
*
* @param P bitboard with player's discs.
* @param O bitboard with opponent's discs.
* @return the number of stable discs on the edges.
*/
int get_edge_stability(const unsigned long long P, const unsigned long long O)
{
	return CountBit(get_stable_edge(P, O));
}


/**
* @brief Get full lines.
*
* @param line all discs on a line.
* @param dir tested direction
* @return a bitboard with full lines along the tested direction.
*/
static unsigned long long get_full_lines(const unsigned long long line, const int dir)
{
	// sequential algorithm
	// 6 << + 6 >> + 12 & + 5 |
	register unsigned long long full;
	const unsigned long long edge = line & 0xff818181818181ffULL;

	full = (line & (((line >> dir) & (line << dir)) | edge));
	full &= (((full >> dir) & (full << dir)) | edge);
	full &= (((full >> dir) & (full << dir)) | edge);
	full &= (((full >> dir) & (full << dir)) | edge);
	full &= (((full >> dir) & (full << dir)) | edge);

	return ((full >> dir) & (full << dir));
}

/**
* @brief Estimate the stability.
*
* Count the number (in fact a lower estimate) of stable discs.
*
* @param P bitboard with player's discs.
* @param O bitboard with opponent's discs.
* @return the number of stable discs.
*/
int get_stability(const unsigned long long P, const unsigned long long O)
{
	const unsigned long long disc = (P | O);
	const unsigned long long central_mask = (P & 0x007e7e7e7e7e7e00ULL);
	const unsigned long long full_h = get_full_lines(disc, 1);
	const unsigned long long full_v = get_full_lines(disc, 8);
	const unsigned long long full_d7 = get_full_lines(disc, 7);
	const unsigned long long full_d9 = get_full_lines(disc, 9);
	register unsigned long long stable_h, stable_v, stable_d7, stable_d9, stable, new_stable;

	// compute the exact stable edges (from precomputed tables)
	new_stable = get_stable_edge(P, O);

	// add full lines
	new_stable |= (full_h & full_v & full_d7 & full_d9 & central_mask);

	// now compute the other stable discs (ie discs touching another stable disc in each flipping direction).
	stable = 0;
	while (new_stable & ~stable) {
		stable |= new_stable;
		stable_h = ((stable >> 1) | (stable << 1) | full_h);
		stable_v = ((stable >> 8) | (stable << 8) | full_v);
		stable_d7 = ((stable >> 7) | (stable << 7) | full_d7);
		stable_d9 = ((stable >> 9) | (stable << 9) | full_d9);
		new_stable = (stable_h & stable_v & stable_d7 & stable_d9 & central_mask);
	}

	return CountBit(stable);
}

void create_quad_parity(UINT32 *q_parity, UINT64 blank)
{
	for (int i = 0; i < 4; i++)
		q_parity[i] = CountBit(blank & quad_parity_bitmask[i]) % 2;
}

/*
############################################################################
#
#  局面の対称変換関数
#  (ひとつの局面には、回転・対称の関係にある7つの同一局面が含まれている)
#
############################################################################
*/
UINT64 rotate_90(UINT64 board)
{
	/* 反時計回り90度の回転 */
	board = board << 1 & 0xaa00aa00aa00aa00 |
		board >> 1 & 0x0055005500550055 |
		board >> 8 & 0x00aa00aa00aa00aa |
		board << 8 & 0x5500550055005500;

	board = board << 2 & 0xcccc0000cccc0000 |
		board >> 2 & 0x0000333300003333 |
		board >> 16 & 0x0000cccc0000cccc |
		board << 16 & 0x3333000033330000;

	board = board << 4 & 0xf0f0f0f000000000 |
		board >> 4 & 0x000000000f0f0f0f |
		board >> 32 & 0x00000000f0f0f0f0 |
		board << 32 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 rotate_180(UINT64 board)
{
	/* 反時計回り180度の回転 */
	board = board << 9 & 0xaa00aa00aa00aa00 |
		board >> 9 & 0x0055005500550055 |
		board >> 7 & 0x00aa00aa00aa00aa |
		board << 7 & 0x5500550055005500;

	board = board << 18 & 0xcccc0000cccc0000 |
		board >> 18 & 0x0000333300003333 |
		board >> 14 & 0x0000cccc0000cccc |
		board << 14 & 0x3333000033330000;

	board = board << 36 & 0xf0f0f0f000000000 |
		board >> 36 & 0x000000000f0f0f0f |
		board >> 28 & 0x00000000f0f0f0f0 |
		board << 28 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 rotate_270(UINT64 board)
{
	/* 反時計回り270度の回転=時計回り90度の回転 */
	board = board << 8 & 0xaa00aa00aa00aa00 |
		board >> 8 & 0x0055005500550055 |
		board << 1 & 0x00aa00aa00aa00aa |
		board >> 1 & 0x5500550055005500;

	board = board << 16 & 0xcccc0000cccc0000 |
		board >> 16 & 0x0000333300003333 |
		board << 2 & 0x0000cccc0000cccc |
		board >> 2 & 0x3333000033330000;

	board = board << 32 & 0xf0f0f0f000000000 |
		board >> 32 & 0x000000000f0f0f0f |
		board << 4 & 0x00000000f0f0f0f0 |
		board >> 4 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 symmetry_x(UINT64 board)
{
	/* X軸に対し対称変換 */
	board = board << 1 & 0xaaaaaaaaaaaaaaaa |
		board >> 1 & 0x5555555555555555;

	board = board << 2 & 0xcccccccccccccccc |
		board >> 2 & 0x3333333333333333;

	board = board << 4 & 0xf0f0f0f0f0f0f0f0 |
		board >> 4 & 0x0f0f0f0f0f0f0f0f;

	return board;
}

UINT64 symmetry_y(UINT64 board)
{
	/* Y軸に対し対称変換 */
	board = board << 8 & 0xff00ff00ff00ff00 |
		board >> 8 & 0x00ff00ff00ff00ff;

	board = board << 16 & 0xffff0000ffff0000 |
		board >> 16 & 0x0000ffff0000ffff;

	board = board << 32 & 0xffffffff00000000 |
		board >> 32 & 0x00000000ffffffff;

	return board;
}

UINT64 symmetry_b(UINT64 board)
{
	/* ブラックラインに対し対象変換 */
	board = board >> 9 & 0x0055005500550055 |
		board << 9 & 0xaa00aa00aa00aa00 |
		board & 0x55aa55aa55aa55aa;

	board = board >> 18 & 0x0000333300003333 |
		board << 18 & 0xcccc0000cccc0000 |
		board & 0x3333cccc3333cccc;

	board = board >> 36 & 0x000000000f0f0f0f |
		board << 36 & 0xf0f0f0f000000000 |
		board & 0x0f0f0f0ff0f0f0f0;

	return board;
}

UINT64 symmetry_w(UINT64 board)
{
	/* ホワイトラインに対し対象変換 */
	board = board >> 7 & 0x00aa00aa00aa00aa |
		board << 7 & 0x5500550055005500 |
		board & 0xaa55aa55aa55aa55;

	board = board >> 14 & 0x0000cccc0000cccc |
		board << 14 & 0x3333000033330000 |
		board & 0xcccc3333cccc3333;

	board = board >> 28 & 0x00000000f0f0f0f0 |
		board << 28 & 0x0f0f0f0f00000000 |
		board & 0xf0f0f0f00f0f0f0f;

	return board;
}