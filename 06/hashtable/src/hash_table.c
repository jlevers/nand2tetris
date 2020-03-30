/*
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * Heavily influenced by https://github.com/jamesroutley/write-a-hash-table
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

ll_node LL_SENTINEL = {NULL, NULL};

static ht_item *ht_new_item(const char *k, const char *v) {
    ht_item *i = calloc(1, sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

static void ht_del_item(ht_item **i) {
    free((*i)->key);
    (*i)->key = NULL;
    free((*i)->value);
    (*i)->value = NULL;
    free(*i);
    *i = NULL;
}

/**** LINKED LIST FUNCTIONS ****/

/**
 * Creates a new linked list of `ht_item`s.
 * 
 * @return a pointer to a new, empty linked list
 */
ll_node *ll_new() {
    return &LL_SENTINEL;
}

/**
 * Adds an item to the head of the given linked list
 * 
 * @param list  the linked list to add to
 * @param key   the key to add to the linked list
 * @param value the value to add to the linked list
 * @return      a pointer to the new head of the linked list
 */
ll_node *ll_insert(ll_node *list, const char *key, const char *value) {
    ll_node *new_node = calloc(1, sizeof(ll_node));
    new_node->value = ht_new_item(key, value);
    new_node->next = list;
    return new_node;
}

/**
 * Searches recursively for a specific `ht_item` key in a linked list.
 * 
 * @param key     the key to search for
 * @param current the current linked list node being recursed over
 * @param prev    the last linked list node that was recursed over
 * @return        the corresponding value to `key`, or NULL
 */
static char *ll_search_recur(const char *key, const ll_node *current, const ll_node *prev) {
    if (current == &LL_SENTINEL) {
        return NULL;
    } else if (!strcmp(current->value->key, key)) {
        return strdup(current->value->value);
    } else {
        return ll_search_recur(key, current->next, current);
    }
}

/**
 * Searches a linked list of `ht_item`s for the given key.
 * 
 * @param node the linked list to search
 * @param key  the key to search for
 * @return     the value corresponding to `key`, or NULL
 */
char *ll_search(ll_node *node, const char *key) {
    return ll_search_recur(key, node, NULL);
}

static int ll_remove_recur(const char *key, ll_node **current, ll_node *prev) {
    if (*current == &LL_SENTINEL) {  // Key doesn't exist in list
        return 0;
    } else if (!strcmp((*current)->value->key, key)) {  // Found correct key
        ht_del_item(&((*current)->value));
        ll_node *next = (*current)->next;
        free(*current);
        *current = NULL;
        if (prev == NULL) {  // If first item in list
            *current = next;
        } else {
            prev->next = next;
        }
        return 1;
    } else {
        return ll_remove_recur(key, &((*current)->next), *current);
    }
}

/**
 * Removes the node from the linked list of `ht_item`s whose key is `key`.
 * 
 * @param node the linked list to remove a node from
 * @param key  the key to remove
 */
int ll_remove(ll_node **node, const char *key) {
    return ll_remove_recur(key, node, NULL);
}

/**
 * Deletes the given linked list.
 * 
 * @param node the linked list to delete
 */
void ll_delete(ll_node **node) {
    if (*node != &LL_SENTINEL) {
        ht_del_item(&((*node)->value));
        ll_delete(&((*node)->next));
        free(*node);
        *node = NULL;
    }
}

/**** HASH TABLE FUNCTIONS ****/

/**
 * Computes the FNV1a hash of the given input.
 * 
 * @param input the value to hash
 * @return      the hashed value of the input
 */
unsigned long long fnv1a(const char *input) {
    unsigned long long hash = HT_FNV_OFFSET_BASIS;
    for (int i = 0; i < strlen(input); i++) {
        hash ^= (int)input[i];
        hash *= HT_FNV_PRIME;
    }
    return hash;
}

static unsigned long long ht_hash(const char *input, const int size) {
    return fnv1a(input) % size;
}

/**
 * Creates a new hash table, with the given number of lists.
 * 
 * @param size the size of the hash table
 * @return     the new hash table
 */
ht_hash_table *ht_new(const int size) {
    ht_hash_table* ht = calloc(1, sizeof(ht_hash_table));
    ht->size = size;
    ht->count = 0;
    ht->nodes = calloc(size, sizeof(ll_node*));
    for (int i = 0; i < size; i++) {
        ht->nodes[i] = ll_new();
    }
    return ht;
}

/**
 * Inserts the given key/value pair into the hash table.
 * 
 * @param ht  the hash table to insert into
 * @param key the key to insert
 * @param val the value to insert
 */
void ht_insert(ht_hash_table *ht, const char *key, const char *val) {
    unsigned long long hash = ht_hash(key, ht->size);
    ht->nodes[hash] = ll_insert(ht->nodes[hash], key, val);
    ht->count++;
}

/**
 * Inserts multiple key/value pairs, pairing each item in the list of keys with the item in the list of values at
 * the same index. There must be the same number of keys and values, and there must be ae NULL sentinel value at the
 * end of both lists.
 * 
 * @param ht        the hash table to insert into
 * @param num_items the number of items to insert. NOTE: the NULL sentinel value at the end of each list doesn't count
 *                  towards this value
 * @param keys      the list of keys to insert, ending with a NULL sentinel value
 * @param vals      the list of values to insert relative to the keys, ending with a NULL sentinel value
 */
void ht_insert_all(ht_hash_table *ht, int num_items, const char *keys[], const char *vals[]) {
    const char *init_items_err =
        "[ERR] The list of keys and the list of values supplied to ht_insert_all() must both be non-NULL and "
        "both be the same length. %s, so no key-value pairs are being added to the hash table.\n";

    // There will be a segfault caused by accessing an invalid array index if num_items is more than the number of key/value
    // pairs given, or if different length key and value lists are given and num_items is the length of the longer one
    if (keys == NULL || vals == NULL) {
        printf(init_items_err, "Either the list of keys or the list of values was NULL");
    } else if (keys[num_items] != NULL || vals[num_items] != NULL) {
        printf(init_items_err,
            "Either the list of keys and of values are different lengths, or one of the lists doesn't end "
            "with a NULL sentinel value");
    } else {
        for (int i = 0; i < num_items; i++) {
            ht_insert(ht, keys[i], vals[i]);
        }
    }
}

/**
 * Searches the hash table for a value corresponding to the given key.
 * 
 * @param ht  the hash table to search
 * @param key the key to search for
 * @return    the value corresponding to the given key if one exists, NULL otherwise
 */
char *ht_search(ht_hash_table *ht, const char *key) {
    unsigned long long hash = ht_hash(key, ht->size);
    ll_node *node = ht->nodes[hash];
    return ll_search(node, key);
}

/**
 * Removes the key/value pair corresponding to the given key from the hash table, if it exists.
 * 
 * @param ht  the hash table to remove the key/value pair from
 * @param key removes the key/value pair corresponding to this key, if one exists
 */
void ht_remove(ht_hash_table *ht, const char *key) {
    unsigned long long hash = ht_hash(key, ht->size);
    int removed = ll_remove(&(ht->nodes[hash]), key);
    if (removed) ht->count--;
}

/**
 * Deletes the hash table, and all key/value pairs in it.
 *
 * @param ht the hash table to delete
 */
void ht_delete(ht_hash_table *ht) {
    for (int i = 0; i < ht->size; i++) {
        ll_delete(&(ht->nodes[i]));
    }
    free(ht->nodes);
    free(ht);
}
