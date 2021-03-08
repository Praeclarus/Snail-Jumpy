global os_file *LogFile;
global u32 LogFileOffset;

internal void
LogMessage(char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    u32 FormatLength = CStringLength(Format)+1;
    CopyMemory(Buffer, Format, Minimum(FormatLength, DEFAULT_BUFFER_SIZE));
    if(FormatLength < DEFAULT_BUFFER_SIZE){
        Buffer[FormatLength-1] = '\r';
        Buffer[FormatLength] = '\n';
        Buffer[FormatLength+1] = '\0';
    }
    
    VWriteToDebugConsole(ConsoleErrorFile, Buffer, VarArgs);
    
#if 0
    u32 Length = CStringLength(Buffer);
    WriteToFile(LogFile, LogFileOffset, Buffer, Length);
    LogFileOffset += Length;
#endif
    
    va_end(VarArgs);
}
