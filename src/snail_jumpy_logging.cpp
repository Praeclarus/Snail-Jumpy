global platform_file *GlobalLogFile;
global u32 GlobalLogFileOffset;

internal void
LogError(char *Message){
    u32 Length = CStringLength(Message);
    WriteToFile(GlobalLogFile, GlobalLogFileOffset, Message, Length);
    GlobalLogFileOffset += Length;
    char End[] = "\n";
    WriteToFile(GlobalLogFile, GlobalLogFileOffset, End, ArrayCount(End));
    GlobalLogFileOffset += ArrayCount(End);
}
