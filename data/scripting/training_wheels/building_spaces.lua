include "scripting/coroutine.lua"
-- include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
-- include "scripting/table.lua"
include "scripting/ui.lua"


run(function()

   push_textdomain("training_wheels")

   local message = {
      title = "Building Spaces - TODO",
      position = "topright",
      body = (
         h1("Building Spaces - TODO") ..
         li_object("barbarians_headquarters", "Building Spaces - TODO")
      )
   }

   local obj_initial_toggle_building_spaces = {
      name = "initial_toggle_building_spaces",
      title="Show building spaces",
      number = 1,
      body = objective_text(_"Show building spaces",
         p([[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’. There are two ways you can do this:]]) ..
         li_image("images/wui/menus/showhide.png", _[[Press the Space bar to toggle them, or select ‘Show Building Spaces’ in the ‘Show / Hide’ menu.]]) ..
         li([[Show the building spaces now.]])
      )
   }

   pop_textdomain()

   -- Teach building spaces
   campaign_message_box(message, 200)

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

   wl.Game().players[1]:mark_training_wheel_as_solved("building_spaces")
end)
