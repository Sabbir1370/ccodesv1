// test/test_unsafe.c - Vulnerable code
#include <string.h>
#include <stdio.h>

void vulnerable_function()
{
    char buffer[10];

    // Unsafe strcpy
    strcpy(buffer, "This is too long for buffer");

    // Unsafe gets
    gets(buffer);

    // Unsafe sprintf
    sprintf(buffer, "Format string %s", "too long");
}

void safe_function()
{
    char buffer[20];

    // Safe usage with bounds checking
    strncpy(buffer, "Safe string", sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Safe alternative to gets
    fgets(buffer, sizeof(buffer), stdin);
}

int main()
{
    vulnerable_function();
    safe_function();
    return 0;
}