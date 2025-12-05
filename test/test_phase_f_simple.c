#include <stdio.h>
#include <string.h>

// Test 1: Simple taint flow (scanf -> strcpy)
void test1() {
    char input[100];
    char buffer[100];
    
    scanf("%s", input);      // Taint source
    strcpy(buffer, input);   // Taint sink
}

// Test 2: Pointer taint
void test2() {
    char data[100];
    char* ptr;
    
    gets(data);              // Taint source
    ptr = data;              // Taint propagation
    char dest[100];
    strcpy(dest, ptr);       // Taint sink
}

// Test 3: Multiple propagations
void test3() {
    char a[100], b[100], c[100];
    
    fgets(a, sizeof(a), stdin);  // Taint source
    strcpy(b, a);                // Propagation
    strcpy(c, b);                // Taint sink
}

// Test 4: Safe code (control)
void test4() {
    char safe[100] = "safe";
    char buffer[100];
    strcpy(buffer, safe);    // No taint
}

int main() {
    test1();
    test2();
    test3();
    test4();
    return 0;
}
