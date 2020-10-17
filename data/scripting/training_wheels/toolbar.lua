include "scripting/coroutine.lua"
-- include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
-- include "scripting/table.lua"
include "scripting/ui.lua"

-- NOCOM get from the interactive player
plr = wl.Game().players[1]

run(function()
   print("This is toolbar.lua")

   push_textdomain("training_wheels")

   local initial_message_02 = {
      title = _"Building Spaces",
      position = "topright",
      body = (
         h1(_"Let’s dive right in!") ..
         li_object("barbarians_headquarters", _[[You will usually start the game with one headquarters. This is the big building with the blue flag in front of it. The headquarters is a warehouse that stores wares, workers and soldiers. Some wares are needed for building houses, others for making other wares. Obviously, the wares in the headquarters will not last forever, so you must make sure to replace them. The most important wares in the early game are the basic construction wares: logs and granite. Let’s make sure that we do not run out of logs. For this, we need a lumberjack and a hut for him to stay in.]], plr.color) ..
         p(_[[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’.]]) ..
         li(_[[Left-click the ‘OK’ button to close this window so that I can show you how.]])
      )
   }

   local initial_message_03 = {
      title = _"Building Spaces",
      position = "topright",
      body = (
         h1(_"Let’s dive right in!") ..
         li_object("barbarians_lumberjacks_hut", _[[Now that I have shown you how to show and hide the building spaces, please switch them on again so that we can place our first building.]], plr.color)
      )
   }

   local obj_initial_toggle_building_spaces = {
      name = "initial_toggle_building_spaces",
      title=_"Show building spaces",
      number = 1,
      body = objective_text(_"Show building spaces",
         p(_[[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’. There are two ways you can do this:]]) ..
         li_image("images/wui/menus/showhide.png", _[[Press the Space bar to toggle them, or select ‘Show Building Spaces’ in the ‘Show / Hide’ menu.]]) ..
         li(_[[Show the building spaces now.]])
      )
   }

   pop_textdomain()

   -- Teach building spaces
   campaign_message_box(initial_message_02, 200)

--[[
   select_item_from_dropdown("dropdown_menu_showhide", 1)
   select_item_from_dropdown("dropdown_menu_showhide", 1)
   local o = campaign_message_with_objective(initial_message_03, obj_initial_toggle_building_spaces)

   -- Wait for buildhelp to come on
   while not wl.ui.MapView().buildhelp do
      sleep(200)
   end
   set_objective_done(o, 500)
   wl.ui.MapView():abort_road_building()
   ]]

   wl.Game().players[1]:mark_training_wheel_as_solved("toolbar")
end)
