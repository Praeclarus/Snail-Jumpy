#if !defined(SNAIL_JUMPY_DEBUG_H)
#define SNAIL_JUMPY_DEBUG_H

struct profiled_block {
    char *Name;
    u64 CycleCount;
    u32 Level;
};

struct profile_data {
    profiled_block Blocks[256];
    u32 CurrentBlockIndex;
    u32 CurrentLevel;
};

// TODO(Tyler): Move this once there is a better spot
global profile_data GlobalProfileData;

internal u32
BeginProfiledBlock(char *Name){
    GlobalProfileData.Blocks[GlobalProfileData.CurrentBlockIndex].Name = Name;
    GlobalProfileData.Blocks[GlobalProfileData.CurrentBlockIndex].Level = GlobalProfileData.CurrentLevel;
    GlobalProfileData.CurrentLevel++;
    
    u32 Result = GlobalProfileData.CurrentBlockIndex++;
    return(Result);
}

internal void
EndProfiledBlock(u32 Index, u64 CycleCount){
    GlobalProfileData.Blocks[Index].CycleCount = CycleCount;
    
    Assert(GlobalProfileData.CurrentLevel > 0);
    GlobalProfileData.CurrentLevel--;
}

struct timed_scope {
    u64 StartCycleCount;
    u32 Index;
    
    timed_scope(char *BlockName){
        Index = BeginProfiledBlock(BlockName);
        this->StartCycleCount = __rdtsc();
    }
    
    ~timed_scope(){
        EndProfiledBlock(Index, __rdtsc()-StartCycleCount);
    }
};

#define TIMED_SCOPE(Id, Name) timed_scope TimedScope##Id(Name);
#define TIMED_FUNCTION() TIMED_SCOPE(FUNC, __FUNCTION__)

#define GetCycles(Id) \
SafeRatio0(GlobalProfileData.TotalCycleCounts[ProfilerIndex##Id], GlobalProfileData.ProfileCounts[ProfilerIndex##Id])

#define GetTimedScopeCycles(Id) \
SafeRatio0(GlobalProfileData.TotalCycleCounts[TimedScope##Id.Index], GlobalProfileData.ProfileCounts[TimedScope##Id.

#define BEGIN_BLOCK(Id) u32 ProfileIndex##Id = BeginProfiledBlock(#Id); u64 StartCycle##Id = __rdtsc();
#define END_BLOCK(Id) EndProfiledBlock(ProfileIndex##Id, __rdtsc()-StartCycle##Id);

#endif //SNAIL_JUMPY_DEBUG_H
