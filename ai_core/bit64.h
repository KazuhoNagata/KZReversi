/***************************************************************************
* Name  : bit64.h
* Brief : ビット演算関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

extern const unsigned long long X_TO_BIT[];

#define x_to_bit(x) X_TO_BIT[x]

/* 着手可能数計算用 */
typedef struct {
	unsigned long high;
	unsigned long low;
} st_bit;

typedef int(*BIT_MOB)(st_bit, st_bit, UINT64 *);

BOOL AlocMobilityFunc(void);
/***************************************************************************
* Name  : CountBit
* Brief : ビット列から１が立っているビットの数を数える
* Return: １が立っているビット数
****************************************************************************/
INT32 CountBit(UINT64 bit);