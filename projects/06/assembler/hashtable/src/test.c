#include <stdio.h>
#include <string.h>

#include "minunit.h"
#include "hash_table.h"

int tests_run = 0;

static char *test_ll() {
    ll_node *ll = ll_new();
    mu_assert("new linked list is not just sentinel", ll == &LL_SENTINEL);

    ll = ll_insert(ll, "abc", "def");
    ll = ll_insert(ll, "123", "456");

    mu_assert("first key of linked list should be \"123\"", !strcmp(ll->value->key, "123"));
    mu_assert("first value of linked list should be \"456\"", !strcmp(ll->value->value, "456"));
    mu_assert("second key of linked list should be \"abc\"", !strcmp(ll->next->value->key, "abc"));
    mu_assert("second value of linked list should be \"def\"", !strcmp(ll->next->value->value, "def"));

    mu_assert("linked list should contain key \"abc\"", !strcmp(ll_search(ll, "abc"), "def"));
    mu_assert("linked list should not contain key \"test\"", ll_search(ll, "test") == NULL);

    ll_remove(ll, "abc");

    mu_assert("linked list should not contain \"abc:def\"", ll->next == &LL_SENTINEL);
    mu_assert("linked list should still contain \"123:456\"", !strcmp(ll->value->key, "123") && !strcmp(ll->value->value, "456"));


    ll_delete(&ll);

    // This doesn't really test that everything is freed, I'm not sure how to do that
    mu_assert("linked list should not exist anymore", ll == NULL);
    return 0;
}

static char *test_ht() {
    ht_hash_table *ht = ht_new(2);
    mu_assert("hashtable size should be 2", ht->size == 2);
    mu_assert("hashtable count should be 0", ht->count == 0);

    ht_delete(ht);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_ll);

    mu_run_test(test_ht);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
