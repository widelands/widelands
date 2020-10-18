-- Detect the player tribe's log producer ad tree planter buildings and teach how to build them

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/buildings.lua"
include "scripting/training_wheels/utils/ui.lua"

run(function()
   sleep(100)

   local interactive_player_slot = wl.ui.MapView().interactive_player
   local player = wl.Game().players[interactive_player_slot]
   local tribe = player.tribe

   print("Player tribe is: " .. tribe.name)

   -- Find the tree collector / log producer building
   local buildings = tribe.buildings
   local log_producer = find_immovable_collector_for_ware(buildings, "tree", "log")
   if log_producer == nil then
      print("Log producer not found")
      return
   end
   print("Log producer is: " .. log_producer.name)

   -- Find a suitable buildable field close to the the starting field
   local starting_field = wl.Game().map.player_slots[interactive_player_slot].starting_field
   local starting_immovable = starting_field.immovable
   if starting_immovable == nil then
      print("No starting field immovable - maybe we have a nomadic starting condition")
      return
   end

   local target_field = find_buildable_field(starting_field, log_producer.size, 4, 6)

   if target_field == nil then
      print("No target field")
      return
   end

   print("Target field is: " .. target_field.x .. " " .. target_field.y)

   local mapview = wl.ui.MapView()
   local auto_roadbuilding = mapview.auto_roadbuilding_mode

   push_textdomain("training_wheels")

   local size_description = _"Click on a small, medium or big building space, then select the building from the small buildings tab."
   if log_producer.size == "medium" then
      size_description = _"Click on a medium or big building space, then select the building from the medium buildings tab."
   elseif log_producer.size == "big" then
      size_description = _"Click on a big building space, then select the building from the big buildings tab."
   end

   local msg_logs = {
      title = _"Logs",
      position = "topright",
      body = (
         p(_"You need to supply your tribe with logs. Please build the following building:") ..
         li_object(log_producer.name, log_producer.descname, player.color) ..
         li_image("images/wui/overlays/" .. log_producer.size .. ".png", size_description)
      ),
      h = 280,
      w = 260,
      modal = false
   }

   local msg_enter_roadbuilding = {
      title = _"Roads",
      position = "topright",
      body = (
         li_object(log_producer.name, "Click on the flag in front of the building to start placing a road", player.color)
      ),
      h = 280,
      w = 260,
      modal = false
   }

   local msg_click_roadbutton = {
      title = _"Roads",
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_way.png", "Click on the ‘Build road’ button, then hold down the ‘Ctrl’ key and click on the indicated flag.")
      ),
      h = 280,
      w = 260,
      modal = false
   }

   local msg_click_road_endflag = {
      title = _"Roads",
      position = "topright",
      body = (
         li_object(log_producer.name, _"Click on the building’s button…") ..
         -- NOCOM We need the tribe's flag image
         li_image("images/wui/fieldaction/menu_build_flag.png", "…then hold down the ‘Ctrl’ key and click on the indicated flag.")
      ),
      h = 280,
      w = 260,
      modal = false
   }

   pop_textdomain()

   target_field:indicate(true)

   campaign_message_box(msg_logs)

   local starting_conquer_range = wl.Game():get_building_description(starting_immovable.descr.name).conquers

   -- NOCOM harden this to player not following instructions
   while not mapview.windows.field_action or not mapview.windows.field_action.tabs["small"] do
      sleep(100)
   end
   target_field:indicate(false)
   mapview.windows.field_action.tabs["small"]:indicate(true)
   while not mapview.windows.field_action.tabs["small"].active do sleep(100) end

   if auto_roadbuilding then
      -- Do this before the road building blocks us
      close_story_messagebox()
      target_field:indicate(true)
      campaign_message_box(msg_click_road_endflag)
      while mapview.windows.field_action do sleep(100) end
      mapview:indicate(false)
   end

   local target_field = wait_for_constructionsite_field(log_producer.name, starting_field, starting_conquer_range)

   if not auto_roadbuilding then
      mapview:indicate(false)
      close_story_messagebox()
      -- If not in roadbuilding mode, get the player to click the constructionsite's flag
      if not mapview.is_building_road then
         print("Extra objective XXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
         target_field:indicate(true)
         campaign_message_box(msg_enter_roadbuilding)

         -- Wait for player to click a flag
         while not mapview.windows.field_action or not mapview.windows.field_action.buttons["build_road"] do
            while not mapview.windows.field_action do sleep(100) end
            if not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"] then
               print("OOps, we want a flag clicked!") -- NOCOM
               while mapview.windows.field_action and (not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"]) do
                  sleep(100)
               end
            end
         end
         close_story_messagebox()
         target_field:indicate(false)
         -- NOCOM indicating does not work - maybe a layer problem local build_road_button = mapview.windows.field_action.buttons["build_road"]
         --build_road_button:indicate(true)
         print("We have a road button!")
         campaign_message_box(msg_click_roadbutton)
         while not wl.ui.MapView().is_building_road do sleep(100) end
         print("We're in roadbuilding mode!")
         --build_road_button:indicate(false)
      end
   end

   target_field = starting_immovable.flag.fields[1]
   target_field:indicate(true)

   while wl.ui.MapView().is_building_road do sleep(100) end
   close_story_messagebox()
   target_field:indicate(false)

   -- NOCOM skip this if player already has one
   while #player:get_buildings(log_producer.name) < 1 do sleep(300) end


   -- wl.Game().players[1]:mark_training_wheel_as_solved("trees")
end)
