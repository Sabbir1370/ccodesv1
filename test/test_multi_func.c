#include <stdio.h>

void func1() {
    printf("Missing: %d\n");  // Line 4
}

void func2() {
    printf("OK: %d\n", 42);   // Line 8
}

int main() {
    func1();
    func2();
    return 0;
}
