#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Debug stuff
struct debug_physics_info {
    v2 P, dP, ddP, Delta;
    b8 DebugThisOne;
};

typedef u32 physics_debugger_flags;
enum physics_debugger_flags_ {
    PhysicsDebuggerFlags_None = 0,
    PhysicsDebuggerFlags_StepPhysics = (1 << 0),
    PhysicsDebuggerFlags_Visualize   = (1 << 1),
};

struct physics_debugger_position {
    // Arbitrary numbers to keep track of position, do not work between physics frames
    u32 Position;
    u32 Object;
};

struct physics_object;
// The debugger currently only supports single moving objects
struct physics_debugger {
    physics_debugger_flags Flags;
    physics_debugger_position Current;
    physics_debugger_position Paused;
    layout Layout;
    f32 Scale = 3.0f;
    v2 Origin;
    b8 DoDebug;
    b8 StartOfPhysicsFrame = true;
    
    // These are so that functions don't need extra return values or arguments
    union {
        v2 Base; // Used by UpdateSimplex to know where to draw the direction from
    };
    
    inline void NewFrame();
    inline void AdvanceCurrentObject();
    inline b8   AdvanceCurrentPosition();
    inline void BreakWhen(b8 Value); // Assert is a macro, so it can't be the name here
    inline b8   IsCurrentObject();
    
    inline void DrawLine(v2 Offset, v2 A, v2 B, color Color);
    inline void DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color);
    inline void DrawNormal(v2 Offset, v2 Point, v2 Delta, color Color);
    inline void DrawPoint(v2 Offset, v2 Point, color Color);
    inline void DrawString(const  char *String, ...);
};

//~ Collision boundary
enum collision_boundary_type {
    BoundaryType_None,
    BoundaryType_Rect,
    BoundaryType_Wedge,
    BoundaryType_FreeForm,
};

typedef u32 boundary_flags;
enum _boundary_flags {
    BoundaryFlag_None = 0,
    BoundaryFlag_CanStandOn = (1 << 0),
};

typedef v2 gjk_simplex[3];

struct collision_boundary {
    collision_boundary_type Type;
    boundary_flags Flags;
    v2 Offset;
    rect Bounds;
    
    union {
        // Rects just use 'rect Bounds'
        
        // FreeForm
        struct {
            v2 *FreeFormPoints;
            u32 FreeFormPointCount;
        };
        
        // Wedges
        struct {
            // but a direction is needed
            v2 WedgePoints[3];
        };
    };
};

global_constant u32 MAX_BOUNDARY_CHILDREN = 3;

//~ Physics

struct physics_object {
    v2 P, dP, ddP;
    v2 Delta;
    f32 Mass;
    collision_boundary *Boundaries;
    u8 BoundaryCount;
    debug_physics_info DebugInfo;
};


struct physics_collision {
    physics_object *ObjectA;
    physics_object *ObjectB;
    u32 BIndex;
    v2 Normal;
    v2 Correction;
    f32 TimeOfImpact;
    f32 AlongDelta;
};

struct physics_system {
    bucket_array<physics_object, 64> Objects;
    bucket_array<physics_object, 64> StaticObjects;
    //freelist_allocator BoundaryAllocator;
    memory_arena PermanentBoundaryMemory;
    memory_arena BoundaryMemory;
    
    void Initialize(memory_arena *Arena);
    void Reload(u32 Width, u32 Height);
    void DoPhysics();
    physics_object *AddObject(collision_boundary *Boundaries, u8 BoundaryCount);
    physics_object *AddStaticObject(collision_boundary *Boundaries, u8 BoundaryCount);
    collision_boundary *AllocPermanentBoundaries(u32 Count);
    collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
