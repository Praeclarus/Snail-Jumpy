#if !defined(SNAIL_JUMPY_DEBUG_H)
#define SNAIL_JUMPY_DEBUG_H

//~ Basic profiling stuff
struct profiled_block {
 const char *Name;
 u64 CycleCount;
 u32 Level;
};

struct profile_data {
 profiled_block Blocks[100000];
 u32 CurrentBlockIndex;
 u32 CurrentLevel;
};

// TODO(Tyler): Move this once there is a better spot
global profile_data ProfileData;

internal u32
BeginProfiledBlock(const char *Name){
 ProfileData.Blocks[ProfileData.CurrentBlockIndex].Name = Name;
 ProfileData.Blocks[ProfileData.CurrentBlockIndex].Level = ProfileData.CurrentLevel;
 ProfileData.CurrentLevel++;
 
 u32 Result = ProfileData.CurrentBlockIndex++;
 return(Result);
}

internal void
EndProfiledBlock(u32 Index, u64 CycleCount){
 ProfileData.Blocks[Index].CycleCount = CycleCount;
 
 Assert(ProfileData.CurrentLevel > 0);
 ProfileData.CurrentLevel--;
}

struct timed_scope {
 u64 StartCycleCount;
 u32 Index;
 
 timed_scope(const char *BlockName){
  Index = BeginProfiledBlock(BlockName);
  this->StartCycleCount = __rdtsc();
 }
 
 ~timed_scope(){
  EndProfiledBlock(Index, __rdtsc()-StartCycleCount);
 }
};

#define _TIMED_SCOPE(Id, Name) timed_scope TimedScope##Id(Name);
#define TIMED_SCOPE(Id) timed_scope TimedScope##Id(#Id);
#define TIMED_FUNCTION() _TIMED_SCOPE(FUNC, __FUNCTION__)
#define BEGIN_TIMED_BLOCK(Id) { timed_scope TimedScope##Id(#Id);
#define END_TIMED_BLOCK() }

#define GetCycles(Id) \
SafeRatio0(ProfileData.TotalCycleCounts[ProfilerIndex##Id], ProfileData.ProfileCounts[ProfilerIndex##Id])

#define GetTimedScopeCycles(Id) \
SafeRatio0(ProfileData.TotalCycleCounts[TimedScope##Id.Index], ProfileData.ProfileCounts[TimedScope##Id.

#define BEGIN_BLOCK(Id) u32 ProfileIndex##Id = BeginProfiledBlock(#Id); u64 StartCycle##Id = __rdtsc();
#define END_BLOCK(Id) EndProfiledBlock(ProfileIndex##Id, __rdtsc()-StartCycle##Id);

//~ Debug/developer stuff

typedef u32 debug_overlay_flags;
enum _debug_overlay_flags {
 DebugOverlay_None     = (0 << 0),
 DebugOverlay_Profiler = (1 << 0),
 DebugOverlay_Miscellaneous = (1 << 1),
 DebugOverlay_Boundaries = (1 << 2),
};

struct debug_config {
 debug_overlay_flags Overlay;
};

#endif //SNAIL_JUMPY_DEBUG_H
