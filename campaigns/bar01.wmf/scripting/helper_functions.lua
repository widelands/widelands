-- =================
-- Helper functions
-- =================

function scroll_smoothly_nordwards_to(f, g_T)
   local mv = wl.ui.MapView()
   local x, y
   local map = wl.Game().map

   if math.abs(f.viewpoint_x - mv.viewpoint_x) <
      math.abs(f.viewpoint_x + 64 * map.width - mv.viewpoint_x)
   then
      x = f.viewpoint_x
   else
      x = f.viewpoint_x + map.width * 64
   end

   y = f.viewpoint_y - map.height * 32 -- because we want to go northwards

   return scroll_smoothly_to_pos(x - mv.width / 2, y - mv.height / 2, g_T)
end