-- =======================================================================
--                       Barbarians campaign mission 1
-- =======================================================================

set_textdomain("scenario_t01.wmf")

-- ===============
-- Initialization
-- ===============

p = wl.Game().players[1]

-- Only lumberjack buildings are allowed
p:forbid_buildings("all")
p:allow_buildings{"lumberjacks_hut"}

-- Place the headquarters & fill it with wares
hq = p:place_building("headquarters_interim", wl.Game().map:get_field(12,10),
   false, true)
hq:set_wares{
   log = 80
}
hq:set_workers{
   builder=10,
   carrier=40,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2
}

-- ==========
-- Constants
-- ==========
home = wl.Game().map:get_field(12,10)
al_thunran = wl.Game().map:get_field(53, 43)
grave = wl.Game().map:get_field(25,22)

-- This function can move to a place, display a modal message box and return
function show_story_box(t, m, pos, gposx, gposy)
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

-- Add an objective
function add_obj(objective, player)
   return player:add_objective(objective.name, objective.title, objective.body)
end

include "scripting/coroutine.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

include "map:scripting/texts.lua"

include "map:scripting/initial_messages.lua"
include "map:scripting/story_messages.lua"
