#include <stdio.h>
#include <string.h>

int main()
{
    char buffer[10];
    char input[20] = "This is too long!";

    // This should trigger SecureMemTracker if enabled
    strcpy(buffer, input);

    printf("Result: %s\n", buffer);
    return 0;
}