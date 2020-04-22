struct profile_data {
    char *Names[256];
};
global profile_data GlobalProfileData;

struct timed_block {
    char *Id;
    u64 StartCycle;
};

internal timed_block
CreateTimedBlock(char *Id){
    timed_block Result;
    Result.Id = Id;
    Result.StartCycle = __rdtsc();
    return(Result);
}

internal u64
EndTimedBlock(timed_block *Block, char *Id){
}