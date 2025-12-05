// test_fmt_simple.c
#include <stdio.h>

int main()
{
    // Should trigger FMT001: Missing argument
    printf("Test: %d\n");
    return 0;
}
