
internal void
UpdateAndRenderDebug(){
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  ProcessDefaultEvent(&Event);
 }
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, Color(0.4f, 0.5f, 0.45f, 1.0f));
 GameRenderer.SetLightingConditions(WHITE, 1.0f);
 GameRenderer.SetCameraSettings(0.5f);
 
 asset_entity *Entity = Strings.FindInHashTablePtr(&AssetSystem.Entities, "snail");
 
 local_persist animation_state Animation = {};
 if(Animation.State == State_None){
  ChangeAnimationState(&Entity->Animation, &Animation, State_Moving);
  Animation.Direction = Direction_Left;
 }
 
 
 v2 P = V2(50.0f, 50.0f);
 
 for(u32 I=0; I < Entity->BoundaryCount; I++){
  rect R = Entity->Boundaries[I].Bounds;
  R += Entity->Boundaries[I].Offset;
  R += P;
  RenderRectOutline(R, -10.0f, Color(0.0f, 0.8f, 0.8f), ScaledItem(1), 0.5f);
 }
 RenderRect(CenterRect(P, V2(1)), -10.0f, RED, ScaledItem(1));
 
 asset_sprite_sheet *Sheet = Entity->Pieces[0];
 
 {
  //f32 Y = -0.5f*Sheet->FrameSize.Height + Sheet->YOffset;
  f32 Y = Sheet->YOffset;
  v2 A = V2(-100.0f, Y-1.0f) + P;
  v2 B = V2( 100.0f, Y) + P;
  RenderRect(Rect(A, B), -10.0f, PINK, ScaledItem(1));
 }
 
 DoEntityAnimation(Entity, &Animation, P, -11.0f);
}
