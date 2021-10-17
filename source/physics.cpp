physics_debugger PhysicsDebugger;

//~ Debug stuff
inline void
physics_debugger::Begin(){
 Current = {};
 Layout = MakeLayout(1500, 700, 30, DebugFont.Size, 100, -10.2f);
 Origin = V2(100.0f, 100.0f);
 DrawPoint(Origin, V2(0), WHITE);
}

inline void
physics_debugger::End(){
 Paused = {};
 StartOfPhysicsFrame = true;
}

inline b8
physics_debugger::DefineStep(){
 if(Current >= Paused &&
    (Flags & PhysicsDebuggerFlags_StepPhysics)){
  return(true);
 }
 
 Current++;
 
 return(false);
}

inline void
physics_debugger::BreakWhen(b8 Value){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics) && 
    Value){
  Flags |= PhysicsDebuggerFlags_StepPhysics;
  Paused = Current;
  Paused++;
 }
}

inline b8 
physics_debugger::IsCurrent(){
 b8 Result = (PhysicsDebugger.Current == PhysicsDebugger.Paused);
 return(Result);
}

inline void
physics_debugger::DrawPoint(v2 Offset, v2 Point, color Color){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 RenderRect(CenterRect(Offset + Scale*Point, V2(1.0f)),
            -10.3f, Color, ScaledItem(0));
}

inline void
physics_debugger::DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 RenderLineFrom(Offset+Scale*A, Scale*Delta, -10, 0.5f, Color, ScaledItem(0));
}

inline void
physics_debugger::DrawNormal(v2 Offset, v2 A, v2 Delta, color Color){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 Delta = Normalize(Delta);
 RenderLineFrom(Offset+Scale*A, 0.2f*Delta, -10, 0.4f, Color, ScaledItem(0));
}

inline void
physics_debugger::DrawLine(v2 Offset, v2 A, v2 B, color Color){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 RenderLine(Offset+Scale*A, Offset+Scale*B, -10, 0.5f, Color, ScaledItem(0));
}

inline void
physics_debugger::DrawString(const char *Format, ...){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 va_list VarArgs;
 va_start(VarArgs, Format);
 VLayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, Format, VarArgs);
 va_end(VarArgs);
}

inline void
physics_debugger::DrawStringAtP(v2 P, const char *Format, ...){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 va_list VarArgs;
 va_start(VarArgs, Format);
 
 P.Y += 0.1f;
 v2 StringP = GameRenderer.WorldToScreen(P, UIItem(0));
 VRenderFormatString(&DebugFont, BLACK, StringP, -10.0f, Format, VarArgs);
 
 va_end(VarArgs);
}

inline void
physics_debugger::DrawPolygon(v2 *Points, u32 PointCount){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 for(u32 I=0; I < PointCount; I++){
  u32 J = (I+1)%PointCount;
  DrawLine(Origin, Points[I], Points[J], PURPLE);
  DrawPoint(Origin, Points[I], GREEN);
  
  v2 P = Origin + (Scale * Points[I]);
  P.Y += 0.1f;
  v2 NameP = GameRenderer.WorldToScreen(P, UIItem(0));
  RenderFormatString(&DebugFont, BLACK, NameP, -10.0f, "%u", I);
 }
}

inline void
physics_debugger::DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount){
 if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
 PhysicsDebugger.DrawPoint(AP, V2(0), WHITE);
 PhysicsDebugger.DrawPoint(BP, V2(0), DARK_GREEN);
 PhysicsDebugger.DrawLineFrom(PhysicsDebugger.Origin, V2(0), Delta, ORANGE);
 PhysicsDebugger.DrawPolygon(Points, PointCount);
 PhysicsDebugger.DrawString("Delta: (%f, %f)", Delta.X, Delta.Y);
} 

//~ Boundary stuff
internal inline b8
IsPointInBoundary(v2 Point, collision_boundary *Boundary, v2 Base=V2(0,0)){
 b8 Result = IsPointInRect(Point, Boundary->Bounds+Base+Boundary->Offset);
 return(Result);
}

internal inline void
RenderBoundary(collision_boundary *Boundary, f32 Z, v2 Offset){
 Offset += Boundary->Offset;
 color Color = MakeColor(0.0f, 0.8f, 0.8f, 1.0f);
 f32 Thickness = 0.5f;
 switch(Boundary->Type){
  case BoundaryType_None: break;
  case BoundaryType_Rect: {
   v2 Points[4] = {
    V2(Boundary->Bounds.Min.X, Boundary->Bounds.Min.Y),
    V2(Boundary->Bounds.Max.X, Boundary->Bounds.Min.Y),
    V2(Boundary->Bounds.Max.X, Boundary->Bounds.Max.Y),
    V2(Boundary->Bounds.Min.X, Boundary->Bounds.Max.Y),
   };
   
   for(u32 I=0; I < 4; I++){
    v2 PointA = Points[I] + Offset;
    v2 PointB = Points[(I+1)%4] + Offset;
    RenderLine(PointA, PointB, Z-0.15f, Thickness, Color, ScaledItem(1));
   }
   
  }break;
  case BoundaryType_FreeForm: {
   u32 Count = Boundary->FreeFormPointCount;
   for(u32 I=0; I < Count; I++){
    v2 PointA = Boundary->FreeFormPoints[I] + Offset;
    v2 PointB = Boundary->FreeFormPoints[(I+1)%Count] + Offset;
    RenderLine(PointA, PointB, Z-0.15f, Thickness, Color, ScaledItem(1));
   }
   
  }break;
  default: INVALID_CODE_PATH;
 }
 
 rect R = Boundary->Bounds + Offset;
 RenderRectOutline(R, Z-0.1f, RED, ScaledItem(1), Thickness);
}

internal inline void
RenderEntityPhysics(entity *Entity){
 if(Entity->PhysicsFlags & PhysicsStateFlag_Inactive) return;
 
 RenderRectOutline(Entity->Bounds + Entity->P, -10.0f, GREEN, ScaledItem(1), 0.02f);
 for(collision_boundary *Boundary = Entity->Boundaries;
     Boundary < Entity->Boundaries+Entity->BoundaryCount;
     Boundary++){
  RenderBoundary(Boundary, -10.1f, Entity->P);
 }
}

internal inline collision_boundary
MakeCollisionPoint(){
 collision_boundary Result = {};
 Result.Type = BoundaryType_Point;
 return(Result);
}

internal inline collision_boundary
MakeCollisionRect(v2 Offset, v2 Size){
 collision_boundary Result = {};
 Result.Type = BoundaryType_Rect;
 Result.Offset = Offset;
 Result.Bounds = CenterRect(V2(0), Size);
 return(Result);
}

internal inline collision_boundary
MakeCollisionWedge(v2 Offset, f32 X, f32 Y, memory_arena *Arena=0){
 if(!Arena) Arena = &EntityManager.BoundaryMemory;
 
 collision_boundary Result = {};
 Result.Type = BoundaryType_FreeForm;
 Result.Offset = Offset;
 f32 MinX = Minimum(X, 0.0f);
 f32 MinY = Minimum(Y, 0.0f);
 Result.Bounds.Min = V2(MinX, MinY);
 
 f32 MaxX = Maximum(X, 0.0f);
 f32 MaxY = Maximum(Y, 0.0f);
 Result.Bounds.Max = V2(MaxX, MaxY);
 
 Result.FreeFormPointCount = 3;
 Result.FreeFormPoints = PushArray(Arena, v2, 3);
 Result.FreeFormPoints[0] = V2(0);
 Result.FreeFormPoints[1] = V2(X, 0.0f);
 Result.FreeFormPoints[2] = V2(0.0f, Y);
 
 return(Result);
}

internal inline collision_boundary
MakeCollisionCircle(v2 Offset, f32 Radius, u32 Segments, memory_arena *Arena=0){
 if(!Arena) Arena = &EntityManager.BoundaryMemory;
 
 collision_boundary Result = {};
 Result.Type = BoundaryType_FreeForm;
 Result.Offset = Offset;
 Result.Bounds = CenterRect(V2(0), 2*V2(Radius));
 
 // TODO(Tyler): There might be a better way to do this that doesn't require
 // calculation beforehand
 Result.FreeFormPointCount = Segments;
 Result.FreeFormPoints = PushArray(Arena, v2, Segments);
 f32 T = 0.0f;
 f32 Step = 1.0f/(f32)Segments;
 for(u32 I = 0; I <= Segments; I++){
  Result.FreeFormPoints[I] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
  T += Step;
 }
 
 return(Result);
}

internal inline collision_boundary
MakeCollisionPill(v2 Offset, f32 Radius, f32 Height, u32 HalfSegments, memory_arena *Arena=0){
 if(!Arena) Arena = &EntityManager.BoundaryMemory;
 
 collision_boundary Result = {};
 Result.Type = BoundaryType_FreeForm;
 Result.Offset = Offset;
 Result.Bounds = MakeRect(V2(-Radius, -Radius), V2(Radius, Height+Radius));
 
 u32 Segments = 2*HalfSegments;
 u32 ActualSegments = Segments + 2;
 Result.FreeFormPointCount = ActualSegments;
 Result.FreeFormPoints = PushArray(Arena, v2, ActualSegments);
 
 f32 HeightOffset = Height;
 f32 T = 0.0f;
 f32 Step = 1.0f/((f32)Segments);
 u32 Index = 0;
 
 for(u32 I=0; I <= HalfSegments; I++){
  Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
  Result.FreeFormPoints[Index].Y += HeightOffset;
  T += Step;
  Index++;
 }
 T -= Step;
 
 HeightOffset = 0;
 for(u32 I=0; I <= HalfSegments; I++){
  Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
  Result.FreeFormPoints[Index].Y += HeightOffset;
  T += Step;
  Index++;
 }
 
 
 return(Result);
}

internal rect
GetBoundsOfBoundaries(collision_boundary *Boundaries, u32 BoundaryCount){
 v2 Min = V2(F32_POSITIVE_INFINITY);
 v2 Max = V2(F32_NEGATIVE_INFINITY);
 for(u32 I=0; I < BoundaryCount; I++){
  Min = MinimumV2(Boundaries[I].Bounds.Min+Boundaries[I].Offset, Min);
  Max = MaximumV2(Boundaries[I].Bounds.Max+Boundaries[I].Offset, Max);
 }
 rect Result = MakeRect(Min, Max);
 return(Result);
}

//~ Collision stuff
internal inline physics_collision
MakeCollision(){
 physics_collision Result = {};
 Result.TimeOfImpact = F32_POSITIVE_INFINITY;
 return(Result);
}

internal b8
CollisionResponseStub(physics_update *Update, physics_collision *Collision){
 b8 Result = false;
 return(Result);
}

//~ Update stuff
internal inline physics_update_context
MakeUpdateContext(memory_arena *Arena, u32 MaxCount){
 physics_update_context Result = {};
 Result.Updates = MakeStack<physics_update>(Arena, MaxCount);
 Result.ChildUpdates = MakeArray<physics_update *>(Arena, MaxCount);
 
 return Result;
}

internal inline void
PrepareUpdateContext(physics_update_context *Context, memory_arena *Arena){
 Context->CurrentID = 1;
 
 for(u32 I=0; I<Context->ChildUpdates.Count; I++){
  physics_update *Update = Context->ChildUpdates[I];
  
  if(Update->Entity->Parent){
   entity *Parent = Update->Entity->Parent;
   Assert(Parent->Update);
   
   if(Parent->Update->ChildID){
    Update->ParentID = Parent->Update->ChildID;
   }else{
    Update->ParentID = Context->CurrentID++;
    Parent->Update->ChildID = Update->ParentID;
   }
   
   while(Parent){
    Update->Delta += Parent->Update->Delta;
    
    Parent = Parent->Update->Entity->Parent;
   }
  }
 }
 
 Context->DeltaCorrections = MakeFullArray<v2>(Arena, Context->CurrentID);
}

inline physics_update *
MakeUpdate(physics_update_context *Context, entity *Entity, physics_layer_flags Layer){
 physics_update *Result = StackPushAlloc(&Context->Updates);
 Result->Entity = Entity;
 Result->Collision = MakeCollision();
 Result->Layer = Layer;
 
 return Result;
}


//~ Physics
physics_particle_system *
entity_manager::AddParticleSystem(v2 P, collision_boundary *Boundary, u32 Count,
                                  f32 COR=1.0f){
 physics_particle_system *Result = BucketArrayAlloc(&ParticleSystems);
 Result->Particles = MakeFullArray<physics_particle_x4>(&ParticleMemory, Count, 16);
 Result->Boundary = Boundary;
 Result->P = P;
 Result->COR = COR;
 return(Result);
}

collision_boundary *
entity_manager::AllocPermanentBoundaries(u32 Count){
 collision_boundary *Result = PushArray(&PermanentBoundaryMemory, collision_boundary, Count);
 return(Result);
}

collision_boundary *
entity_manager::AllocBoundaries(u32 Count){
 collision_boundary *Result = PushArray(&BoundaryMemory, collision_boundary, Count);
 return(Result);
}

//~ Collision detection

internal inline b8
DoAABBTest(rect BoundsA, v2 Offset, v2 AP, 
           rect BoundsB, v2 BP, v2 Delta){
 BoundsA += Offset;
 rect RectA1 = BoundsA;
 rect RectA2 = RectA1 + Delta;
 rect RectA;
 RectA.Min = MinimumV2(RectA1.Min, RectA2.Min);
 RectA.Max = MaximumV2(RectA1.Max, RectA2.Max);
 RectA += AP;
 rect RectB = BoundsB;
 RectB = RectB + BP;
 b8 Result = DoRectsOverlap(RectA, RectB);
 return(Result);
}

internal inline v2
DoSupport(collision_boundary *Boundary, 
          v2 P, v2 Delta,
          v2 Direction){
 v2 Result = {};
 
 switch(Boundary->Type){
  case BoundaryType_Rect: {
   v2 Min = Boundary->Bounds.Min;
   v2 Max = Boundary->Bounds.Max;
   Result = Min;
   if(Dot(V2(1, 0), Direction) > 0.0f) { Result.X = Max.X; }
   if(Dot(V2(0, 1), Direction) > 0.0f) { Result.Y = Max.Y; }
  }break;
  case BoundaryType_FreeForm: {
   b8 Found = false;
   for(u32 I = 0; I < Boundary->FreeFormPointCount; I++){
    v2 Point = Boundary->FreeFormPoints[I];
    if((Dot(Point, Direction) > Dot(Result, Direction)) ||
       (!Found)){
     Result = Point;
     Found = true;
    }
   }
   Assert(Found);
   
  }break;
  case BoundaryType_None: break;
  case BoundaryType_Point: break;
  default: INVALID_CODE_PATH;
 }
 
 Result += Boundary->Offset;
 Result +=  P;
 if(Dot(Delta, Direction) > 0.0f){
  Result += Delta;
 }
 
 return(Result);
}

internal inline v2 
CalculateSupport(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta, v2 Direction){
 v2 Result = DoSupport(BoundaryA, AP, Delta, Direction) - DoSupport(BoundaryB, BP, V2(0), -Direction);
 return(Result);
}

internal b8
UpdateSimplex(v2 Simplex[3], u32 *SimplexCount, v2 *Direction){
 b8 Result = false;
 
 switch(*SimplexCount){
  case 2: {
   if(Dot(Simplex[0]-Simplex[1], -Simplex[1]) > 0.0f){
    *Direction = TripleProduct(Simplex[0]-Simplex[1], -Simplex[1]);
    if((Direction->X == 0.0f) && (Direction->Y == 0.0f)){
     // TODO(Tyler): I have no idea if there is a better way to do this
     *Direction = TripleProduct(Simplex[0]-Simplex[1], V2(0.1f, 0.1f)-Simplex[1]);
    }
    
    PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
   }else{
    *Direction = -Simplex[1];
    Simplex[0] = Simplex[1];
    *SimplexCount = 1;
    
    PhysicsDebugger.Base = Simplex[0];
   }
  }break;
  case 3: {
   // TODO(Tyler): This is a place that could be significantly improved
   
   v2 P2P0 = Simplex[0]-Simplex[2];
   v2 P2P1 = Simplex[1]-Simplex[2];
   f32 Z = P2P1.X*P2P0.Y - P2P1.Y*P2P0.X;
   
   v2 P2P0Direction = V2(-Z*P2P0.Y, Z*P2P0.X);
   b8 OutsideP2P0 = Dot(P2P0Direction, -Simplex[2]) >= 0.0f;
   v2 P2P1Direction = V2(Z*P2P1.Y, -Z*P2P1.X);
   b8 OutsideP2P1 = Dot(P2P1Direction, -Simplex[2]) >= 0.0f;
   b8 AlongP2P0 = Dot(P2P0 , -Simplex[2]) >= 0.0f;
   b8 AlongP2P1 = Dot(P2P1, -Simplex[2]) >= 0.0f;
   
   if(!OutsideP2P0 && OutsideP2P1 && AlongP2P1){
    // Area 4
    Simplex[0] = Simplex[1];
    Simplex[1] = Simplex[2];
    *SimplexCount = 2;
    *Direction = P2P1Direction;
    PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
   }else if(OutsideP2P0 && !OutsideP2P1 && AlongP2P0){
    // Area 6
    Simplex[1] = Simplex[2];
    *SimplexCount = 2;
    *Direction = P2P0Direction;
    PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
   }else if(OutsideP2P0 && OutsideP2P1 && !AlongP2P0 && !AlongP2P0){
    // Area 5
    Simplex[0] = Simplex[2];
    *SimplexCount = 1;
    *Direction = -Simplex[0];
    PhysicsDebugger.Base = Simplex[0];
   }else{
    // Area 7, we have enclosed the origin and have a collision
    Result = true;
   }
  }break;
  default: {
   INVALID_CODE_PATH;
  }break;
 }
 
 return(Result);
}

internal inline b8
DoGJK(v2 Simplex[3], 
      collision_boundary *BoundaryA, v2 AP, 
      collision_boundary *BoundaryB, v2 BP, v2 Delta){
 f32 ObjectBPAlongDelta = Dot(Delta, BP-AP);
 u32 SimplexCount = 1; // Account for the first support point
 v2 Direction = AP - BP;
 if((Direction.X == 0.0f) && (Direction.Y == 0.0f)){
  Direction = V2(1, 0);
 }
 Simplex[0] = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, Direction);
 Direction = -Simplex[0];
 
 b8 DoesCollide = false;
 PhysicsDebugger.Base = Simplex[0];
 while(true){
  if(PhysicsDebugger.DefineStep()) return(false);
  if(PhysicsDebugger.IsCurrent()){
   PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Simplex, SimplexCount);
   
   PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, PhysicsDebugger.Base, Direction, PINK);
   
   PhysicsDebugger.DrawString("Simplex Count: %u", SimplexCount);
  }
  
  
  v2 NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, Direction);
  if(Dot(NewPoint, Direction) < 0.0f){ 
   // The new point is in the wrong direction, hence
   // there is no point going the right direction, hence
   // the simplex doesn't enclose the origin, hence
   // no collision
   break;
  }
  Simplex[SimplexCount] = NewPoint;
  SimplexCount++;
  if(UpdateSimplex(Simplex, &SimplexCount, &Direction)){
   DoesCollide = true;
   break;
  }
 }
 
 return(DoesCollide);
}

struct epa_result {
 v2 Correction;
 f32 TimeOfImpact;
 v2 Normal;
};

// Variation on the expanding polytope algorithm that takes the object's delta into account
internal epa_result
DoDeltaEPA(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta, v2 Simplex[3]){
 const f32 Epsilon = 0.0001f;
 
 dynamic_array<v2> Polytope; 
 InitializeArray(&Polytope, 20, &TransientStorageArena);
 ArrayAdd(&Polytope, Simplex[0]);
 ArrayAdd(&Polytope, Simplex[1]);
 ArrayAdd(&Polytope, Simplex[2]);
 
 v2 DeltaNormal = Normalize(Clockwise90(Delta));
 v2 DeltaDirection = Normalize(Delta);
 
 enum found_edge_type {
  FoundEdge_None,
  FoundEdge_Colinear,
  FoundEdge_Beyond,
  FoundEdge_Ordinary,
 };
 
 epa_result Result = {};
 while(true){
  u32 EdgeIndex = 0;
  f32 EdgeDistance = 0;
  v2 InverseNormal = {};
  v2 IntersectionPoint = {};
  found_edge_type FoundEdge = FoundEdge_None;
  
  for(u32 I=0; I < Polytope.Count; I++){
   v2 A = Polytope[I];
   v2 B = Polytope[(I+1)%Polytope.Count];
   
   f32 AAlongDeltaNormal = Dot(DeltaNormal, A);
   f32 BAlongDeltaNormal = Dot(DeltaNormal, B);
   
   if(((-Epsilon <= AAlongDeltaNormal) && (AAlongDeltaNormal <= Epsilon)) && // Collinear
      ((-Epsilon <= BAlongDeltaNormal) && (BAlongDeltaNormal <= Epsilon))){
    FoundEdge = FoundEdge_Colinear;
    EdgeIndex = I;
    InverseNormal = V2(0);
    EdgeDistance = Dot(InverseNormal, -A);
    IntersectionPoint = V2(0);
   }else if(((AAlongDeltaNormal >= 0) && (BAlongDeltaNormal <= 0)) || 
            ((AAlongDeltaNormal <= 0) && (BAlongDeltaNormal >= 0))){ // The delta line  intersects
    AAlongDeltaNormal = AbsoluteValue(AAlongDeltaNormal);
    BAlongDeltaNormal = AbsoluteValue(BAlongDeltaNormal);
    f32 ABPercent = AAlongDeltaNormal/(AAlongDeltaNormal + BAlongDeltaNormal);
    v2 Point = A + ABPercent*(B-A);
    
    f32 PointAlongDeltaDirection = Dot(DeltaDirection, Point);
    f32 DeltaLength = Dot(DeltaDirection, Delta);
    
    if(-Epsilon <= PointAlongDeltaDirection){
     if(PointAlongDeltaDirection <= DeltaLength+Epsilon){ // The delta intersects
      FoundEdge = FoundEdge_Ordinary;
      EdgeIndex = I;
      InverseNormal = Normalize(TripleProduct(B-A, -A));
      EdgeDistance = Dot(InverseNormal, -A);
      PointAlongDeltaDirection = Clamp(PointAlongDeltaDirection, 0.0f, DeltaLength);
      IntersectionPoint = PointAlongDeltaDirection*DeltaDirection;
      break;
     }else if(FoundEdge != FoundEdge_Colinear){ // Intersect along the delta but beyond it
      FoundEdge = FoundEdge_Beyond;
      EdgeIndex = I;
      InverseNormal = Normalize(TripleProduct(B-A, -A));
      EdgeDistance = Dot(InverseNormal, -A);
      IntersectionPoint = Point;
     }
    }
   }
  }
  PhysicsDebugger.BreakWhen(FoundEdge == FoundEdge_None);
  
  f32 Distance;
  v2 NewPoint = V2(0);
  if((InverseNormal.X == 0.0f) && (InverseNormal.Y == 0.0f)){
   Distance = EdgeDistance;
  }else{
   NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, -InverseNormal);
   Distance = Dot(NewPoint, -InverseNormal);
  }
  
  //~ DEBUG
  if(PhysicsDebugger.DefineStep()) return(Result);
  if(PhysicsDebugger.IsCurrent()){
   PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Polytope.Items, Polytope.Count);
   
   f32 Percent = Dot(DeltaDirection, IntersectionPoint)/Dot(DeltaDirection, Delta);
   if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
    Percent = 0.0f;
   }
   
   v2 Base = 0.5f*(Polytope[EdgeIndex] + Polytope[(EdgeIndex+1)%Polytope.Count]);
   
   PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base, -InverseNormal, PINK);
   PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base,  InverseNormal, YELLOW);
   PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, IntersectionPoint, ORANGE);
   
   PhysicsDebugger.DrawString("Polytope Count: %u", Polytope.Count);
   PhysicsDebugger.DrawString("Time of impact: %f", 1.0f-Percent);
  }
  
  f32 DistanceEpsilon = 0.00001f;
  if((-DistanceEpsilon <= (Distance-EdgeDistance)) && ((Distance-EdgeDistance) <= DistanceEpsilon)){
   f32 Percent = Dot(DeltaDirection, IntersectionPoint)/Dot(DeltaDirection, Delta);
   if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
    Percent = 0.0f;
   }
   
   f32 TimeEpsilon = 0.0001f;
   Result.TimeOfImpact = (1.0f - Percent) - TimeEpsilon;
   if(Result.TimeOfImpact > 1.0f){
    Result.TimeOfImpact = 1.0f;
   }else if(Result.TimeOfImpact < -TimeEpsilon){
    //v2 Difference = Delta - IntersectionPoint;
    v2 Difference = Delta - (Percent+Epsilon)*Delta;
    Result.Correction = InverseNormal * Dot(InverseNormal, Difference);
    //Result.Correction = Difference;
    Result.TimeOfImpact = 0.0f;
   }else if(Result.TimeOfImpact < 0.0f){
    Result.TimeOfImpact = 0.0f;
   }
   Result.Normal = InverseNormal;
   
   break;
  }else{
   ArrayInsert(&Polytope, EdgeIndex+1, NewPoint);
  }
 }
 
 //~ DEBUG
 if(PhysicsDebugger.DefineStep()) return(Result); 
 if(PhysicsDebugger.IsCurrent()){
  PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Polytope.Items, Polytope.Count);
  
  PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, V2(0), Result.Normal, PINK);
  
  PhysicsDebugger.DrawString("Polytope.Count: %u", Polytope.Count);
  PhysicsDebugger.DrawString("Time of impact: %f", Result.TimeOfImpact);
  PhysicsDebugger.DrawString("Correction: (%f, %f)", Result.Correction.X, Result.Correction.Y);
 }
 
 
 return(Result);
}

//~ Physics system

internal physics_collision
DoCollision(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta){
 physics_collision Result = MakeCollision();
 Result.AlongDelta        = Dot(Normalize(Delta), BP-AP);
 
 v2 Simplex[3];
 if(DoGJK(Simplex, BoundaryA, AP, BoundaryB, BP, Delta)){
  epa_result EPAResult = DoDeltaEPA(BoundaryA, AP, BoundaryB, BP, Delta, Simplex);
  Result.Normal            = EPAResult.Normal;
  Result.Correction        = EPAResult.Correction;
  Result.TimeOfImpact      = EPAResult.TimeOfImpact;
 }
 
 return(Result);
}

internal b8
ChooseCollision(physics_collision *OldCollision, physics_collision *NewCollision){
 local_constant f32 Epsilon = 0.0001f;
 b8 Result = false;
 
 if(NewCollision->TimeOfImpact < OldCollision->TimeOfImpact){
  Result = true;
 }else if((NewCollision->TimeOfImpact > OldCollision->TimeOfImpact-Epsilon) && 
          (NewCollision->TimeOfImpact < OldCollision->TimeOfImpact+Epsilon)){
  if(NewCollision->AlongDelta < OldCollision->AlongDelta){
   Result = true;
  }
 }
 
 return(Result);
}

internal inline physics_collision 
MakeOtherCollision(entity *Entity, physics_collision *Collision){
 physics_collision Result = *Collision;
 Result.EntityB = Entity;
 Result.Normal = -Collision->Normal;
 Result.Correction = -Collision->Correction;
 
 return Result;
}

void 
entity_manager::DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta){
 if((Delta.X == 0.0f) && (Delta.Y == 0.0f)) { return; }
 
 FOR_EACH_ENTITY(this){
  if(!(It.Item->TypeFlags & EntityTypeFlag_Static)) continue;
  entity *EntityB = It.Item;
  
  if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, EntityB->Bounds, EntityB->P, Delta)){
   continue;
  }
  
  for(collision_boundary *BoundaryB = EntityB->Boundaries;
      BoundaryB < EntityB->Boundaries+EntityB->BoundaryCount;
      BoundaryB++){
   physics_collision Collision = DoCollision(Boundary, P, BoundaryB, EntityB->P, Delta);
   Collision.EntityB = EntityB;
   if(ChooseCollision(OutCollision, &Collision)){
    *OutCollision = Collision;
   }
  }
 }
 
 //~ Tilemaps
 FOR_ENTITY_TYPE(this, tilemap_entity){
  tilemap_entity *Tilemap = It.Item;
  v2 RelativeP = P - Tilemap->P;
  rect Bounds = Boundary->Bounds + (Boundary->Offset+RelativeP);
  Bounds = RectSweep(Bounds, Delta);
  Bounds.Min.X /= Tilemap->TileSize.X;
  Bounds.Min.Y /= Tilemap->TileSize.Y;
  Bounds.Max.X /= Tilemap->TileSize.X;
  Bounds.Max.Y /= Tilemap->TileSize.Y;
  
  rect_s32 BoundsS32 = RectS32(Bounds);
  v2s TilemapMax = V2S(Tilemap->TilemapData.Width, Tilemap->TilemapData.Height);
  BoundsS32.Min = MaximumV2S(V2S(0), BoundsS32.Min);
  BoundsS32.Max = MaximumV2S(V2S(0), BoundsS32.Max);
  BoundsS32.Min = MinimumV2S(TilemapMax, BoundsS32.Min);
  BoundsS32.Max = MinimumV2S(TilemapMax, BoundsS32.Max);
  
  for(s32 Y = BoundsS32.Min.Y; Y < BoundsS32.Max.Y; Y++){
   for(s32 X = BoundsS32.Min.X; X < BoundsS32.Max.X; X++){
    u8 TileID = Tilemap->PhysicsMap[(Y*Tilemap->TilemapData.Width)+X];
    v2 TileP = V2((f32)X, (f32)Y);
    TileP += V2(0.5f);
    TileP.X *= Tilemap->TileSize.X;
    TileP.Y *= Tilemap->TileSize.Y;
    TileP += Tilemap->P;
    rect TileBounds = CenterRect(V2(0), Tilemap->TileSize);
    
    if(TileID > 0){
     TileID--;
     Assert(TileID < Tilemap->BoundaryCount);
     
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
     if(DebugConfig.Overlay & DebugOverlay_Boundaries){
      collision_boundary *B = &Tilemap->Boundaries[TileID];
      RenderBoundary(B, -10.0f, TileP);
     }
#endif
     physics_collision Collision = DoCollision(Boundary, P, &Tilemap->Boundaries[TileID], TileP, Delta);
     if(ChooseCollision(OutCollision, &Collision)){
      *OutCollision = Collision;
     }
    }
   }
  }
 }
 
}

void 
entity_manager::DoTriggerCollisions(physics_trigger_collision *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta){
 //~ Triggers
 FOR_EACH_ENTITY(this){
  if(!(It.Item->TypeFlags & EntityTypeFlag_Trigger)) continue;
  entity *EntityB = It.Item;
  if(EntityB->PhysicsFlags & PhysicsStateFlag_Inactive) continue;
  if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, EntityB->Bounds, EntityB->P, Delta)) continue;
  
  for(collision_boundary *BoundaryB = EntityB->Boundaries;
      BoundaryB < EntityB->Boundaries+EntityB->BoundaryCount;
      BoundaryB++){
   v2 Simplex[3];
   
   if(DoGJK(Simplex, Boundary, P, BoundaryB, EntityB->P, Delta)){
    OutTrigger->Trigger = EntityB;
   }
  }
 }
}

void
entity_manager::DoCollisionsRelative(physics_update_context *Context, physics_collision *OutCollision,
                                     collision_boundary *Boundary, v2 P, v2 Delta, 
                                     entity *EntityA, physics_layer_flags Layer, u32 StartIndex){
 for(u32 I=StartIndex; I<Context->Updates.Count; I++){
  physics_update *UpdateB = &Context->Updates[I];
  // NOTE(Tyler): The assumption here is that an updated entity is active
  entity *EntityB = UpdateB->Entity;
  if(!(UpdateB->Layer & Layer)) continue;
  
  v2 RelativeDelta = Delta-UpdateB->Delta;
  
  if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, EntityB->Bounds, EntityB->P, RelativeDelta)){
   continue;
  }
  
  for(collision_boundary *BoundaryB = EntityB->Boundaries;
      BoundaryB < EntityB->Boundaries+EntityB->BoundaryCount;
      BoundaryB++){
   physics_collision Collision = DoCollision(Boundary, P, BoundaryB, EntityB->P, RelativeDelta);
   Collision.EntityB = EntityB;
   if(ChooseCollision(OutCollision, &Collision)){
    *OutCollision = Collision;
    UpdateB->Collision = MakeOtherCollision(EntityA, &Collision);
   }
  }
 }
 
 FOR_EACH_ENTITY(this){
  if(!(It.Item->TypeFlags & EntityTypeFlag_Dynamic)) continue;
  entity *EntityB = It.Item;
  if(EntityB == EntityA) continue;
  if(EntityB->Update) continue;
  if(!(ENTITY_TYPE_LAYER_FLAGS[It.CurrentArray] & Layer)) continue;
  
  if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, EntityB->Bounds, EntityB->P, Delta)){
   continue;
  }
  
  for(collision_boundary *BoundaryB = EntityB->Boundaries;
      BoundaryB < EntityB->Boundaries+EntityB->BoundaryCount;
      BoundaryB++){
   physics_collision Collision = DoCollision(Boundary, P, BoundaryB, EntityB->P, Delta);
   Collision.EntityB = EntityB;
   if(ChooseCollision(OutCollision, &Collision)){
    *OutCollision = Collision;
   }
  }
 }
}

// NOTE(Tyler): No starting at certain indices
void
entity_manager::DoCollisionsNotRelative(physics_update_context *Context, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer){
 FOR_EACH_ENTITY(this){
  if(!(It.Item->TypeFlags & EntityTypeFlag_Dynamic)) continue;
  entity *EntityB = It.Item;
  if(EntityB == EntityA) continue;
  if(!(ENTITY_TYPE_LAYER_FLAGS[It.CurrentArray] & Layer)) continue;
  
  if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, EntityB->Bounds, EntityB->P, Delta)){
   continue;
  }
  
  for(collision_boundary *BoundaryB = EntityB->Boundaries;
      BoundaryB < EntityB->Boundaries+EntityB->BoundaryCount;
      BoundaryB++){
   physics_collision Collision = DoCollision(Boundary, P, BoundaryB, EntityB->P, Delta);
   Collision.EntityB = EntityB;
   if(ChooseCollision(OutCollision, &Collision)){
    *OutCollision = Collision;
   }
  }
 }
}

//~ Do physics

void
entity_manager::DoFloorRaycast(physics_update_context *Context, entity *Entity, physics_layer_flags Layer, f32 Depth=5.0f){
 if(PhysicsDebugger.DefineStep()) return;
 if(PhysicsDebugger.IsCurrent()){
  PhysicsDebugger.DrawString("Floor raycast");
 }
 
 v2 Raycast = V2(0, -Depth);
 physics_collision Collision = MakeCollision();
 for(collision_boundary *Boundary = Entity->Boundaries;
     Boundary < Entity->Boundaries+Entity->BoundaryCount;
     Boundary++){
  DoStaticCollisions(&Collision, Boundary, Entity->P, Raycast);
  DoCollisionsNotRelative(Context, &Collision, Boundary, Entity->P, Raycast, Entity, Layer);
 }
 
 if(PhysicsDebugger.DefineStep()) return;
 
 if(Collision.TimeOfImpact >= 1.0f){ 
  if(PhysicsDebugger.IsCurrent()){ PhysicsDebugger.DrawString("No floor, too far"); }
  Entity->PhysicsFlags |= PhysicsStateFlag_Falling;
  return;
 }
 if(Collision.Normal.Y < WALKABLE_STEEPNESS) { 
  if(PhysicsDebugger.IsCurrent()){ PhysicsDebugger.DrawString("No floor, too steep"); }
  return; 
 }
 
 Entity->Parent = Collision.EntityB;
 
 Entity->P += Raycast*Collision.TimeOfImpact;
 Entity->P += Collision.Correction;
 
 Entity->dP       -= Collision.Normal*Dot(Entity->dP, Collision.Normal);
 Entity->TargetdP -= Collision.Normal*Dot(Entity->TargetdP, Collision.Normal);
 Entity->FloorNormal = Collision.Normal;
 
 
 if(PhysicsDebugger.IsCurrent()){
  entity *EntityB = Collision.EntityB;
  PhysicsDebugger.DrawString("Yes floor");
  PhysicsDebugger.DrawPoint(Entity->P, V2(0), WHITE);
  PhysicsDebugger.DrawPoint(EntityB->P, V2(0), DARK_GREEN);
  PhysicsDebugger.DrawNormal(EntityB->P, V2(0), Collision.Normal, PINK);
  PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision.TimeOfImpact);
  PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision.Correction.X, Collision.Correction.Y);
 }
}

void
entity_manager::DoPhysics(physics_update_context *Context){
 TIMED_FUNCTION();
 
 PrepareUpdateContext(Context, &TransientStorageArena);
 
 local_constant f32 Epsilon = 0.0001f;
 
 // DEBUG
 PhysicsDebugger.Begin();
 
 f32 dTime = OSInput.dTime;
 
 physics_trigger_collision *Triggers = PushArray(&TransientStorageArena, physics_trigger_collision, Context->Updates.Count);
 
#if 0 
 //~ DEBUG
 if(PhysicsDebugger.StartOfPhysicsFrame){
  Object->DebugInfo.P = Object->P;
  Object->DebugInfo.dP = Object->dP;
  Object->DebugInfo.ddP = Object->ddP;
  Object->DebugInfo.Delta = Object->Delta;
 }
 if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
  Object->P = Object->DebugInfo.P;
  Object->dP = Object->DebugInfo.dP;
  Object->ddP = Object->DebugInfo.ddP;
  Object->Delta = Object->DebugInfo.Delta;
 }
#endif
 
 // TODO(Tyler): Move this into physics_debugger
 if(PhysicsDebugger.StartOfPhysicsFrame){
  PhysicsDebugger.StartOfPhysicsFrame = false;
 }
 
 //~ DEBUG
 
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
 if(DebugConfig.Overlay & DebugOverlay_Boundaries){
  FOR_EACH_ENTITY(this){
   entity *Entity = It.Item;
   if(Entity->Type == EntityType_Tilemap) continue;
   RenderEntityPhysics(Entity);
  }
 }
#endif
 
 //~ Do collisions
 u32 IterationsToDo = 8;
 f32 FrameTimeRemaining = 1.0f;
 u32 Iteration = 0;
 while((FrameTimeRemaining > 0) &&
       (Iteration < IterationsToDo)){
  Iteration++;
  
  f32 CurrentTimeOfImpact = 1.0f;
  
  //~ Detect collisions
  for(u32 I=0; I<Context->Updates.Count; I++){
   physics_update *Update = &Context->Updates[I];
   entity *Entity = Update->Entity;
   Update->Collision = MakeCollision();
   
   local_constant f32 Epsilon = 0.01f;
   if((-Epsilon <= Update->Delta.X) && (Update->Delta.X <= Epsilon) &&
      (-Epsilon <= Update->Delta.Y) && (Update->Delta.Y <= Epsilon)){
    Update->Delta = {};
    continue;
   }
   
   if((-Epsilon <= Entity->dP.X) && (Entity->dP.X <= Epsilon) &&
      (-Epsilon <= Entity->dP.Y) && (Entity->dP.Y <= Epsilon)){
    Entity->dP = {};
    continue;
   }
   
   //~ DEBUG
   
   physics_trigger_collision Trigger = {};
   for(collision_boundary *Boundary = Entity->Boundaries;
       Boundary < Entity->Boundaries+Entity->BoundaryCount;
       Boundary++){
    DoStaticCollisions(&Update->Collision, Boundary, Entity->P, Update->Delta);
    DoCollisionsRelative(Context, &Update->Collision, Boundary, Entity->P, Update->Delta, Entity, I+1);
    DoTriggerCollisions(&Trigger, Boundary, Entity->P, Update->Delta);
   }
   
   Triggers[I] = Trigger;
   if(Update->Collision.TimeOfImpact < CurrentTimeOfImpact){
    CurrentTimeOfImpact = Update->Collision.TimeOfImpact;
    for(u32 J=0; J<I; J++){
     Triggers[J]   = {};
    }
   }
  }
  
  //~ Solve collisions
  for(u32 I=0; I<Context->Updates.Count; I++){
   physics_update *Update = &Context->Updates[I];
   entity *Entity = Update->Entity;
   physics_collision *Collision = &Update->Collision;
   
   f32 COR = 1.0f;
   FrameTimeRemaining -= FrameTimeRemaining*CurrentTimeOfImpact;
   
   if(PhysicsDebugger.DefineStep()) return;
   
   Entity->P += CurrentTimeOfImpact*Update->Delta;
   Update->Delta -= Update->Delta*CurrentTimeOfImpact;
   if(PhysicsDebugger.IsCurrent()){
    PhysicsDebugger.DrawPoint(Entity->P, V2(0), YELLOW);
   }
   
   if(Triggers[I].Trigger){
    entity *Trigger = Triggers[I].Trigger;
    
    Trigger->TriggerResponse(Trigger, Entity);
    if(PhysicsDebugger.IsCurrent()){
     PhysicsDebugger.DrawString("Entity hit trigger");
    }
   }
   
   if(Collision->TimeOfImpact < 1.0f){
    if(Entity->Response(Update, Collision)){
     if(PhysicsDebugger.IsCurrent()){
      PhysicsDebugger.DrawString("Entity handled collision");
     }
    }else{
     if(Dot(Update->Delta, Update->Collision.Normal) < 0.0f){
      Entity->dP       -= COR*Collision->Normal*Dot(Entity->dP, Collision->Normal);
      Entity->TargetdP -= COR*Collision->Normal*Dot(Entity->TargetdP, Collision->Normal);
      
      v2 DeltaCorrection = COR*Collision->Normal*Dot(Update->Delta, Collision->Normal);
      Update->Delta    -= DeltaCorrection;
      Context->DeltaCorrections[Update->ChildID] = DeltaCorrection;
     }
     
     Entity->P += 0.5f*Collision->Correction;
     
     if(Collision->Normal.Y > WALKABLE_STEEPNESS){
      Entity->PhysicsFlags &= ~PhysicsStateFlag_Falling;
     }
     
     //~ DEBUG
     if(PhysicsDebugger.IsCurrent()){
      PhysicsDebugger.DrawString("Yes collision");
      PhysicsDebugger.DrawPoint(Entity->P-0.5f*Collision->Correction, V2(0), YELLOW);
      PhysicsDebugger.DrawPoint(Entity->P, V2(0), WHITE);
      entity *EntityB = Collision->EntityB;
      if(EntityB) { 
       PhysicsDebugger.DrawPoint(EntityB->P, V2(0), DARK_GREEN);
       PhysicsDebugger.DrawNormal(EntityB->P, V2(0), Collision->Normal, PINK);
      }
      PhysicsDebugger.DrawString("CurrentTimeOfImpact: %f", CurrentTimeOfImpact);
      PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision->TimeOfImpact);
      PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision->Correction.X, Collision->Correction.Y);
      PhysicsDebugger.DrawString("Along delta: %f", Collision->AlongDelta);
     }
    }
    
    Update->Collision = MakeCollision();
   }
  }
  
  for(u32 I=0; I<Context->Updates.Count; I++){
   physics_update *Update = &Context->Updates[I];
   if(Update->ParentID){
    Update->Delta -= Context->DeltaCorrections[Update->ParentID];
   }
  }
  
  if(PhysicsDebugger.DefineStep()) return;
 }
 
 //~ Do floor raycasts
 FOR_EACH_ENTITY(this){
  if(!(It.Item->TypeFlags & EntityTypeFlag_Dynamic)) continue;
  entity *Entity = It.Item;
  Entity->Parent = 0;
  if((Entity->PhysicsFlags & PhysicsStateFlag_DontFloorRaycast) ||
     (Entity->PhysicsFlags & PhysicsStateFlag_Falling)){
  }else{
   DoFloorRaycast(Context, Entity, ENTITY_TYPE_LAYER_FLAGS[It.CurrentArray]);
  }
  
  if(PhysicsDebugger.DefineStep()) return;
 }
 
 //~ Do particles
 
 // TODO(Tyler): Move the particle system somewhere else
#if 0    
#define RepeatExpr(V) F32X4(V, V, V, V)
 
 // TODO(Tyler): This isn't a very good particle system. We might want 
 // particle collisions though which might be hard with this system
 FOR_BUCKET_ARRAY(It, &ParticleSystems){
  physics_particle_system *System = It.Item;
  f32 COR = System->COR;
  
  for(u32 I=0; I < System->Particles.Count; I++){
   physics_particle_x4 *Particle = &System->Particles[I];
   
   f32 BaseLifetime = 0.5f;
   f32_x4 BaseLifetimeX4 = F32X4(BaseLifetime);
   
   f32_x4 dTimeX4 = F32X4(dTime);
   f32_x4 DragCoefficient = F32X4(0.7f);
   v2_x4 ddP = V2X4(F32X4(0.0f), F32X4(-11.0f));
   ddP.X += -DragCoefficient*Particle->dP.X*AbsoluteValue(Particle->dP.X);
   ddP.Y += -DragCoefficient*Particle->dP.Y*AbsoluteValue(Particle->dP.Y);
   v2_x4 Delta = (dTimeX4*Particle->dP + 
                  F32X4(0.5f)*Square(dTimeX4)*ddP);
   Particle->dP += dTimeX4*ddP;
   
   Particle->P += Delta;
   
   v2_x4 StartP = V2X4(System->P);
   v2_x4 StartdP = V2X4(System->StartdP);
   
   u32 Seed = (u32)(u64)Particle;
   // TODO(Tyler): The random number generation is awful 
   // and could be a good place to speed up!
   f32_x4 RandomPX = StartP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 12, 0.1f));
   f32_x4 RandomPY = StartP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 13, 0.1f));
   f32_x4 RandomdPX = StartdP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 19, 0.2f));
   f32_x4 RandomdPY = StartdP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 17, 0.2f));
   f32_x4 RandomLifetimes = BaseLifetimeX4 + RepeatExpr(GetRandomFloat(Seed+=Seed, 5, 0.2f));
   
   __m128 M = _mm_cmple_ps(Particle->Lifetime.V, _mm_setzero_ps());
   Particle->Lifetime.V = _mm_or_ps(_mm_and_ps(M, RandomLifetimes.V), 
                                    _mm_andnot_ps(M, Particle->Lifetime.V));
   Particle->P.X.V = _mm_or_ps(_mm_and_ps(M, RandomPX.V), 
                               _mm_andnot_ps(M, Particle->P.X.V));
   Particle->P.Y.V = _mm_or_ps(_mm_and_ps(M, RandomPY.V), 
                               _mm_andnot_ps(M, Particle->P.Y.V));
   Particle->dP.X.V = _mm_or_ps(_mm_and_ps(M, RandomdPX.V), 
                                _mm_andnot_ps(M, Particle->dP.X.V));
   Particle->dP.Y.V = _mm_or_ps(_mm_and_ps(M, RandomdPY.V), 
                                _mm_andnot_ps(M, Particle->dP.Y.V));
   Particle->Lifetime -= dTimeX4;
   
   // TODO(Tyler): The rendering here is one of the slowest parts
   for(u32 I=0; I < 4; I++){
    f32 Lifetime = GetOneF32(Particle->Lifetime, I);
    if(Lifetime > 0.0f){
     v2 P = V2(GetOneF32(Particle->P.X, I), GetOneF32(Particle->P.Y, I));
     color C = Color(0.6f, 0.5f, 0.3f, 1.0f);
     C = Alphiphy(C, Lifetime/BaseLifetime);
     RenderRect(CenterRect(P, V2(0.07f)), -10.0f, C, &GameCamera);
    }
   }
   
  }
 }
#undef RepeatExpr
#endif
 
 //~ DEBUG
 PhysicsDebugger.End();
}