-- Detect the player tribe's log producer ad tree planter buildings and teach how to build them

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

local function find_log_producer(buildings)
   for b_idx, building in ipairs(buildings) do
      if (building.type_name == "productionsite") then
         if #building.inputs == 0 then
            if #building.collected_immovables > 0 then
               local produces_log = false
               for ware_idx, ware in ipairs(building.output_ware_types) do
                  if ware.name == "log" then
                     produces_log = true
                     break
                  end
               end
               if produces_log then
                  for imm_idx, immovable in ipairs(building.collected_immovables) do
                     if immovable:has_attribute("tree") then
                        return building
                     end
                  end
               end
            end
         end
      end
   end
   return nil
end

local function find_target_field(starting_field, size, range)
   for f_idx, field in ipairs(starting_field:region(range)) do
      if field:has_caps(size) then
         return field
      end
   end
   return nil
end


run(function()
   sleep(100)

   local interactive_player_slot = wl.ui.MapView().interactive_player
   local player = wl.Game().players[interactive_player_slot]
   local tribe = player.tribe

   print("Player tribe is: " .. tribe.name)

   -- Find the tree collector / log producer building
   local buildings = tribe.buildings
   local log_producer = find_log_producer(buildings)
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

   local target_field = find_target_field(starting_field, log_producer.size, 4)
   if target_field == nil then
      target_field = find_target_field(starting_field, log_producer.size, 5)
      if target_field == nil then
         target_field = find_target_field(starting_field, log_producer.size, 6)
      end
   end

   if target_field == nil then
      print("No target field")
      return
   end

   print("Target field is: " .. target_field.x .. " " .. target_field.y)

   push_textdomain("training_wheels")

   local size_description = _"Click on a small, medium or big building space, then select the building from the small buildings tab."
   if log_producer.size == "medium" then
      size_description = _"Click on a medium or big building space, then select the building from the medium buildings tab."
   elseif log_producer.size == "big" then
      size_description = _"Click on a big building space, then select the building from the big buildings tab."
   end

   msg_logs = {
      title = _"Logs",
      position = "topright",
      body = (
         ""
      ),
      h = 380,
      w = 260
   }


   local obj_logs = {
   name = "obj_logs",
      title = _"Logs",
      number = 1,
      body = (
         objective_text(_"Place Building",
         p(_"You need to supply your tribe with logs. Please build the following building:") ..
         li_object(log_producer.name, log_producer.descname, player.color) ..
         li_image("images/wui/overlays/" .. log_producer.size .. ".png", size_description))
      )
   }

   pop_textdomain()

   target_field:indicate(true)

   local o = campaign_message_with_objective(msg_logs, obj_logs, 0)

   local starting_conquer_range = wl.Game():get_building_description(starting_immovable.descr.name).conquers
   local search_area = starting_field:region(starting_conquer_range)
   local lumberjack_field = nil
   repeat
      sleep(1000)
      for f_idx, field in ipairs(search_area) do
         if field.immovable ~= nil then
            if field.immovable.descr.name == "constructionsite" and field.immovable.building == log_producer.name then
               print("Found constructionsite")
               lumberjack_field = field
            end
         end
      end
   until lumberjack_field ~= nil

   set_objective_done(o)
   target_field:indicate(false)

   local mapview = wl.ui.MapView()

   if not mapview.is_building_road then
      print("NOCOM Indicate first flag")
   end

   target_field = starting_immovable.flag.fields[1]
   target_field:indicate(true)

   -- NOCOM road building objective

   -- NOCOM skip this if player already has one
   while #player:get_buildings(log_producer.name) < 1 do sleep(300) end


   target_field:indicate(false)

   -- wl.Game().players[1]:mark_training_wheel_as_solved("trees")
end)
