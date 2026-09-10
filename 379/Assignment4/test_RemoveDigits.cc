#define UNIT_TEST
#include "a4tasks.cc"
#include <assert.h>
#include <string.h>
#include <iostream>

void test_RemoveDigits() {
    char buf[100];

    // Case 1: Empty string
    strcpy(buf, "");
    assert(strcmp(RemoveDigits(buf), "") == 0);
    std::cout << "Test case 1 passed: Empty string" << std::endl;

    // Case 2: No digits
    strcpy(buf, "Hello World!");
    assert(strcmp(RemoveDigits(buf), "Hello World!") == 0);
    std::cout << "Test case 2 passed: No digits" << std::endl;

    // Case 3: Only digits
    strcpy(buf, "1234567890");
    assert(strcmp(RemoveDigits(buf), "") == 0);
    std::cout << "Test case 3 passed: Only digits" << std::endl;

    // Case 4: Mixed characters
    strcpy(buf, "abc123def456");
    assert(strcmp(RemoveDigits(buf), "abcdef") == 0);
    std::cout << "Test case 4 passed: Mixed characters" << std::endl;

    // Case 5: Spaces and digits
    strcpy(buf, "a 1 b 2 c 3");
    assert(strcmp(RemoveDigits(buf), "a  b  c ") == 0);
    std::cout << "Test case 5 passed: Spaces and digits" << std::endl;

    // Case 6: Special characters and digits
    strcpy(buf, "!@#1$2%3^");
    assert(strcmp(RemoveDigits(buf), "!@#$%^") == 0);
    std::cout << "Test case 6 passed: Special characters and digits" << std::endl;

    std::cout << "All RemoveDigits tests passed!" << std::endl;
}

int main() {
    test_RemoveDigits();
    return 0;
}
