// test/test_taint.c
#include <stdio.h>
#include <string.h>

// Test 1: Simple taint flow
void test_taint_flow()
{
    char buffer[100];
    char user_input[200];

    // Taint source
    scanf("%s", user_input); // user_input becomes tainted

    // Taint propagation
    char *data = user_input; // data becomes tainted

    // Taint sink
    strcpy(buffer, data); // CRITICAL: Tainted data flows to strcpy
}

// Test 2: Taint through pointer
void test_taint_pointer()
{
    char *ptr;
    char input[100];

    fgets(input, sizeof(input), stdin); // input tainted
    ptr = input;                        // ptr tainted

    char dest[50];
    sprintf(dest, "Data: %s", ptr); // CRITICAL: Tainted format string
}

// Test 3: Taint with conditionals
void test_taint_conditional()
{
    char safe[100] = "safe";
    char unsafe[100];
    char output[200];

    gets(unsafe); // unsafe tainted

    char *data;
    if (rand() % 2)
    {
        data = safe;
    }
    else
    {
        data = unsafe; // data might be tainted
    }

    strcpy(output, data); // POTENTIAL: Conditional taint flow
}

// Test 4: No taint (safe)
void test_no_taint()
{
    char buffer[100];
    const char *safe = "constant";

    strcpy(buffer, safe); // SAFE: No taint
}

// Test 5: Taint in loop
void test_taint_loop()
{
    char inputs[10][100];
    char output[100];

    for (int i = 0; i < 10; i++)
    {
        scanf("%s", inputs[i]); // Each input tainted
    }

    // Use tainted input
    strcpy(output, inputs[5]); // CRITICAL: Tainted
}

// Test 6: System command injection
void test_command_injection()
{
    char command[200];
    char user_cmd[100];

    printf("Enter command: ");
    gets(user_cmd); // user_cmd tainted

    sprintf(command, "ls %s", user_cmd); // Tainted command construction
    system(command);                     // CRITICAL: Command injection
}

int main()
{
    test_taint_flow();
    test_taint_pointer();
    test_taint_conditional();
    test_no_taint();
    test_taint_loop();
    test_command_injection();
    return 0;
}