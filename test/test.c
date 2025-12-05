#include <stdio.h>
#include <string.h>

void test()
{
    char input[100];
    char buffer[100];

    scanf("%s", input);    // Should taint 'input'
    strcpy(buffer, input); // Should detect taint flow
}

int main()
{
    test();
    return 0;
}