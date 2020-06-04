
// TODO(Tyler): This logger is pretty terrible and could be made more
// effective

global os_file *LogFile;
global u32 LogFileOffset;

internal void
LogError(char *Message){
    u32 Length = CStringLength(Message);
    WriteToFile(LogFile, LogFileOffset, Message, Length);
    LogFileOffset += Length;
    char End[] = "\n";
    WriteToFile(LogFile, LogFileOffset, End, ArrayCount(End));
    LogFileOffset += ArrayCount(End);
}
