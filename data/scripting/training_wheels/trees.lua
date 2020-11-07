-- Detect the player tribe's log producer and tree planter and teach about trees

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
   local player = get_interactive_player()
   local tribe = player.tribe

   -- Find the tree collector + log producer buildings
   local buildings = tribe.buildings
   local tree_planter = find_immovable_creator(buildings, "tree")
   if tree_planter == nil then
      print("Tree planter not found for tribe '" .. tribe.name .. "'. Aborting training wheel 'trees'.")
      return
   end
   local log_producer = find_immovable_collector_for_ware(buildings, "tree", "log")
   if log_producer == nil then
      print("Log producer not found for tribe '" .. tribe.name .. "'. Aborting training wheel 'logs'.")
      return
   end

   -- If the player already built it, teach this some other time
   if has_productive_building_type(player, tree_planter.name) then
      player:skip_training_wheel(training_wheel_name)
      return
   end

   -- Find a suitable buildable field close to a log producer
   local conquering_field = wl.Game().map.player_slots[wl.Game().interactive_player].starting_field
   local conquering_immovable = conquering_field.immovable

   -- Wait for a warehouse
   local warehouse_immovable = wait_for_warehouse(player, buildings)

   if conquering_immovable == nil then
      conquering_immovable = warehouse_immovable
      conquering_field = warehouse_immovable.fields[1]
   end

   -- Find a finished log producer
   push_textdomain("training_wheels")

   local size_description = _"Click on a small, medium or big building space, then select the building from the small buildings tab."
   if log_producer.size == "medium" then
      size_description = _"Click on a medium or big building space, then select the building from the medium buildings tab."
   elseif log_producer.size == "big" then
      size_description = _"Click on a big building space, then select the building from the big buildings tab."
   end

   local log_reminder_message = {
      title = _"Logs",
      position = "topright",
      body = (
         p(_"We need to fell trees. Build the following building and connect it to your road network:") ..
         li_object(log_producer.name, log_producer.descname, player.color) ..
         li_image("images/wui/overlays/" .. log_producer.size .. ".png", size_description)
      ),
      h = 280,
      w = 260,
      modal = false
   }

   pop_textdomain()

   -- Wait for log producer
   wait_for_building_field(log_producer.name, player, log_reminder_message, 180)
   local logproducer_field = player:get_buildings(log_producer.name)[1].fields[1]

   -- Give the worker some time to arrive and cut some trees
   sleep(30 * 1000)
   -- Ensure that the log producer building is still there
   repeat
      wait_for_building_field(log_producer.name, player, log_reminder_message, 180)
      logproducer_field = player:get_buildings(log_producer.name)[1].fields[1]
   until logproducer_field ~= nil

   -- Check whether we already have a constructionsite from savegame
   local starting_conquer_range = wl.Game():get_building_description(conquering_immovable.descr.name).conquers
   local constructionsite_search_area = conquering_field:region(starting_conquer_range)
   local constructionsite_field = find_constructionsite_field(tree_planter.name, constructionsite_search_area)
   local target_field = nil
   local teach_placing_constructionsite = false

   -- If there is no constructionsite from savegame, we will want to teach player how to place the building
   if constructionsite_field == nil then
      teach_placing_constructionsite = true

      -- Wait until we find a suitable field near a log producer, then acquire lock.
      -- The check again if there is still a suitable field and if not, release the lock and try again.
      local function wait_for_starting_conditions(conquering_field, player, starting_conquer_range)
         local result = nil

         -- Ensure we still have a log producer
         while #player:get_buildings(log_producer.name) < 1 do sleep(1000) end
         logproducer_field = player:get_buildings(log_producer.name)[1].fields[1]

         -- Find a suitable field close to the log producer
         repeat
            result = find_buildable_field(logproducer_field, player, tree_planter.size, 0, tree_planter.workarea_radius)
            if result == nil then
               sleep(1000)
            end
         until result ~= nil

         -- All set - now wait for lock
         wait_for_lock(player, training_wheel_name)

         -- Check that we still have an appropriate field
         if #player:get_buildings(log_producer.name) < 1 then
            result = nil
         else
            logproducer_field = player:get_buildings(log_producer.name)[1].fields[1]
            result = find_buildable_field(logproducer_field, player, tree_planter.size, 0, tree_planter.workarea_radius)
         end
         if result == nil then
            -- While we were waiting for the lock, appropriate fields became unavailable.
            -- Release the lock and try again.
            player:release_training_wheel_lock()
            wait_for_starting_conditions(starting_field, player, starting_conquer_range)
         end
         return result
      end

      constructionsite_field = wait_for_starting_conditions(conquering_field, player, starting_conquer_range)
   else
      -- All set - now wait for lock
      wait_for_lock(player, training_wheel_name)
      teach_placing_constructionsite = false
   end

   -- The player built it in the meantime. Teach this some other time.
   if has_productive_building_type(player, tree_planter.name) then
      player:skip_training_wheel(training_wheel_name)
      return
   end

   -- Define our messages
   push_textdomain("training_wheels")

   local msg_trees = {
      title = _"Trees",
      position = "topright",
      body = (
         li_object(log_producer.name, _"You are felling trees to supply your tribe with logs.", player.color) ..
         p(_"We will now plant more trees so you won't run out. Build the following building and connect it to your road network:") ..
         li_object(tree_planter.name, tree_planter.descname, player.color) ..
         li_image("images/wui/overlays/" .. tree_planter.size .. ".png", size_description)
      ),
      h = 380,
      w = 260,
      modal = false
   }

   local msg_finished = {
      title = _"Logs",
      position = "topright",
      body = (
         li_object(tree_planter.name, _"Well done! We will soon be able to support our log production with more trees.", player.color)
      ),
      h = 140,
      w = 260,
      scroll_back = true,
   }

   pop_textdomain()

   target_field = constructionsite_field

   -- If there is no constructionsite from savegame, we teach player how to place the building
   if teach_placing_constructionsite then
      target_field:indicate(true)
      campaign_message_box(msg_trees)
      scroll_to_field(target_field)

      -- Wait for player to activate the small building tab
      wait_for_field_action_tab("small")
      mapview.windows.field_action.tabs["small"]:indicate(true)
      while not mapview.windows.field_action.tabs["small"].active do
         sleep(100)
         if not mapview.windows.field_action then
            mapview:indicate(false)
         end
         wait_for_field_action_tab("small")
         mapview.windows.field_action.tabs["small"]:indicate(true)
      end

      -- Now wait for the constructionsite
      constructionsite_field = wait_for_constructionsite_field(tree_planter.name, constructionsite_search_area, msg_trees, 120)

      close_story_messagebox()
      target_field:indicate(false)
      mapview:indicate(false)
   end

   -- Wait for the builder to arrive
   local success = wait_for_builder_or_building(player, constructionsite_field, tree_planter.name, constructionsite_search_area, 60)

   -- We might still have some indicators and messages boxes left over from unexpected player actions
   clean_up_message_boxes_and_indicators()

   if success then
      finish_training_wheel_for_placing_building(constructionsite_field, tree_planter.name, msg_finished, player, training_wheel_name)
   else
      -- Player was too uncooperative, we'll have to try again some time with a new game
      player:release_training_wheel_lock()
   end
end)
