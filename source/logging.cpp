
// TODO(Tyler): This logger is pretty terrible and could be made more
// effective

global os_file *LogFile;
global u32 LogFileOffset;

internal void
LogMessage(char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, DEFAULT_BUFFER_SIZE, Format, VarArgs);
    
    u32 Length = CStringLength(Buffer);
    WriteToFile(LogFile, LogFileOffset, Buffer, Length);
    LogFileOffset += Length;
    char End[] = "\n";
    WriteToFile(LogFile, LogFileOffset, End, ArrayCount(End));
    LogFileOffset += ArrayCount(End);
    
    va_end(VarArgs);
}
