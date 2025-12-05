// test_format_comprehensive.c - Test all format string vulnerabilities
#include <stdio.h>
#include <string.h>

void safe_examples()
{
    // These should NOT trigger warnings
    printf("Hello World\n");
    printf("Number: %d\n", 42);
    printf("Name: %s, Age: %d\n", "Alice", 25);
    printf("Percent: %%\n");
    printf("Float: %.2f\n", 3.14159);

    // With width/precision
    printf("%10d\n", 123);
    printf("%.3f\n", 1.23456);

    // Multiple specifiers
    printf("%d %s %c %f\n", 1, "test", 'A', 2.5);
}

void unsafe_examples()
{
    char buffer[100];
    char user_input[100];
    int x = 10;

    // 1. Missing arguments (HIGH severity)
    printf("Missing int: %d\n");            // Line 23
    printf("Missing string: %s\n");         // Line 24
    printf("Multiple missing: %d %s %f\n"); // Line 25

    // 2. Extra arguments (HIGH severity)
    printf("Extra: %d\n", x, x, x); // Line 28

    // 3. Non-constant format strings (CRITICAL)
    printf(buffer); // Line 31
    strcpy(buffer, "Value: %d");
    printf(buffer); // Line 33

    // 4. User-controlled format strings (CRITICAL)
    scanf("%99s", user_input);
    printf(user_input); // Line 37

    // 5. Wrong argument types (would need type checking)
    printf("String as int: %d\n", "not a number"); // Line 40
}

void edge_cases()
{
    // These test edge cases
    printf("%%");               // OK: Just percent sign
    printf("%%%%");             // OK: Multiple percents
    printf("Test %% %d\n", 42); // OK: Mixed

    // Empty or whitespace
    printf("");    // OK: Empty string
    printf("   "); // OK: Whitespace

    // Newlines and special chars
    printf("Line1\nLine2\n"); // OK
    printf("Tab\tHere\n");    // OK
}

int main()
{
    printf("=== Safe Examples ===\n");
    safe_examples();

    printf("\n=== Unsafe Examples ===\n");
    unsafe_examples();

    printf("\n=== Edge Cases ===\n");
    edge_cases();

    return 0;
}