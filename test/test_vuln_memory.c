#include <stdlib.h>
#include <string.h>

// UNSAFE: Use after free
void use_after_free()
{
    char *ptr = (char *)malloc(10);
    strcpy(ptr, "hello");
    free(ptr);
    ptr[0] = 'x'; // Use after free
}

// UNSAFE: Double free
void double_free()
{
    char *ptr = (char *)malloc(10);
    free(ptr);
    free(ptr); // Double free
}

// UNSAFE: Memory leak
void memory_leak()
{
    char *ptr = (char *)malloc(100);
    // Forgot to free(ptr)
}

// UNSAFE: Uninitialized pointer
void uninitialized_pointer()
{
    int *ptr;
    *ptr = 10; // Using uninitialized pointer
}

// SAFE: Proper memory management
void safe_memory_use()
{
    char *ptr = (char *)malloc(10);
    if (ptr != NULL)
    {
        strcpy(ptr, "safe");
        // Use ptr...
        free(ptr);
        ptr = NULL; // Good practice
    }
}

int main()
{
    use_after_free();
    double_free();
    memory_leak();
    uninitialized_pointer();
    safe_memory_use();
    return 0;
}