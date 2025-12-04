/* Buffer overflow vulnerabilities */

/* UNSAFE: Classic buffer overflow */
void unsafe_strcpy_example()
{
    // nothing

    char buffer[10];
    char input[20] = "AAAAAAAAAAAAAAAAAAA";
    strcpy(buffer, input); /* Buffer overflow! */
}