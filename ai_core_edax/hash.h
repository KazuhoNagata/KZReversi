/**
 * @file hash.h
 *
 * Hash table's header.
 *
 * @date 1998 - 2017
 * @author Richard Delorme
 * @version 4.4
 */

#include "stdafx.h"


#pragma once

#include "board.h"
#include "util.h"

#define HASH_COLLISIONS(x)
#define HASH_STATS(x)

#define HASH_N_WAY 4

 /** hash align */
#define HASH_ALIGNED 1

/** Allow type puning */
#ifndef USE_TYPE_PUNING
#ifdef ANDROID
#define USE_TYPE_PUNING false
#else
#define USE_TYPE_PUNING true
#endif
#endif

#define PREPARE_LOCKED 3
#define LOCKED 2

// middle, endgame attribute
#define HASH_ATTR_MIDDLE 0
#define HASH_ATTR_WLD    1
#define HASH_ATTR_EXACT  2

/** HashData : data stored in the hash table */
typedef struct HashData {
	unsigned char depth;      /*!< depth */
	unsigned char selectivity;/*!< selectivity */
	unsigned char cost;       /*!< search cost */
	unsigned char date;       /*!< dating technique */
	signed char lower;        /*!< lower bound of the position score */
	signed char upper;        /*!< upper bound of the position score */
	unsigned char move[2];    /*!< best moves */
} HashData;

/** Hash  : item stored in the hash table*/
typedef struct Hash {
	HASH_COLLISIONS(unsigned long long key;)
		Board board;
	HashData data;
} Hash;

/** HashLock : lock for table entries */
typedef struct HashLock {
	SpinLock spin;
} HashLock;

/** HashTable: position storage */
typedef struct HashTable {
	void *memory;                 /*!< allocated memory */
	Hash *hash;  				  /*!< hash table */
	HashLock *lock;               /*!< table with locks */
	unsigned long long hash_mask; /*!< a bit mask for hash entries */
	unsigned int lock_mask;       /*!< a bit mask for lock entries */
	int n_hash;                   /*!< hash table size */
	int n_lock;                   /*!< number of locks */
	unsigned char date;           /*!< date */
} HashTable;

/* declaration */
void hash_code_init(void);
void hash_move_init(void);
void hash_init(HashTable*, const unsigned long long);
void hash_cleanup(HashTable*);
void hash_clear(HashTable*);
void hash_free(HashTable*);
void hash_store(HashTable*, const Board*, const unsigned long long, const int, const int, const int, const int, const int, const int, const int);
void hash_force(HashTable*, const Board*, const unsigned long long, const int, const int, const int, const int, const int, const int, const int);
bool hash_get(HashTable*, const Board*, const unsigned long long, HashData *);
void hash_copy(const HashTable*, HashTable*);
void hash_print(const HashData*, FILE*);
void hash_feed(HashTable*, const Board*, const unsigned long long, const int, const int, const int, const int, const int);
void hash_exclude_move(HashTable*, const Board*, const unsigned long long, const int);
extern unsigned int writeable_level(HashData *data);

extern const HashData HASH_DATA_INIT;
extern unsigned long long hash_rank[16][256];
extern unsigned long long hash_move[64][60];
