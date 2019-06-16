#undef Assert
#define AssertStatement HardAssert
#define Assert HardAssert
#define HardAssert(b) if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 1); }
#define SoftAssert(b) if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 0); }
#define Log(...) _DebugLog(__FILE__, __LINE__, __VA_ARGS__)
#define INVALID_CODE_PATH Assert("Invalid Code Path" == 0)

internal void _AssertFailure(char *expression, int line, char *file, int crash);
internal void _DebugLog(char *file, int line, char *format, ...);