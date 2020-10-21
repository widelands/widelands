-- Teach placing flags on the road if there is room for them

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/buildings.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

run(function()
   sleep(10)

   local mapview = wl.ui.MapView()
   local interactive_player_slot = wl.Game().interactive_player
   local player = wl.Game().players[interactive_player_slot]

   -- Find a building to search its region
   local starting_field = wl.Game().map.player_slots[interactive_player_slot].starting_field
   local starting_immovable = starting_field.immovable

   -- Wait for a warehouse if we don't have an immovable on the starting field
   if starting_immovable == nil then
      starting_immovable = wait_for_warehouse(player, player.tribe.buildings)
      starting_field = starting_immovable.fields[1]
   end

   local function wait_for_starting_conditions(starting_field, player, starting_conquer_range)
      local result = nil
      repeat
         result = find_needed_flag_on_road(starting_field, player, starting_conquer_range)
         sleep(1000)
      until result ~= nil

      -- All set - now wait for lock
      wait_for_lock(player, training_wheel_name)

      -- Check that we still are in need of a flag after the lock has been acquired
      result = find_needed_flag_on_road(starting_field, player, starting_conquer_range)
      if result == nil then
         player:release_training_wheel_lock()
         wait_for_starting_conditions(starting_field, player, starting_conquer_range)
      end
      return result
   end

   local target_field = wait_for_starting_conditions(
                           starting_field,
                           player,
                           wl.Game():get_building_description(starting_immovable.descr.name).conquers)

   -- Define our messages
   push_textdomain("training_wheels")

   local msg_click_flag_symbol = {
      title = _"Flags",
      position = "topright",
      body = (
         p(_"Your carriers can transport your wares faster if they share the load.") ..
         li_image("images/wui/overlays/set_flag.png", _"Click on the flag symbol.")
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_flag_button = {
      title = _"Flags",
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_flag.png", _"Click on the flag button.")
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_more_flags = {
      title = _"Flags",
      position = "topright",
      body = (
         li_image("images/wui/overlays/set_flag.png", _"Now check if you can place even more flags on your roads.") ..
         p("Click ‘OK’ when you’re done.")
      ),
      h = 120,
      w = 260,
      modal = false
   }


   pop_textdomain()


   -- Force buildhelp so we can show the flag position
   mapview.buildhelp = true
   target_field:indicate(true)
   campaign_message_box(msg_click_flag_symbol)

   wait_for_field_action_tab("roads")
   while not mapview.windows.field_action.tabs["roads"].active do
      sleep(100)
      if not mapview.windows.field_action.tabs["roads"].active then
         mapview.windows.field_action.tabs["roads"]:indicate(true)
      end
      wait_for_field_action_tab("roads")
   end
   target_field:indicate(false)
   mapview.windows.field_action.buttons["build_flag"]:indicate(true)
   close_story_messagebox()
   campaign_message_box(msg_click_flag_button)

   while mapview.windows.field_action do sleep(100) end
   mapview:indicate(false)

   close_story_messagebox()
   campaign_message_box(msg_click_more_flags)

   while mapview.windows.story_message_box ~= nil do sleep(100) end

   player:mark_training_wheel_as_solved(training_wheel_name)
   sleep(2000)
end)
