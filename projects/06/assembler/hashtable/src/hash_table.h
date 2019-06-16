#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

// A key/value pair in a hash table
typedef struct {
    char* key;
    char* value;
} ht_item;

// A hash table
typedef struct {
    int base_size;
    int size;
    int count;
    ht_item **items;
} ht_hash_table;

ht_hash_table* ht_new();
void ht_insert(ht_hash_table*, const char*, const char*);
char* ht_search(ht_hash_table*, const char*);
void ht_delete(ht_hash_table*, const char*);

#endif
