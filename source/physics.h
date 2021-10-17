#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Debug stuff
struct debug_physics_info {
 v2 P, dP, ddP, Delta;
};

typedef u32 physics_debugger_flags;
enum physics_debugger_flags_ {
 PhysicsDebuggerFlags_None = 0,
 PhysicsDebuggerFlags_StepPhysics = (1 << 0),
};

// The debugger currently only supports single moving objects
struct physics_debugger {
 physics_debugger_flags Flags;
 // Arbitrary numbers to keep track of position, do not work between physics frames
 u32 Current;
 u32 Paused;
 layout Layout;
 f32 Scale = 3.0f;
 v2 Origin;
 b8 StartOfPhysicsFrame = true;
 
 // These are so that functions don't need extra return values or arguments
 union {
  v2 Base; // Used by UpdateSimplex to know where to draw the direction from
 };
 
 inline void Begin();
 inline void End();
 inline b8   DefineStep();
 inline void BreakWhen(b8 Value); // Assert is a macro, so it can't be the name here
 inline b8   IsCurrent();
 
 inline void DrawPolygon(v2 *Points, u32 PointCount);
 inline void DrawLine(v2 Offset, v2 A, v2 B, color Color);
 inline void DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color);
 inline void DrawNormal(v2 Offset, v2 Point, v2 Delta, color Color);
 inline void DrawPoint(v2 Offset, v2 Point, color Color);
 inline void DrawString(const  char *String, ...);
 inline void DrawStringAtP(v2 P, const char *Format, ...);
 
 inline void DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount);
};

//~ Collision boundary
enum collision_boundary_type {
 BoundaryType_None,
 BoundaryType_Rect,
 BoundaryType_FreeForm,
 BoundaryType_Point, // Basically identical(right now) to BoundaryType_None
};

typedef v2 gjk_simplex[3];

struct collision_boundary {
 collision_boundary_type Type;
 v2 Offset;
 rect Bounds;
 
 union {
  // Rects just use 'rect Bounds'
  
  // FreeForm
  struct {
   v2 *FreeFormPoints;
   u32 FreeFormPointCount;
  };
 };
};

//~ Particles
struct physics_particle_x4 {
 v2_x4 P, dP;
 
 f32_x4 Lifetime;
};

struct physics_particle_system {
 collision_boundary *Boundary;
 array<physics_particle_x4> Particles;
 v2 P;
 v2 StartdP;
 f32 COR;
};


//~ Physics

struct entity;
struct physics_collision;
struct physics_update;
typedef b8   collision_response_function(physics_update *Update, physics_collision *Collision);
typedef void trigger_response_function(entity *Entity, entity *EntityB);

typedef u32 physics_state_flags;
enum physics_state_flags_ {
 PhysicsStateFlag_None             = (0 << 0),
 PhysicsStateFlag_Falling          = (1 << 0),
 PhysicsStateFlag_DontFloorRaycast = (1 << 1),
 PhysicsStateFlag_Inactive         = (1 << 2),
 PhysicsStateFlag_TriggerIsActive  = (1 << 3),
};

typedef u32 physics_layer_flags;
enum physics_layer_flags_ {
 PhysicsLayerFlag_None       = (0 << 0),
 PhysicsLayerFlag_Basic      = (1 << 0),
 PhysicsLayerFlag_Projectile = (1 << 1),
 PhysicsLayerFlag_PlayerTrigger = (1 << 2),
 PhysicsLayerFlag_Static     = (PhysicsLayerFlag_Basic |
                                PhysicsLayerFlag_Projectile)
};

struct physics_collision {
 entity *EntityB;
 v2 Normal;
 v2 Correction;
 f32 TimeOfImpact;
 f32 AlongDelta;
};

struct physics_update {
 entity *Entity;
 v2 Delta;
 
 physics_layer_flags Layer;
 u16 ParentID;
 u16 ChildID;
 
 // If collided
 physics_collision Collision;
};

struct physics_update_context {
 stack<physics_update> Updates;
 array<physics_update *> ChildUpdates;
 array<v2> DeltaCorrections;
 u16 CurrentID;
};

//~ Triggers
struct physics_trigger_collision {
 entity *Trigger;
};

#endif //SNAIL_JUMPY_PHYSICS_H
