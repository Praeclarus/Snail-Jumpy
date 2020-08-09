global os_file *LogFile;
global u32 LogFileOffset;

internal void
LogMessage(char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, DEFAULT_BUFFER_SIZE, Format, VarArgs);
    
    u32 Length = CStringLength(Buffer);
    char End[] = "\n";
    WriteToFile(LogFile, LogFileOffset, Buffer, Length);
    LogFileOffset += Length;
    WriteToFile(LogFile, LogFileOffset, End, ArrayCount(End));
    LogFileOffset += ArrayCount(End);
    
    WriteToDebugConsole(ConsoleErrorFile, Buffer);
    WriteToDebugConsole(ConsoleErrorFile, End);
    
    va_end(VarArgs);
}

internal void
ConsoleLog(char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, DEFAULT_BUFFER_SIZE, Format, VarArgs);
    char End[] = "\n";
    
    WriteToDebugConsole(ConsoleErrorFile, Buffer);
    WriteToDebugConsole(ConsoleErrorFile, End);
    
    va_end(VarArgs);
}
