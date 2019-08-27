#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <stdlib.h>

// A key/value pair in a hash table
typedef struct ht_item {
    char* key;
    char* value;
} ht_item;

// A node in a linked list of hash table items
typedef struct ll_node {
    ht_item *value;
    struct ll_node *next;
} ll_node;

// A hash table
typedef struct ht_hash_table {
    int size;
    int count;
    ll_node **nodes;
} ht_hash_table;

extern ll_node LL_SENTINEL;

static const unsigned long long HT_FNV_OFFSET_BASIS = 0xCBF29CE484222325U;
static const unsigned long long HT_FNV_PRIME = 0x100000001B3U;

/* Linked list functions */
ll_node *ll_new();
ll_node *ll_insert(ll_node*, const char*, const char*);
char *ll_search(ll_node*, const char*);
ll_node *ll_remove(ll_node*, const char*);
void ll_delete(ll_node*);

/* Hash table functions */
const unsigned long long fnv1a(const char*);
ht_hash_table *ht_new(const int);
void ht_insert(ht_hash_table*, const char*, const char*);
char *ht_search(ht_hash_table*, const char*);
void ht_remove(ht_hash_table*, const char*);
void ht_delete(ht_hash_table*);

#endif
