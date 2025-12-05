// test_all_detectors.c - Comprehensive test for all detectors
#include <stdio.h>
#include <string.h>

// Test 1: Format string vulnerabilities (FMT001)
void test_format_strings()
{
    printf("=== Testing Format String Vulnerabilities ===\n");

    // Should be SAFE (no warnings):
    printf("Hello World\n");
    printf("Number: %d\n", 42);
    printf("Name: %s, Age: %d\n", "Alice", 25);
    printf("Escaped: %%\n");

    // Should trigger FMT001 warnings:
    printf("Missing arg: %d\n");         // Line 13: Missing argument
    printf("Extra args: %d\n", 1, 2, 3); // Line 14: Extra arguments
    printf("Wrong count: %d %s\n", 42);  // Line 15: Missing string arg

    char user_input[100];
    printf(user_input); // Line 18: Variable format - CRITICAL

    char fmt[] = "Value: %d";
    printf(fmt, 100); // Line 21: Variable format - CRITICAL
}

// Test 2: Buffer overflow (SecureMemTracker)
void test_buffer_overflows()
{
    printf("\n=== Testing Buffer Overflows ===\n");

    char small[10];
    char large[100] = "This is a very long string that will overflow the buffer";

    // Should trigger SecureMemTracker:
    strcpy(small, large);        // Line 30: Buffer overflow
    strcat(small, " more text"); // Line 31: Buffer overflow

    // Safer alternatives (should not trigger):
    strncpy(small, large, sizeof(small) - 1);
    small[sizeof(small) - 1] = '\0';
}

// Test 3: Use before initialization (for future detector)
void test_uninitialized()
{
    printf("\n=== Testing Uninitialized Variables ===\n");

    int uninit_int;
    char uninit_buf[50];

    printf("Uninit int: %d\n", uninit_int); // Line 43: Use before init
    strcpy(uninit_buf, "test");             // Line 44: Use before init
}

// Test 4: Taint analysis (for TaintFlowDetector)
void test_taint()
{
    printf("\n=== Testing Taint Analysis ===\n");

    char input[100];
    char buffer[50];

    // Simulate user input
    scanf("%s", input); // Line 52: Tainted source

    // Should trigger taint analysis:
    strcpy(buffer, input);          // Line 55: Tainted flow
    printf("Output: %s\n", buffer); // Line 56: Tainted sink
}

int main()
{
    test_format_strings();
    test_buffer_overflows();
    test_uninitialized();
    test_taint();

    return 0;
}