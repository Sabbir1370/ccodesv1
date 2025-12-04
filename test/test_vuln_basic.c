// Buffer overflow vulnerabilities
#include <stdio.h>
#include <string.h>

// UNSAFE: Classic buffer overflow
void unsafe_strcpy_example()
{
    char buffer[10];
    char input[20] = "AAAAAAAAAAAAAAAAAAA";
    strcpy(buffer, input); // Buffer overflow!
}

// UNSAFE: gets() function
void unsafe_gets_example()
{
    char buffer[10];
    gets(buffer); // Never use gets()!
}

// SAFE: Using strncpy with bounds checking
void safe_strcpy_example()
{
    char buffer[10];
    char input[20] = "AAAAAAAAAAAAAAAAAAA";
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
}

int main()
{
    unsafe_strcpy_example();
    // unsafe_gets_example();  // Commented out for compilation
    safe_strcpy_example();
    return 0;
}