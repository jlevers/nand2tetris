#include <stdio.h>
#include <string.h>

#include "minunit.h"
#include "hash_table.h"

int tests_run = 0;

static char *test_ll() {
    // Test ll_new()
    ll_node *ll = ll_new();
    mu_assert("new linked list is not just sentinel", ll == &LL_SENTINEL);

    // Test ll_insert()
    ll = ll_insert(ll, "abc", "def");
    ll = ll_insert(ll, "123", "456");
    mu_assert("first key of linked list should be \"123\"", !strcmp(ll->value->key, "123"));
    mu_assert("first value of linked list should be \"456\"", !strcmp(ll->value->value, "456"));
    mu_assert("second key of linked list should be \"abc\"", !strcmp(ll->next->value->key, "abc"));
    mu_assert("second value of linked list should be \"def\"", !strcmp(ll->next->value->value, "def"));

    // Test ll_search()
    char *abc = ll_search(ll, "abc");
    mu_assert("linked list should contain key \"abc\"", !strcmp(abc, "def"));
    mu_assert("linked list should not contain key \"test\"", ll_search(ll, "test") == NULL);
    free(abc);

    // Test ll_remove()
    ll_remove(&ll, "abc");
    mu_assert("linked list should not contain \"abc:def\"", ll->next == &LL_SENTINEL);
    mu_assert("linked list should still contain \"123:456\"", !strcmp(ll->value->key, "123") && !strcmp(ll->value->value, "456"));

    // Test ll_delete()
    // I'm really just testing this by making sure valgrind doesn't show a memory leak. I'm not sure
    // how to programmatically test if a set of pointers inside a struct got freed.
    ll_delete(&ll);
    return 0;
}

static char *test_ht() {
    // Test fnv1a()
    mu_assert("FNV1a hash of \"abc\" should be e71fa2190541574b", fnv1a("abc") == 0xe71fa2190541574b);
    mu_assert("FNV1a hash of \"123\" should be 456fc2181822c4db", fnv1a("123") == 0x456fc2181822c4db);

    // Test ht_new()
    ht_hash_table *ht = ht_new(2);
    mu_assert("hashtable ht size should be 2", ht->size == 2);
    mu_assert("hashtable ht count should be 2", ht->count == 0);

    // Test ht_insert()
    ht_insert(ht, "abc", "def");
    ht_insert(ht, "123", "456");
    ht_insert(ht, "qwer", "tyuiop");
    mu_assert("hash table should have key/value pair abc:def", !strcmp(ht->nodes[1]->next->value->value, "def"));
    mu_assert("hash table should have key/value pair 123:456", !strcmp(ht->nodes[1]->value->value, "456"));
    mu_assert("hash table should have key/value pair qwer:tyuiop", !strcmp(ht->nodes[0]->value->value, "tyuiop"));
    mu_assert("hash table count should be 3", ht->count == 3);

    // Test ht_insert_all()
    const char* keys1[] = {"initkey1", "initkey2", NULL};
    const char* vals1[] = {"initval1", "initval2", NULL};
    const char* keys2[] = {"no", NULL};
    const char* vals2[] = {"sentinel"};
    const char* keys3[] = {"different", "lengths", NULL};
    const char* vals3[] = {"this", "one's", "longer", NULL};
    ht_insert_all(ht, 2, keys1, vals1);
    mu_assert("hash table should have key/value pair initkey1:initval1", !strcmp(ht->nodes[1]->value->value, "initval1"));
    mu_assert("hash table should have key/value pair initkey1:initval1", !strcmp(ht->nodes[0]->value->value, "initval2"));
    mu_assert("hash table count should be 5", ht->count == 5);
    ht_insert_all(ht, 1, keys2, vals2);
    mu_assert("hash table count should still be 5", ht->count == 5);
    ht_insert_all(ht, 2, keys3, vals3);
    mu_assert("hash table count should still be 5", ht->count == 5);


    // Test ht_search()
    char *ht_search_init1 = ht_search(ht, "initkey1");
    char *ht_search_init2 = ht_search(ht, "initkey2");
    char *ht_search_abc = ht_search(ht, "abc");
    char *ht_search_123 = ht_search(ht, "123");
    char *ht_search_qwer = ht_search(ht, "qwer");
    mu_assert("hash table search should not find value for key \"foo\"", ht_search(ht, "foo") == NULL);
    mu_assert("hash table search should find value \"initval1\" for key \"initkey1\"", !strcmp(ht_search_init1, "initval1"));
    mu_assert("hash table search should find value \"initval2\" for key \"initkey2\"", !strcmp(ht_search_init2, "initval2"));
    mu_assert("hash table search should find value \"def\" for key \"abc\"", !strcmp(ht_search_abc, "def"));
    mu_assert("hash table search should find value \"456\" for key \"123\"", !strcmp(ht_search_123, "456"));
    mu_assert("hash table search should find value \"tyuiop\" for key \"qwer\"", !strcmp(ht_search_qwer, "tyuiop"));
    free(ht_search_init1);
    free(ht_search_init2);
    free(ht_search_abc);
    free(ht_search_123);
    free(ht_search_qwer);

    // Test ht_remove()
    ht_remove(ht, "abc");
    mu_assert("hash table should no longer contain key/value pair abc:def", ht->nodes[1]->next->next == &LL_SENTINEL);
    ht_remove(ht, "qwer");
    mu_assert("hash table should no longer contain key/value pair qwer:tyuiop", ht->nodes[0]->next == &LL_SENTINEL);
    mu_assert("hash table count should be 3", ht->count == 3);
    ht_remove(ht, "foo");
    mu_assert("hash table count should still be 3", ht->count == 3);

    // Test ht_delete()
    // I'm really just testing this by making sure valgrind doesn't show a memory leak. I'm not sure
    // how to programmatically test if a set of pointers inside a struct got freed.
    ht_delete(ht);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_ll);
    mu_run_test(test_ht);
    return 0;
}

int main(int argc, char **argv) {
    printf("[TEST] Begin test output\n");
    printf("------------------------\n\n");

    char *result = all_tests();
    if (result != 0) {
        printf("[TEST] %s\n", result);
    } else {
        printf("\n[TEST] ALL TESTS PASSED\n");
    }

    printf("[TEST] Tests run: %d\n", tests_run);
    printf("\n----------------------\n");
    printf("[TEST] End test output\n");

    return result != 0;
}
