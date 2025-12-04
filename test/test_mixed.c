// Mixed safe and unsafe patterns for comprehensive testing
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global variables
int global_uninitialized; // Uninitialized global
int global_initialized = 0;

// Function with potential issues
void problematic_function(char *input)
{
    // UNSAFE: No bounds checking
    char local_buffer[20];
    strcpy(local_buffer, input);

    // UNSAFE: Potential integer overflow
    int length = strlen(input);
    char *heap_buffer = malloc(length); // No +1 for null terminator

    // SAFE: After fixing
    if (heap_buffer)
    {
        strncpy(heap_buffer, input, length);
        heap_buffer[length] = '\0'; // Might be out of bounds
        free(heap_buffer);
    }
}

// Safer version
void safer_function(const char *input)
{
    size_t length = strlen(input);
    char *buffer = malloc(length + 1);

    if (buffer)
    {
        strcpy(buffer, input); // Safe now due to +1
        printf("Copied: %s\n", buffer);
        free(buffer);
    }
}

// Uninitialized variable usage
void use_before_init()
{
    int x;         // Uninitialized
    int y = x + 5; // Using uninitialized variable
}

// Safe initialization
void safe_init()
{
    int x = 0; // Properly initialized
    int y = x + 5;
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        problematic_function(argv[1]);
        safer_function(argv[1]);
    }

    use_before_init();
    safe_init();

    return 0;
}