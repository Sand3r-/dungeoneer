internal void
_AssertFailure(char *expression, int line, char *file, int crash)
{
    if(crash)
    {
        platform->OutputError("Assertion Failure",
                              "Assertion of %s at %s:%i failed. Trying to crash...",
                              expression, file, line);
        *(int *)0 = 0;
    }
    else
    {
        platform->OutputError("Assertion Failure",
                              "Soft assertion of %s at %s:%i failed.",
                              expression, file, line);
    }
}

internal void
_DebugLog(char *file, int line, char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "%s:%i - ", file, line);
    vfprintf(stdout, format, args);
    fprintf(stdout, "%s", "\n");
    va_end(args);
}
