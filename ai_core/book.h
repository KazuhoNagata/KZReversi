/***************************************************************************
* Name  : book.h
* Brief : 定石関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#define NOT_CHANGE 0
#define CHANGE_LITTLE 1
#define CHANGE_MIDDLE 2
#define CHANGE_RANDOM 3

typedef struct node
{
	struct node *child;
	struct node *next;
	UINT64 bk;
	UINT64 wh;
	int eval;
	short move;
	short depth;
}BooksNode;

extern BOOL g_book_done;
extern BooksNode g_bookTree;

/***************************************************************************
* Name  : GetMoveFromBooks
* Brief : 定石やからCPUの着手を決定する
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 GetMoveFromBooks(UINT64 bk, UINT64 wh, UINT32 color, UINT32 change, INT32 turn);
BOOL OpenBook(char *filename);
void BookFree(BooksNode *head);