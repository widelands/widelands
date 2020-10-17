-- NOCOM push_textdomain("scenario_tutorial01_basic_control.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"


run(function()

   print("This is lumberjack.lua")

   -- NOCOM get the actual interactive player
   local plr = wl.Game().players[1]
   local test_field = wl.Game().map.player_slots[1].starting_field

   lumberjack_message_01 = {
      title = _"Lumberjack’s Spot",
      position = "topright",
      body = (
         li_object("barbarians_lumberjacks_hut",
            [[We will want to build the tribe's log producer here]], plr.color)
      ),
      h = 250,
      w = 350
   }

   sleep(100)
   test_field:indicate(true)

   campaign_message_box(lumberjack_message_01)

   sleep(500)

   sleep(500)


   while #plr:get_buildings("barbarians_lumberjacks_hut") < 1 do sleep(300) end

   test_field:indicate(false)

   wl.Game().players[1]:mark_training_wheel_as_solved("lumberjack")
end)
