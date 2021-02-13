
collision_boundary *WallBoundary;
collision_boundary *CoinBoundary;
collision_boundary *ProjectileBoundary;

collision_boundary *DEBUGPlayerBoundary;
collision_boundary *DEBUGWedgeBoundary;
collision_boundary *DEBUGCircleBoundary;

//~ Boundary management
internal inline b8
IsPointInBoundary(v2 Point, collision_boundary *Boundary, v2 Base=V2(0,0)){
    b8 Result = false;
    switch(Boundary->Type){
        case BoundaryType_None: break;
        case BoundaryType_Rect: {
            Result = IsV2InRectangle(Point, OffsetRect(Boundary->Bounds, Base+Boundary->Offset));
        }break;
        case BoundaryType_Circle: {
            f32 Distance = SquareRoot(LengthSquared(Point-(Base+Boundary->Offset)));
            Result = (Distance <= Boundary->Radius);
        }break;
        case BoundaryType_Wedge: {
            NOT_IMPLEMENTED_YET
        }break;
        default: INVALID_CODE_PATH;
    }
    return(Result);
}

internal inline void
RenderBoundary(camera *Camera, collision_boundary *Boundary, f32 Z, v2 Offset){
    switch(Boundary->Type){
        case BoundaryType_None: break;
        case BoundaryType_Rect: {
            v2 P = Offset + Boundary->Offset;
            RenderRectangle(P+Boundary->Bounds.Min, P+Boundary->Bounds.Max, Z, 
                            Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
        case BoundaryType_Circle: {
            RenderCircle(Offset+Boundary->Offset, Boundary->Radius, Z,
                         Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
        default: INVALID_CODE_PATH;
    }
}

internal inline collision_boundary
MakeCollisionRect(v2 Offset, v2 Size){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Rect;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V20, Size);
    return(Result);
}

internal inline collision_boundary
MakeCollisionCircle(v2 Offset, f32 Radius){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Circle;
    Result.Offset = Offset;
    Result.Radius = Radius;
    Result.Bounds = CenterRect(V20, 2*V2(Radius, Radius));
    return(Result);
}

//~ Spacial hashing
internal void
CollisionSystemNewFrame(){
    TIMED_FUNCTION();
    
#if 0    
    
    CollisionTable.Items = PushArray(&TransientStorageArena, collision_table_item *, 
                                     CollisionTable.Width*CollisionTable.Height);
    ZeroMemory(CollisionTable.Items, 
               CollisionTable.Width*CollisionTable.Height*sizeof(collision_table_item));
    
    FOR_BUCKET_ARRAY(&EntityManager.Walls){
        wall_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Walls, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Wall;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Coins){
        coin_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Coins, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Coin;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Doors){
        door_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Doors, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Door;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Enemies){
        enemy_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Enemies, Location);
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
    
    
    FOR_BUCKET_ARRAY(&EntityManager.Projectiles){
        projectile_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Projectiles, Location);
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = EntityType_Projectile;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
    
    // Player
    {
        player_entity *Entity = EntityManager.Player;
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
#endif
    
}

//~ Physics

void
physics_system::Initialize(memory_arena *Arena){
    BucketArrayInitialize(&Objects, Arena);
    BucketArrayInitialize(&StaticObjects, Arena);
    BoundaryMemory = PushNewArena(Arena, 512*sizeof(collision_boundary));
    
    WallBoundary = PhysicsSystem.AllocBoundaries(1);
    *WallBoundary = MakeCollisionRect(V20, TILE_SIZE);
    //*WallBoundary = MakeCollisionCircle(V20, 0.75f*TILE_SIDE);
    
    CoinBoundary = PhysicsSystem.AllocBoundaries(1);
    *CoinBoundary = MakeCollisionRect(V20, V2(0.3f, 0.3f));
    
    ProjectileBoundary = PhysicsSystem.AllocBoundaries(1);
    *ProjectileBoundary = MakeCollisionRect(V20, V2(0.1f, 0.1f));
    
    DEBUGPlayerBoundary = PhysicsSystem.AllocBoundaries(1);
    *DEBUGPlayerBoundary = MakeCollisionRect(V20, TILE_SIZE);
    
    DEBUGWedgeBoundary = PhysicsSystem.AllocBoundaries(1);
    DEBUGWedgeBoundary->Type = BoundaryType_Wedge;
    DEBUGWedgeBoundary->Bounds.Min = V2(-0.5f, 0.0f);
    DEBUGWedgeBoundary->Bounds.Max = V2( 0.0f, 0.5f);
    DEBUGWedgeBoundary->Points[0] = V2(-0.5f,  0.0f);
    DEBUGWedgeBoundary->Points[1] = V2( 0.0f,  0.0f);
    DEBUGWedgeBoundary->Points[2] = V2( 0.0f,  0.5f);
    
    DEBUGCircleBoundary = PhysicsSystem.AllocBoundaries(1);
    DEBUGCircleBoundary->Type = BoundaryType_Circle;
    DEBUGCircleBoundary->Bounds.Min = V2(-0.5f, 0.0f);
    DEBUGCircleBoundary->Bounds.Max = V2( 0.0f, 0.5f);
    DEBUGCircleBoundary->Radius = 0.5f;
}

void
physics_system::Reload(u32 Width, u32 Height){
    SpaceTable.Width = Truncate((f32)Width / TILE_SIDE);
    SpaceTable.Height = Truncate((f32)Height / TILE_SIDE);
    BucketArrayRemoveAll(&Objects);
    BucketArrayRemoveAll(&StaticObjects);
}

physics_object *
physics_system::AddObject(collision_boundary *Boundaries, u8 Count){
    physics_object *Result = BucketArrayAlloc(&Objects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    
    return(Result);
}

physics_object *
physics_system::AddStaticObject(collision_boundary *Boundaries, u8 Count){
    physics_object *Result = BucketArrayAlloc(&StaticObjects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    
    return(Result);
}

collision_boundary *
physics_system::AllocBoundaries(u32 Count){
    collision_boundary *Result = PushArray(&BoundaryMemory, collision_boundary, Count);
    return(Result);
}

//~ GJK stuff

internal inline v2
GJKSupport(collision_boundary *Boundary, 
           physics_object *Object, v2 Delta,
           v2 Direction){
    Assert((Direction.X != 0.0f) || (Direction.Y != 0.0f));
    v2 Result = {};
    
    switch(Boundary->Type){
        case BoundaryType_Rect: {
            v2 Min = Boundary->Bounds.Min;
            v2 Max = Boundary->Bounds.Max;
            v2 Points[4] = {
                Min,
                V2(Min.X, Max.Y),
                Max,
                V2(Max.X, Min.Y),
            };
            for(u32 I=0; I < 4; I++){
                if(Dot(Points[I], Direction) > Dot(Result, Direction)){
                    Result = Points[I];
                }
            }
        }break;
        case BoundaryType_Circle: {
            // TODO(Tyler): There might be a better way to do this
            f32 DirectionLength = Length(Direction);
            v2 UnitDirection = Direction/DirectionLength;
            Result = UnitDirection * Boundary->Radius;
        }break;
        case BoundaryType_Wedge: {
            for(u32 I=0; I < 3; I++){
                v2 Point = Boundary->Points[I];
                if(Dot(Point, Direction) > Dot(Result, Direction)){
                    Result = Point;
                }
            }
        }break;
        default: INVALID_CODE_PATH;
    }
    
    Result += Object->P;
    if(Dot(Delta, Direction) > 0.0f){
        Result += Delta;
    }
    
    return(Result);
}

internal inline v2 
CalculateSupport(physics_object *ObjectA, physics_object *ObjectB, v2 Delta, v2 Direction){
    // TODO(Tyler): Currently this only supports single collision boundaries
    v2 Result = GJKSupport(ObjectA->Boundaries, ObjectA, Delta, Direction) - GJKSupport(ObjectB->Boundaries, ObjectB, V20, -Direction);
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
                    *Direction = TripleProduct(Simplex[0]-Simplex[1], V2(0.01f, 0.0f)-Simplex[1]);
                }
            }else{
                *Direction = -Simplex[1];
                Simplex[0] = Simplex[1];
                *SimplexCount = 1;
            }
        }break;
        case 3: {
            // TODO(Tyler): This could probably be made more efficient, by using less
            // ifs and &&s
            
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
            }else if(OutsideP2P0 && !OutsideP2P1 && AlongP2P0){
                // Area 6
                Simplex[1] = Simplex[2];
                *SimplexCount = 2;
                *Direction = P2P0Direction;
            }else if(OutsideP2P0 && OutsideP2P1 && !AlongP2P0 && !AlongP2P0){
                // Area 5
                Simplex[0] = Simplex[2];
                *SimplexCount = 1;
                *Direction = -Simplex[0];
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

struct epa_result {
    f32 ActualTimeOfImpact;
    f32 WorkingTimeOfImpact;
    v2 Normal;
};

#if 0
internal epa_result
DoEPA(physics_object *ObjectA, physics_object *ObjectB, v2 Simplex[3]){
    
    dynamic_array<v2> Polytope; 
    DynamicArrayInitialize(&Polytope, 20, &TransientStorageArena);
    DynamicArrayPushBack(&Polytope, Simplex[0]);
    DynamicArrayPushBack(&Polytope, Simplex[1]);
    DynamicArrayPushBack(&Polytope, Simplex[2]);
    
    epa_result Result = {};
    // TODO(Tyler): If the normalization ever becomes a problem it could be cached
    // though that might not be needed
    while(true){
        f32 Closest = F32_POSITIVE_INFINITY;
        u32 ClosestIndex = 0;
        v2 Normal = {};
        for(u32 I=0; I < Polytope.Count; I++){
            v2 A = Polytope[I];
            v2 B = Polytope[(I+1)%Polytope.Count];
            // NOTE(Tyler): This must be normalized
            v2 InverseNormal = TripleProduct(B-A, -A);
            InverseNormal = Normalize(InverseNormal);
            f32 Distance = Dot(InverseNormal, -A);
            if(Distance < Closest){
                Closest = Distance;
                ClosestIndex = I;
                Normal = Invert(InverseNormal);
            }
        }
        Assert(Closest != F32_POSITIVE_INFINITY);
        
        v2 NewPoint = CalculateSupport(ObjectA, ObjectB, Normal);
        f32 Epsilon = 0.0001f;
        f32 Distance = Dot(NewPoint, Normal);
        if((-Epsilon <= (Distance-Closest)) && ((Distance-Closest) <= Epsilon)){
            Result = Normal*Distance;
            break;
        }else{
            DynamicArrayInsertNewArrayItem(&Polytope, ClosestIndex+1, NewPoint);
        }
    }
    
    return(Result);
}
#endif

// Expanding polytope algorithm that expands the poltope in the direction of the difference of the velocities,
// This requires a GJK with a sweeping support function
internal epa_result
DoVelocityEPA(physics_object *ObjectA, physics_object *ObjectB, v2 Delta, v2 Simplex[3]){
    TIMED_FUNCTION();
    //LogMessage("%f, DoVelocityEPA", Counter);
    
    const f32 Epsilon = 0.0001f;
    
    dynamic_array<v2> Polytope; 
    DynamicArrayInitialize(&Polytope, 20, &TransientStorageArena);
    DynamicArrayPushBack(&Polytope, Simplex[0]);
    DynamicArrayPushBack(&Polytope, Simplex[1]);
    DynamicArrayPushBack(&Polytope, Simplex[2]);
    
    v2 DeltaNormal = Normalize(Clockwise90(Delta));
    
    epa_result Result = {};
    // TODO(Tyler): If the normalization ever becomes a problem it could be cached
    // though that might not be needed
    while(true){
        b8 FoundEdge = false;
        b8 IsColinear = false;
        u32 EdgeIndex = 0;
        f32 EdgeDistance = 0;
        v2 InverseNormal = {};
        v2 IntersectionPoint = {};
        
        for(u32 I=0; I < Polytope.Count; I++){
            v2 A = Polytope[I];
            v2 B = Polytope[(I+1)%Polytope.Count];
            
            f32 AAlongDeltaNormal = Dot(DeltaNormal, A);
            f32 BAlongDeltaNormal = Dot(DeltaNormal, B);
            
            //LogMessage("   %f %u, Delta: (%f, %f), A: (%f, %f) %f, B: (%f, %f) %f", 
            //Counter, I, Delta.X, Delta.Y, A.X, A.Y, AAlongDeltaNormal, 
            //B.X, B.Y, BAlongDeltaNormal);
            
            if(((-Epsilon <= AAlongDeltaNormal) && (AAlongDeltaNormal <= Epsilon)) && // Collinear
               ((-Epsilon <= BAlongDeltaNormal) && (BAlongDeltaNormal <= Epsilon))){
                FoundEdge = true;
                IsColinear = true;
                EdgeIndex = I;
                InverseNormal = Normalize(TripleProduct(B-A, -A));
                EdgeDistance = Dot(InverseNormal, -A);
                //IntersectionPoint = Delta;
                IntersectionPoint = V20;
                
                //LogMessage("      %f, Colinear", Counter);
                
                //break;
            }else if(((AAlongDeltaNormal >= 0) && (BAlongDeltaNormal <= 0)) || 
                     ((AAlongDeltaNormal <= 0) && (BAlongDeltaNormal >= 0))){ // The delta line  intersects
                AAlongDeltaNormal = AbsoluteValue(AAlongDeltaNormal);
                BAlongDeltaNormal = AbsoluteValue(BAlongDeltaNormal);
                f32 ABPercent = AAlongDeltaNormal/(AAlongDeltaNormal + BAlongDeltaNormal);
                v2 Point = A + ABPercent*(B-A);
                
                v2 DeltaDirection = Normalize(Delta);
                f32 PointAlongDeltaDirection = Dot(DeltaDirection, Point);
                f32 DeltaLength = Dot(DeltaDirection, Delta);
                
                //LogMessage("      %f, Intersects", Counter);
                
                if((-Epsilon <= PointAlongDeltaDirection) && 
                   (PointAlongDeltaDirection <= DeltaLength + Epsilon)){ // The delta intersects
                    FoundEdge = true;
                    EdgeIndex = I;
                    InverseNormal = Normalize(TripleProduct(B-A, -A));
                    EdgeDistance = Dot(InverseNormal, -A);
                    IntersectionPoint = Point;
                    
                    break;
                }else if(-Epsilon <= PointAlongDeltaDirection){ // The  delta ray intersects
                    if(!IsColinear){
                        FoundEdge = true;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        IntersectionPoint = Point;
                    }
                    
                    //LogMessage("         %f, Almost %f %f", Counter, PointAlongDeltaDirection, DeltaLength);
                }else{
                    //LogMessage("         %f, Actually no %f %f", Counter, PointAlongDeltaDirection, DeltaLength);
                }
            }
        }
        Assert(FoundEdge);
        
        f32 Distance;
        v2 NewPoint = V20;
        if((InverseNormal.X == 0.0f) && (InverseNormal.Y == 0.0f)){
            Distance = EdgeDistance;
        }else{
            v2 Normal = -InverseNormal;
            NewPoint = CalculateSupport(ObjectA, ObjectB, Delta, Normal);
            Distance = Dot(NewPoint, Normal);
        }
        if((-Epsilon <= (Distance-EdgeDistance)) && ((Distance-EdgeDistance) <= Epsilon)){
            f32 Percent = IntersectionPoint.X/Delta.X;
            if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                Percent = 0.0f;
            }else if(Delta.X == 0.0f){
                Percent = IntersectionPoint.Y/Delta.Y;
            }
            
            //Percent += Epsilon;
            //if(Percent > 1.0f){ Percent = 1.0f; }
            
            Result.WorkingTimeOfImpact = 1.0f - Percent;
            if(Result.WorkingTimeOfImpact > 1.0f){
                Result.WorkingTimeOfImpact = 1.0f;
            }else if(Result.WorkingTimeOfImpact < 0.0f){
                Result.WorkingTimeOfImpact = 0.0f;
            }
            Result.ActualTimeOfImpact = 1.0f - Percent;
            
            //Result.IntersectionPoint = IntersectionPoint;
            Result.Normal = InverseNormal;
            
            //LogMessage("   %f, TOI: %f", Counter, Result.TimeOfImpact);
            
            break;
        }else{
            DynamicArrayInsertNewArrayItem(&Polytope, EdgeIndex+1, NewPoint);
        }
    }
    
    return(Result);
}

void
physics_system::DoPhysics(){
    TIMED_FUNCTION();
    CollisionSystemNewFrame();
    
    const f32 Epsilon = 0.0001f;
    
    FOR_BUCKET_ARRAY(ObjectA, &Objects){
        Renderer.BeginClipRegion(V20, V2(9, 4), &GameCamera);
        rect Rect = OffsetRect(ObjectA->Boundaries->Bounds, ObjectA->Boundaries->Offset+ObjectA->P);
        RenderRectangle(Rect.Min, Rect.Max, -10.0f, Color(1, 0, 0, 0.5f), &GameCamera);
        Renderer.EndClipRegion();
        
        // NOTE(Tyler): This may not be the best way to integrate dP, Delta, but it works
        f32 DragCoefficient = 0.7f;
        ObjectA->ddP.X += -DragCoefficient*ObjectA->dP.X*AbsoluteValue(ObjectA->dP.X);
        ObjectA->ddP.Y += -DragCoefficient*ObjectA->dP.Y*AbsoluteValue(ObjectA->dP.Y);
        v2 Delta = V20;
        Delta += (OSInput.dTime*ObjectA->dP + 
                  0.5f*Square(OSInput.dTime)*ObjectA->ddP);
        ObjectA->dP += OSInput.dTime*ObjectA->ddP;
        
        f32 FrameTimeRemaining = 1.0f;
        u32 Iteration = 0;
        while((FrameTimeRemaining > 0) &&
              (Iteration < MAX_PHYSICS_ITERATIONS)){
            Iteration++;
            
            if((Delta.X > -Epsilon) && (Delta.X < Epsilon) &&
               (Delta.Y > -Epsilon) && (Delta.Y < Epsilon)) break;
            
            dynamic_array<v2> CollidingSimplices;
            DynamicArrayInitialize(&CollidingSimplices, 8*3, &TransientStorageArena);
            dynamic_array<physics_object *> CollidingObjects;
            DynamicArrayInitialize(&CollidingObjects, 8, &TransientStorageArena);
            
            b8 DoesCollideAny = false;
            
            FOR_BUCKET_ARRAY(ObjectB, &StaticObjects){
#if 1
                RenderCircle(ObjectB->P, ObjectB->Boundaries->Radius, 
                             -1.0f, Color(1.0f, 0.0f, 0.0f, 0.7f), &GameCamera);
#else
                rect Rect = OffsetRect(ObjectB->Boundaries->Bounds, ObjectB->Boundaries->Offset+ObjectB->P);
                RenderRectangle(Rect.Min, Rect.Max, -10.0f, Color(1, 0, 0, 0.5f), &GameCamera);
#endif
                
                f32 ObjectBPAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
                v2 Simplex[3] = {}; 
                u32 SimplexCount = 1; // Account for the first support point
                v2 Direction = ObjectA->P - ObjectB->P;
                if((Direction.X == 0.0f) && (Direction.Y == 0.0f)){
                    Direction = V2(1, 0);
                }
                Simplex[0] = CalculateSupport(ObjectA, ObjectB, Delta, Direction);
                Direction = -Simplex[0];
                
                b8 DoesCollide = false;
                while(true){
                    v2 NewPoint = CalculateSupport(ObjectA, ObjectB, Delta, Direction);
                    if(Dot(NewPoint, Direction) < 0){ 
                        // The new point is in the wrong direction because there is no point going the right direction
                        break;
                    }
                    Simplex[SimplexCount] = NewPoint;
                    SimplexCount++;
                    if(UpdateSimplex(Simplex, &SimplexCount, &Direction)){
                        DoesCollide = true;
                        break;
                    }
                }
                
                if(DoesCollide){
                    DynamicArrayPushBack(&CollidingObjects,   ObjectB);
                    DynamicArrayPushBack(&CollidingSimplices, Simplex[0]);
                    DynamicArrayPushBack(&CollidingSimplices, Simplex[1]);
                    DynamicArrayPushBack(&CollidingSimplices, Simplex[2]);
                    DoesCollideAny = true;
                }
            }
            
            if(DoesCollideAny){
                epa_result Colliding = {};
                Colliding.ActualTimeOfImpact = F32_POSITIVE_INFINITY;
                f32 CollidingObjectDistanceAlongDelta = F32_POSITIVE_INFINITY;
                physics_object *DEBUGCollidingObject = 0;
                u32 DEBUGCollidingIndex;
                
                for(u32 I = 0; I < CollidingObjects.Count; I++){
                    physics_object *ObjectB = CollidingObjects[I];
                    v2 *Simplex = &CollidingSimplices[I*3];
                    epa_result EPAResult = DoVelocityEPA(ObjectA, ObjectB, Delta, Simplex);
                    if(EPAResult.ActualTimeOfImpact < Colliding.ActualTimeOfImpact){
                        Colliding = EPAResult;
                        CollidingObjectDistanceAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
                        DEBUGCollidingObject = ObjectB;
                        DEBUGCollidingIndex = I;
                    }else if(EPAResult.ActualTimeOfImpact == Colliding.ActualTimeOfImpact){
                        f32 ObjectBDistanceAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
                        if(ObjectBDistanceAlongDelta < CollidingObjectDistanceAlongDelta){
                            Colliding = EPAResult;
                            CollidingObjectDistanceAlongDelta = ObjectBDistanceAlongDelta;
                            DEBUGCollidingObject = ObjectB;
                            DEBUGCollidingIndex = I;
                        }
                    }
                }
                
                f32 COR = 1.0f;
                
                FrameTimeRemaining -= FrameTimeRemaining*Colliding.WorkingTimeOfImpact;
                ObjectA->P += Colliding.ActualTimeOfImpact*Delta;
                //LogMessage("%f, (%f, %f)", 
                //Counter, Colliding.Correction.X, Colliding.Correction.Y);
                v2 OlddP = ObjectA->dP;
                ObjectA->dP = ObjectA->dP - COR*Dot(Colliding.Normal, ObjectA->dP)*Colliding.Normal;
                Delta = Delta - COR*Dot(Colliding.Normal, Delta)*Colliding.Normal;
            }else{
                FrameTimeRemaining -= FrameTimeRemaining;
                ObjectA->P += Delta;
            }
        }
        
        ObjectA->ddP = {};
    }
}

inline void
entity_manager::DoPhysics(){
    PhysicsSystem.DoPhysics();
}