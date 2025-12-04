// Examples of safe C code patterns
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// SAFE: Bounded string copy
void safe_string_copy(char *dest, size_t dest_size, const char *src)
{
    if (dest == NULL || src == NULL)
        return;
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// SAFE: Checked memory allocation
void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// SAFE: Input validation
int safe_division(int numerator, int denominator)
{
    if (denominator == 0)
    {
        fprintf(stderr, "Division by zero attempted\n");
        return 0;
    }
    return numerator / denominator;
}

// SAFE: Loop with bounds checking
void safe_array_access(int *array, size_t size, size_t index)
{
    if (index < size)
    {
        array[index] = 42;
    }
    else
    {
        fprintf(stderr, "Array index out of bounds\n");
    }
}

// SAFE: Null termination guarantee
void null_terminate_string(char *str, size_t size)
{
    if (size > 0)
    {
        str[size - 1] = '\0';
    }
}

int main()
{
    char buffer[20];
    safe_string_copy(buffer, sizeof(buffer), "Hello World");

    int *arr = (int *)safe_malloc(10 * sizeof(int));
    safe_array_access(arr, 10, 5);

    int result = safe_division(100, 5);
    printf("Result: %d\n", result);

    free(arr);
    return 0;
}