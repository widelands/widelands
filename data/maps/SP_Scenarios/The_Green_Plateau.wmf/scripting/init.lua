-- =======================================================================
--                             The Green Plateau
-- =======================================================================

push_textdomain("map_the_green_plateau.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]
p2 = wl.Game().players[2]
p3 = wl.Game().players[3]
p4 = wl.Game().players[4]
wl.Game().allow_diplomacy = false

include "map:scripting/texts.lua"
include "map:scripting/initial_conditions.lua"

-- Main mission thread
function mission_thread()
   sleep(300)

   campaign_message_box(briefing_1_the_forbidden_island)
   local o = add_campaign_objective(obj_forbidden_island)

   local map = wl.Game().map
   while not p1:seen_field(map:get_field(5,8)) do sleep(2345) end

   -- Unhide the castle and keep it unhidden so that it can be attacked
   local castle = map:get_field(23, 19)
   p1:reveal_fields(castle:region(4))

   -- Show the way to the castle very briefly
   local way = array_combine(
      map:get_field( 5,  8):region(2),
      map:get_field( 9, 10):region(2),
      map:get_field(12, 12):region(2),
      map:get_field(15, 14):region(2),
      map:get_field(18, 16):region(2),
      map:get_field(20, 17):region(2)
   )
   p1:reveal_fields(way)
   sleep(100)
   p1:hide_fields(way)

   -- Move to the castle
   scroll_to_field(castle)

   campaign_message_box(briefing_2_found_ancient_castle)
   o.done = true
   o = add_campaign_objective(obj_capture_ancient_castle)

   -- Wait till we conquered the castle
   while #p1:get_buildings"atlanteans_castle" < 1 do sleep(2345) end
   o.done = true

   scroll_to_field(castle)

   p1:reveal_fields(castle:region(18))
   campaign_message_box(briefing_3_captured_ancient_castle)
   local o_erwyn = add_campaign_objective(obj_defeat_erwyn)
   local o_jomo = add_campaign_objective(obj_defeat_jomo)

   sleep(100)
   p1:hide_fields(castle:region(18))

   -- Function to check for victory over erwyn
   run(function()
      while not p2.defeated do sleep(3434) end
      campaign_message_box(briefing_erwyn_defeated)
      o_erwyn.done = true
   end)
   -- Function to check for victory over jomo
   run(function()
      while not p3.defeated do sleep(3434) end
      campaign_message_box(briefing_jomo_defeated)
      o_jomo.done = true
   end)

   while not (p2.defeated and p3.defeated) do sleep(4325) end
   sleep(4325)

   campaign_message_box(last_briefing_victory)
end

run(mission_thread)
