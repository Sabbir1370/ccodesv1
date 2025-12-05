// test_buffer_fixed.c
#include <stdio.h>
#include <string.h>

void buffer_test1()
{
    char small[10];
    char large[100] = "This is a very long string";
    int i;

    strcpy(small, large); // Line 9: Buffer overflow

    int arr[5];
    for (i = 0; i < 10; i++)
    {               // Line 13: Potential overflow
        arr[i] = i; // Line 14
    }
}

void buffer_test2()
{
    int buffer[10];
    int *ptr = buffer;

    ptr += 15; // Line 20: Pointer arithmetic
}

int main()
{
    buffer_test1();
    buffer_test2();
    return 0;
}