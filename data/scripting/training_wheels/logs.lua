-- Detect the player tribe's log producer and teach how to build it, along with Ctrl-click for the roads

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/buildings.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

run(function()
   -- Give the buildhelp training wheel a chance to execute first
   sleep(2000)

   local mapview = wl.ui.MapView()
   local player = get_interactive_player()
   local tribe = player.tribe

   -- Find the tree collector / log producer building
   local buildings = tribe.buildings
   local log_producer = find_immovable_collector_for_ware(buildings, "tree", "log")
   if log_producer == nil then
      print("Log producer not found for tribe '" .. tribe.name .. "'. Aborting training wheel 'logs'.")
      return
   end

   -- If the player already built it, teach this some other time
   if has_productive_building_type(player, log_producer.name) then
      player:skip_training_wheel(training_wheel_name)
      return
   end

   -- Find a suitable buildable field close to the the starting field
   local conquering_field = wl.Game().map.player_slots[wl.Game().interactive_player].starting_field
   local conquering_immovable = conquering_field.immovable

   -- Wait for a warehouse
   local warehouse_immovable = wait_for_warehouse(player, buildings)

   if conquering_immovable == nil then
      conquering_immovable = warehouse_immovable
      conquering_field = warehouse_immovable.fields[1]
   end

   -- Check whether we already have a constructionsite from savegame
   local starting_conquer_range = wl.Game():get_building_description(conquering_immovable.descr.name).conquers
   local constructionsite_search_area = conquering_field:region(starting_conquer_range)
   local constructionsite_field = find_constructionsite_field(log_producer.name, constructionsite_search_area)
   local target_field = nil
   local teach_placing_constructionsite = false

   -- If there is no constructionsite from savegame, we will want to teach player how to place the building
   if constructionsite_field == nil then
      teach_placing_constructionsite = true

      -- Wait until we find a suitable field near trees, then acquire lock.
      -- The check again if there is still a suitable field and if not, release the lock and try again.
      local function wait_for_starting_conditions(conquering_field, player, starting_conquer_range)
         local result = nil

         -- Find a suitable field close to a tree
         local function find_tree_field(conquering_field, player, starting_conquer_range)
            local tree_fields = find_immovable_fields(conquering_field, "tree", math.ceil(starting_conquer_range / 2), starting_conquer_range + log_producer.workarea_radius / 2)
            if #tree_fields > 0 then
               for f_idx, tree_field in ipairs(tree_fields) do
                  local found_tree_field = find_buildable_field(tree_field, player, log_producer.size, 1, log_producer.workarea_radius / 2)
                  if found_tree_field ~= nil then
                     return found_tree_field
                  end
               end
            end
            return nil
         end

         repeat
            result = find_tree_field(conquering_field, player, starting_conquer_range)
            if result == nil then
               sleep(1000)
            end
         until result ~= nil

         -- All set - now wait for lock
         wait_for_lock(player, training_wheel_name)

         -- Check that we still have an appropriate field
         result = find_tree_field(conquering_field, player, starting_conquer_range)
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
   if has_productive_building_type(player, log_producer.name) then
      player:skip_training_wheel(training_wheel_name)
      return
   end

   -- Define our messages
   push_textdomain("training_wheels")

   local size_description = _("Click on a small, medium or big building space, then select the building from the small buildings tab.")
   if log_producer.size == "medium" then
      size_description = _("Click on a medium or big building space, then select the building from the medium buildings tab.")
   elseif log_producer.size == "big" then
      size_description = _("Click on a big building space, then select the building from the big buildings tab.")
   end

   local explain_control_key = join_sentences(_("If you hold down the ‘Ctrl’ key while clicking on the second flag, this will also place more flags on your road if possible."), _("Your carriers can transport your wares faster if they share the load."))

   local msg_logs = {
      title = _("Logs"),
      position = "topright",
      body = (
         p(_("You need to supply your tribe with logs. Please build the following building:")) ..
         li_object(log_producer.name, log_producer.descname, player.color) ..
         li_image("images/wui/overlays/" .. log_producer.size .. ".png", size_description)
      ),
      h = 280,
      w = 260,
      modal = false
   }

   local msg_enter_roadbuilding = {
      title = _("Roads"),
      position = "topright",
      body = (
         li_object(log_producer.name, _("Click on the flag in front of the building to start placing a road."), player.color)
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_roadbutton = {
      title = _("Roads"),
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_way.png", _("Click on the ‘Build road’ button, then and click on the indicated flag.")) ..
         li_arrow(explain_control_key)
      ),
      h = 240,
      w = 260,
      modal = false
   }

   local msg_click_road_endflag = {
      title = _("Roads"),
      position = "topright",
      body = (
         li_object(log_producer.name, _("Click on the building’s button…"), player.color) ..
         -- We can't get the tribe's flag image, so we settle for the main building
         li_object(warehouse_immovable.descr.name, _("…then click on the flag in front of the target building."), player.color) ..
         li_arrow(explain_control_key)
      ),
      h = 380,
      w = 260,
      modal = false
   }

   local msg_finished = {
      title = _("Logs"),
      position = "topright",
      body = (
         li_object(log_producer.name, _("Well done! We will soon start producing logs, which we will need for building more buildings."), player.color)
      ),
      h = 140,
      w = 260,
      scroll_back = true
   }

   pop_textdomain()

   target_field = constructionsite_field

   -- If there is no constructionsite from savegame, we teach player how to place the building
   if teach_placing_constructionsite then
      target_field:indicate(true)
      campaign_message_box(msg_logs)
      scroll_to_field(target_field)

      -- Wait for player to activate the small building tab
      wait_for_field_action_tab(log_producer.size)
      mapview.windows.field_action.tabs[log_producer.size]:indicate(true)
      while not mapview.windows.field_action.tabs[log_producer.size].active do
         sleep(100)
         if not mapview.windows.field_action then
            mapview:indicate(false)
         end
         wait_for_field_action_tab(log_producer.size)
         mapview.windows.field_action.tabs[log_producer.size]:indicate(true)
      end

      -- Explain road building before the road building mode blocks us
      if mapview.auto_roadbuilding_mode then
         close_story_messagebox()
         target_field:indicate(true)
         campaign_message_box(msg_click_road_endflag)
         while mapview.windows.field_action do sleep(100) end
         mapview:indicate(false)
         scroll_to_field(target_field)
      end

      -- Now wait for the constructionsite
      constructionsite_field = wait_for_constructionsite_field(log_producer.name, constructionsite_search_area, msg_logs, 120)
      target_field:indicate(false)
   end

   -- When not auto roadbuilding, we need to click on the constructionsite's flag too
   if not mapview.is_building_road and not mapview.auto_roadbuilding_mode then
      mapview:indicate(false)
      close_story_messagebox()

      target_field = constructionsite_field.immovable.flag.fields[1]
      target_field:indicate(true)
      campaign_message_box(msg_enter_roadbuilding)

      -- Wait for player to click a flag
      while not mapview.windows.field_action or not mapview.windows.field_action.buttons["build_road"] do
         while not mapview.windows.field_action or not mapview.windows.field_action.tabs["roads"] do sleep(100) end
         if not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"] then
            while mapview.windows.field_action and (not mapview.windows.field_action.tabs["roads"].active or mapview.windows.field_action.buttons["build_flag"]) do
               sleep(100)
            end
         end
      end
      close_story_messagebox()

      -- Explain road building button
      target_field:indicate(false)
      local build_road_button = mapview.windows.field_action.buttons["build_road"]
      build_road_button:indicate(true)
      campaign_message_box(msg_click_roadbutton)
      while not mapview.is_building_road do sleep(100) end
      mapview:indicate(false)
   end

   -- Indicate target flag for road building and wait for the road
   target_field = warehouse_immovable.flag.fields[1]
   target_field:indicate(true)
   scroll_to_field(target_field)

   while mapview.is_building_road do sleep(100) end
   close_story_messagebox()
   target_field:indicate(false)

   -- Wait for the builder to arrive
   local success = wait_for_builder_or_building(player, constructionsite_field, log_producer.name, constructionsite_search_area, 60)

   -- We might still have some indicators and messages boxes left over from unexpected player actions
   clean_up_message_boxes_and_indicators()

   if success then
      finish_training_wheel_for_placing_building(constructionsite_field, log_producer.name, msg_finished, player, training_wheel_name)
   else
      -- Player was too uncooperative, we'll have to try again some time with a new game
      player:release_training_wheel_lock()
   end
end)
