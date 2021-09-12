
internal void
UpdateAndRenderDebug(){
 TIMED_FUNCTION();
 
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  ProcessDefaultEvent(&Event);
 }
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
 GameRenderer.SetLightingConditions(WHITE, 1.0f);
 GameRenderer.SetCameraSettings(0.5f);
 
 asset_sprite_sheet *Asset = AssetSystem.GetSpriteSheet(Strings.GetString("player"));
 asset_animation    *Animation = Strings.GetInHashTablePtr(&AssetSystem.Animations, "enemy");
 local_persist animation_state State = {};
 State.State     = State_Moving;
 State.Direction = Direction_Left;
 
 UpdateSpriteSheetAnimation(Asset, Animation, &State);
 RenderSpriteSheetAnimation(Asset, Animation, &State, V2(50), 10.0f, 1);
 //RenderSpriteSheetAnimationFrame(Asset, V2(10), 10.0f, 1, 0, 0);
 
}
