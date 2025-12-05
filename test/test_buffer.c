// test_buffer.c
#include <stdio.h>
#include <string.h>

void buffer_test1()
{
    char small[10];
    char large[100] = "This is a very long string";

    // Should be caught by SecureMemTracker, but BUF001 might also flag it
    strcpy(small, large); // Line 8

    // Array loop
    int arr[5];
    for (int i = 0; i < 10; i++)
    {               // Line 12: Potential overflow
        arr[i] = i; // Line 13: arr[5-9] out of bounds
    }
}

void buffer_test2()
{
    // Pointer operations
    int buffer[10];
    int *ptr = buffer;

    // Potential issues
    ptr += 15;                             // Line 20: Pointer arithmetic
    memcpy(buffer, ptr, sizeof(int) * 20); // Line 21: Potential overflow
}

int main()
{
    buffer_test1();
    buffer_test2();
    return 0;
}