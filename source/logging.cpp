
#if 0
#if defined(SNAIL_JUMPY_ASSET_PROCESSOR_BUILD)
#include <stdio.h>
#define WriteToConsole(Format, Args) vprintf(Format, Args)
#else 
#endif
#endif

#define WriteToConsole(Buffer, Args) OSVWriteToDebugConsole(Buffer, Args);

#if defined(SNAIL_JUMPY_DEBUG_BUILD)
internal void
VLogMessage(const char *Format, va_list VarArgs){
    char Buffer[DEFAULT_BUFFER_SIZE];
    u32 FormatLength = CStringLength(Format)+1;
    CopyMemory(Buffer, Format, Minimum(FormatLength, DEFAULT_BUFFER_SIZE));
    if(FormatLength < DEFAULT_BUFFER_SIZE){
        Buffer[FormatLength-1] = '\r';
        Buffer[FormatLength] = '\n';
        Buffer[FormatLength+1] = '\0';
    }
    
    WriteToConsole(Buffer, VarArgs);
    
#if 0
    u32 Length = CStringLength(Buffer);
    WriteToFile(LogFile, LogFileOffset, Buffer, Length);
    LogFileOffset += Length;
#endif
}

internal void
LogMessage(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    VLogMessage(Format, VarArgs);
    
    va_end(VarArgs);
}
#else
internal void VLogMessage(const char *Format, va_list VarArgs){}
internal void LogMessage(const char *Format, ...) {}
#endif
