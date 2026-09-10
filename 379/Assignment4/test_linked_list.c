#define UNIT_TEST
#include "linked_list.c"
#include <assert.h>

void test_insertFirst_empty_list() {
    // Ensure list is empty
    head = NULL;

    insertFirst(1, 10);

    assert(head != NULL);
    assert(head->key == 1);
    assert(head->data == 10);
    assert(head->next == NULL);

    // Clean up
    free(head);
    head = NULL;
    printf("test_insertFirst_empty_list passed\n");
}

void test_insertFirst_non_empty_list() {
    // Ensure list is empty
    head = NULL;

    insertFirst(1, 10);
    struct node *first_node = head;

    insertFirst(2, 20);

    assert(head != NULL);
    assert(head != first_node);
    assert(head->key == 2);
    assert(head->data == 20);
    assert(head->next == first_node);
    assert(head->next->key == 1);
    assert(head->next->next == NULL);

    // Clean up
    free(head->next);
    free(head);
    head = NULL;
    printf("test_insertFirst_non_empty_list passed\n");
}

int main() {
    test_insertFirst_empty_list();
    test_insertFirst_non_empty_list();
    printf("All tests passed!\n");
    return 0;
}
