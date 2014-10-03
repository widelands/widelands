-- =======================================================================
--                       Barbarians campaign mission 1
-- =======================================================================

set_textdomain("scenario_t01.wmf")

-- ===============
-- Initialization
-- ===============

p = wl.Game().players[1]

-- ==========
-- Constants
-- ==========
home = wl.Game().map:get_field(12,10)
al_thunran = wl.Game().map:get_field(53, 43)
grave = wl.Game().map:get_field(25,22)

-- This function can move to a place, display a modal message box and return
function show_story_box(t, m, pos, gposx, gposy)
	while (wl.ui.MapView().is_building_road) do sleep(2000) end
   plr = wl.Game().players[1]
   posx = gposx
   posy = gposy
   local pts = nil
   if pos ~= nil then
      plr:reveal_fields(pos:region(8))
      pts = scroll_smoothly_to(pos)

      posx = gposx or 0
      posy = gposy or 80
   end
   plr:message_box(t, m, { posx = posx, posy = posy, h = 400 })
   if pts then
      timed_scroll(array_reverse(pts))
   end
   sleep(500)
end

include "scripting/coroutine.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
include "map:scripting/story_messages.lua"
