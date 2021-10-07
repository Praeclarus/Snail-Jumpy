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
 PhysicsStateFlag_HadAnUpdate      = (1 << 3),
 PhysicsStateFlag_TriggerIsActive  = (1 << 4),
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
 
 // If collided
 physics_collision Collision;
};

//~ Triggers
struct physics_trigger_collision {
 entity *Trigger;
};

//~ System
struct entity_manager;
struct physics_system {
 // TODO(Tyler): This only need be here if we do physics particles
 bucket_array<physics_particle_system, 64> ParticleSystems;
 
 memory_arena ParticleMemory;
 memory_arena PermanentBoundaryMemory;
 memory_arena BoundaryMemory;
 
 stack<physics_update> Updates;
 
 void Initialize(memory_arena *Arena);
 void Reload(u32 Width, u32 Height);
 
 void DoPhysics(entity_manager *Manager);
 void DoFloorRaycast(entity_manager *Manager, entity *Entity, physics_layer_flags Layer, f32 Depth);
 
 void DoStaticCollisions(entity_manager *Manager, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoTriggerCollisions(entity_manager *Manager, physics_trigger_collision *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoCollisionsRelative(entity_manager *Manager, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer, u32 StartIndex=0);
 void DoCollisionsNotRelative(entity_manager *Manager, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer);
 
 inline physics_update *MakeUpdate(entity *Entity, physics_layer_flags Layer);
 
 physics_particle_system *AddParticleSystem(v2 P, collision_boundary *Boundary, u32 ParticleCount, f32 COR);
 
 collision_boundary *AllocPermanentBoundaries(u32 Count);
 collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
