// Simple function
int simple_return()
{
    return 42;
}

// Function with if/else
int conditional(int x)
{
    if (x > 0)
    {
        return x + 1;
    }
    else
    {
        return x - 1;
    }
}

// Function with while loop
int while_loop(int n)
{
    int sum = 0;
    int i = 0;
    while (i < n)
    {
        sum += i;
        i = i + 1;
    }
    return sum;
}

// Function with multiple statements
int complex_function(int a, int b)
{
    int result = 0;

    if (a > b)
    {
        result = a - b;
    }
    else
    {
        result = b - a;
    }

    int i = 0;
    while (i < result)
    {
        result = result - 1;
        i = i + 1;
    }

    return result;
}