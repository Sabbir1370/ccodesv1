// test_phase_g_complete.c
#include <stdio.h>
#include <string.h>

// Test 1: Format string (FMT001)
void test_format()
{
    printf("Missing: %d\n"); // Line 6: FMT001
    char buf[100];
    printf(buf); // Line 8: FMT001 CRITICAL
}

// Test 2: Uninitialized (INIT001)
void test_uninit()
{
    int x;
    printf("%d\n", x); // Line 13: INIT001
}

// Test 3: Buffer/unsafe (MEM001)
void test_buffer()
{
    char dest[10];
    strcpy(dest, "too long"); // Line 18: MEM001
}

// Test 4: Taint (TAINT001 would need user input)
void test_taint()
{
    char input[100];
    scanf("%s", input); // Line 23: TAINT source
    printf(input);      // Line 24: TAINT sink + FMT001
}

int main()
{
    test_format();
    test_uninit();
    test_buffer();
    test_taint();
    return 0;
}