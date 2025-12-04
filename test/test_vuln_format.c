#include <stdio.h>

// UNSAFE: User-controlled format string
void unsafe_printf(char *user_input)
{
    printf(user_input); // Format string vulnerability!
}

// UNSAFE: Missing format arguments
void missing_format_args()
{
    printf("%s %s", "hello"); // Missing argument
}

// SAFE: Constant format string
void safe_printf()
{
    printf("%s\n", "Safe format string");
}

// UNSAFE: sprintf without bounds checking
void unsafe_sprintf(char *input)
{
    char buffer[50];
    sprintf(buffer, "Input: %s", input); // Could overflow
}

// SAFE: Use snprintf
void safe_sprintf(char *input)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Input: %s", input);
}

int main()
{
    char user_input[100] = "%x %x %x";
    unsafe_printf(user_input);
    missing_format_args();
    safe_printf();
    unsafe_sprintf("Very long input that might overflow buffer");
    safe_sprintf("Safe input");
    return 0;
}