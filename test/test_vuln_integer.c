#include <limits.h>

// UNSAFE: Integer overflow
void integer_overflow()
{
    int x = INT_MAX;
    x = x + 1; // Integer overflow
}

// UNSAFE: Signed/unsigned mismatch
void signed_unsigned_mismatch()
{
    int signed_int = -1;
    unsigned int unsigned_int = 100;

    if (signed_int < unsigned_int)
    {
        // This might not work as expected!
    }
}

// UNSAFE: Division by zero potential
void division_by_zero(int divisor)
{
    int result = 100 / divisor; // Could be zero
}

// SAFE: Checked arithmetic
void safe_arithmetic()
{
    int x = INT_MAX;
    if (x < INT_MAX)
    {
        x = x + 1; // Safe
    }

    int divisor = 0;
    if (divisor != 0)
    {
        int result = 100 / divisor; // Safe
    }
}

int main()
{
    integer_overflow();
    signed_unsigned_mismatch();
    division_by_zero(0);
    safe_arithmetic();
    return 0;
}