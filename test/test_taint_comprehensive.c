// test/test_taint_comprehensive.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test 1: Basic taint flow
void test_basic()
{
    char input[100], buffer[100];
    scanf("%s", input);
    strcpy(buffer, input); // Should detect
}

// Test 2: Pointer taint
void test_pointer()
{
    char data[100], *ptr, dest[100];
    gets(data);
    ptr = data;
    strcpy(dest, ptr); // Should detect through pointer
}

// Test 3: Multiple propagations
void test_propagation()
{
    char a[100], b[100], c[100];
    fgets(a, sizeof(a), stdin);
    strcpy(b, a);
    strcpy(c, b); // Should detect chain
}

// Test 4: Safe code (no taint)
void test_safe()
{
    char safe[100] = "constant";
    char buffer[100];
    strcpy(buffer, safe); // Should NOT detect
}

// Test 5: System command injection
void test_command()
{
    char cmd[200], input[100];
    printf("Enter: ");
    fgets(input, sizeof(input), stdin);
    sprintf(cmd, "ls %s", input); // Should detect
    // system(cmd);  // Would be critical
}

int main()
{
    test_basic();
    test_pointer();
    test_propagation();
    test_safe();
    test_command();
    return 0;
}