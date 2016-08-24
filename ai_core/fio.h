/***************************************************************************
* Name  : fio.h
* Brief : ファイルIO関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

typedef struct {
	int code;
	int codeSize;
}CodeInfo;

typedef struct{
	char chr;
	int occurrence;
	int parent;
	int left;
	int right;
}TreeNode;

UCHAR *DecodeBookData(INT64 *decodeDataLen_p, char *filename);
UCHAR *DecodeEvalData(INT64 *decodeDataLen_p, char *filename);
BOOL OpenMpcInfoData(MPCINFO *p_info, INT32 info_len, char *filename);