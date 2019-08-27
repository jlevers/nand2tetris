/*
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * Inspired by, and nearly identical to, https://github.com/jamesroutley/write-a-hash-table
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

ll_node LL_SENTINEL = {NULL, NULL};

static ht_item *ht_new_item(const char *k, const char *v) {
    ht_item *i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

static void ht_del_item(ht_item *i) {
    free(i->key);
    free(i->value);
    free(i);
}

/**** LINKED LIST FUNCTIONS ****/

/**
 * Creates a new linked list of `ht_item`s.
 * @return a pointer to a new, empty linked list
 */
ll_node *ll_new() {
    return &LL_SENTINEL;
}

/**
 * Adds an item to the head of the given linked list
 * @param  list  the linked list to add to
 * @param  key   the key to add to the linked list
 * @param  value the value to add to the linked list
 * @return       a pointer to the new head of the linked list
 */
ll_node *ll_insert(struct ll_node *list, const char *key, const char *value) {
    ll_node *new = malloc(sizeof(ll_node));
    new->value = ht_new_item(key, value);
    new->next = list;
    return new;
}

/**
 * Searches recursively for a specific `ht_item` key in a linked list
 * @param  key     the key to search for
 * @param  current the current linked list node being recursed over
 * @param  prev    the last linked list node that was recursed over
 * @return         the corresponding value to `key`, or NULL
 */
static char *ll_search_recur(const char *key, ll_node *current, ll_node *prev) {
    if (current == &LL_SENTINEL) {
        return NULL;
    } else if (strcmp(current->value->key, key) == 0) {
        return current->value->value;
    } else {
        return ll_search_recur(key, current->next, current);
    }
}

/**
 * Searches a linked list of `ht_item`s for the given key
 * @param  node the linked list to search
 * @param  key  the key to search for
 * @return      the value corresponding to `key`, or NULL
 */
char *ll_search(ll_node *node, const char *key) {
    return ll_search_recur(key, node, NULL);
}

static ll_node *ll_remove_recur(const char *key, ll_node *current, ll_node *prev) {
    if (current == &LL_SENTINEL) {  // Key doesn't exist in list
        return current;
    } else if (strcmp(current->value->key, key) == 0) {  // Found correct key
        ht_del_item(current->value);
        ll_node *head;
        if (prev == NULL) {  // If first item in list
            head = current->next;
        } else {
            prev->next = current->next;
            head = NULL;
        }
        free(current);
        current = NULL;
        return head;
    } else {
        ll_remove_recur(key, current->next, current);
        return current;
    }
}

/**
 * Removes the node from the linked list of `ht_item`s whose key is `key`
 * @param node the linked list to remove a node from
 * @param key  the key to remove
 */
ll_node *ll_remove(ll_node *node, const char *key) {
    return ll_remove_recur(key, node, NULL);
}

/**
 * Deletes the given linked list
 * @param node the linked list to delete
 */
void ll_delete(ll_node *node) {
    if (node != &LL_SENTINEL) {
        ht_del_item(node->value);
        ll_delete(node->next);
        free(node);
    }
}

/**** HASH TABLE FUNCTIONS ****/

/**
 * Computes the FNV1a hash of the given input
 * @param  input the value to hash
 * @return       the hashed value of the input
 */
const unsigned long long fnv1a(const char *input) {
    unsigned long long hash = HT_FNV_OFFSET_BASIS;
    for (int i = 0; i < strlen(input); i++) {
        hash ^= (int)input[i];
        hash *= HT_FNV_PRIME;
    }
    return hash;
}

static const unsigned long long ht_hash(const char *input, const int size) {
    return fnv1a(input) % size;
}

ht_hash_table *ht_new(const int size) {
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    ht->size = size;
    ht->count = 0;
    ht->nodes = calloc(size, sizeof(ll_node*));
    for (int i = 0; i < size; i++) {
        ht->nodes[i] = ll_new();
    }
    return ht;
}

void ht_insert(ht_hash_table* ht, const char *key, const char *val) {
    unsigned long long hash = ht_hash(key, ht->size);
    ht->nodes[hash] = ll_insert(ht->nodes[hash], key, val);
    ht->count++;
}

char *ht_search(ht_hash_table *ht, const char *key) {
    unsigned long long hash = ht_hash(key, ht->size);
    ll_node *node = ht->nodes[hash];
    return ll_search(node, key);
}

void ht_remove(ht_hash_table *ht, const char *key) {
    unsigned long long hash = ht_hash(key, ht->size);
    ht->nodes[hash] = ll_remove(ht->nodes[hash], key);
    ht->count--;
}

void ht_delete(ht_hash_table *ht) {
    for (int i = 0; i < ht->size; i++) {
        ll_delete(ht->nodes[i]);
    }
    free(ht->nodes);
    free(ht);
}
