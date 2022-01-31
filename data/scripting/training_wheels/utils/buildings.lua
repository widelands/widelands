-- TODO(GunChleoc): Document more fully
include "scripting/messages.lua"
include "scripting/training_wheels/utils/ui.lua"

function has_productive_building_type(player, buildingname)
   for b_idx, building in ipairs(player:get_buildings(buildingname)) do
      if building.productivity > 10 then
         return true
      end
   end
   return false
end

function select_warehouse_types(buildings)
   local result = {}
   for b_idx, building in ipairs(buildings) do
      if (building.type_name == "warehouse") then
         table.insert(result, building.name)
      end
   end
   return result
end

-- Wait for a warehouse
function wait_for_warehouse(player, buildings)
   local warehouse_types = select_warehouse_types(buildings)
   repeat
      for b_idx, warehouse in ipairs(warehouse_types) do
         local candidates = player:get_buildings(warehouse)
         if #candidates > 0 then
            warehouse_immovable = candidates[1]
            break
         end
      end
      if warehouse_immovable == nil then
         sleep(300)
      end
   until warehouse_immovable ~= nil
   return warehouse_immovable
end

-- Find tribe-dependent building type e.g. "trees" and "log" will give us a lumberjack
function find_immovable_collector_for_ware(buildings, immovable_attribute, warename)
   for b_idx, building in ipairs(buildings) do
      if (building.type_name == "productionsite") then
         if #building.inputs == 0 then
            if #building.collected_immovables > 0 then
               local produces_log = false
               for ware_idx, ware in ipairs(building.output_ware_types) do
                  if ware.name == warename then
                     produces_log = true
                     break
                  end
               end
               if produces_log then
                  for imm_idx, immovable in ipairs(building.collected_immovables) do
                     if immovable:has_attribute(immovable_attribute) then
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

-- Search a radius for a buildplot with size "small" or "medium" etc.
-- Tries to find it within min_radius first, then expands the radius by 1 until max_radius is reached
-- Returns nil if nothing was found, a field otherwise.
function find_buildable_field(center_field, player, size, min_radius, max_radius)
   if max_radius < min_radius then
      print("ERROR: max_radius < min_radius in find_buildable_field")
      return nil
   end

   local function find_buildable_field_helper(center_field, player, size, range)
      for f_idx, field in ipairs(center_field:region(range)) do
         if player == field.owner and field:has_caps(size) then
            local sufficient_space = true
            -- Search around the field's flag position for the space for roads
            for nf_idx, nearby_field in ipairs(field.brn:region(1)) do
               if player ~= nearby_field.owner or not nearby_field:has_caps("flag") then
                  sufficient_space = false
               end
            end
            if sufficient_space then
               -- Ensure the border won't interfere
               for nf_idx, nearby_field in ipairs(field:region(2)) do
                  if player ~= nearby_field.owner then
                     sufficient_space = false
                  end
               end
               if sufficient_space then
                  return field
               end
            end
         end
      end
      return nil
   end

   local target_field = nil
   local radius = min_radius
   repeat
      target_field = find_buildable_field_helper(center_field, player, size, radius)
      if target_field ~= nil then
         return target_field
      end
      radius = radius + 1
   until radius == max_radius
   return target_field
end

function find_immovable_fields(center_field, immovable_attribute, inner_radius, outer_radius)
   local found_fields = {}
   if outer_radius <= inner_radius then
      print("ERROR: outer_radius <= inner_radius in find_immovable_field")
      return found_fields
   end

   local function find_immovable_fields_helper(center_field, immovable_attribute, inner_radius, outer_radius)
      -- Hollow region takes first the outer, then the inner radius
      local fields_this_round = {}
      if center_field ~= nil then
         for f_idx, field in ipairs(center_field:region(outer_radius, inner_radius)) do
            if field.immovable ~= nil and field.immovable:has_attribute(immovable_attribute) then
               table.insert(fields_this_round, field)
            end
         end
      end
      return fields_this_round
   end

   local radius = inner_radius + 1
   repeat
      found_fields = find_immovable_fields_helper(center_field, immovable_attribute, inner_radius, radius)
      if #found_fields > 0 then
         return found_fields
      end
      radius = radius + 1
   until radius == outer_radius
   return found_fields
end

-- We can't list constructionsites directly, so we search a region for it
function find_constructionsite_field(buildingname, search_area)
   for f_idx, field in ipairs(search_area) do
      if field.immovable ~= nil and
         field.immovable.descr.name == "constructionsite" and
         field.immovable.building == buildingname then
         return field
      end
   end
   return nil
end

function wait_for_constructionsite_field(buildingname, search_area, reminder_message, seconds)
   local target_field = nil
   local counter = 0
   seconds = seconds * 10
   repeat
      counter = counter + 1
      sleep(100)
      target_field = find_constructionsite_field(buildingname, search_area)
   until target_field ~= nil or counter % seconds == 0
   if target_field == nil then
      -- Player did not build the correct building. Remind player to place the building.
      campaign_message_box(reminder_message)
      target_field = wait_for_constructionsite_field(buildingname, search_area, reminder_message, seconds)
      close_story_messagebox()
   end
   return target_field
end

function find_needed_flag_on_road(center_field, player, radius)
   for f_idx, field in ipairs(center_field:region(radius)) do
      if player == field.owner and field.buildable and field.has_roads == true then
         return field
      end
   end
   return nil
end


-- Wait for a builder to arrive at the target field, or for the building type
-- to be present in case the player rips the building and places it somwehere else
-- Returns false if the construction site was ripped
function wait_for_builder_or_building(player, target_field, buildingname, constructionsite_search_area, seconds)
   if target_field == nil then return false end

   push_textdomain("training_wheels")

   local msg_road_not_connected = {
      title = _("Roads"),
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_way.png", _("Click on the flag in front of the building, then on the ‘Build road’ button, then on another flag."))
      ),
      h = 140,
      w = 260,
      modal = false
   }

   pop_textdomain()

   local mapview = wl.ui.MapView()
   local buildername = player.tribe.builder
   local counter = 0
   local constructionsite_retries = 0
   while true do
      counter = counter + 1
      if counter % seconds == 0 then
         -- Builder has not arrived, explain road building again and wait for it
         target_field.brn:indicate(true)
         close_story_messagebox()
         campaign_message_box(msg_road_not_connected)
         scroll_to_field(target_field)
         while not mapview.is_building_road do sleep(100) end
         while mapview.is_building_road do sleep(100) end
         mapview:indicate(false)
      end
      sleep(1000)
      -- Check that we still have a constructionsite
      target_field = find_constructionsite_field(buildingname, constructionsite_search_area)
      if target_field == nil then
         constructionsite_retries = constructionsite_retries + 1
         if constructionsite_retries == 10 then
            wl.Game().map:get_field(0, 0):indicate(false)
            close_story_messagebox()
            return false
         end
      else
         -- A finished building somewhere else is also a success
         if #player:get_buildings(buildingname) > 0 then
            target_field:indicate(false)
            close_story_messagebox()
            return true
         end
         -- Now check for the builder
         for b_idx, bob in ipairs(target_field.bobs) do
            if bob.descr.name == buildername then
               target_field:indicate(false)
               close_story_messagebox()
               return true
            end
         end
      end
   end
end

function finish_training_wheel_for_placing_building(constructionsite_field, buildingname, msg_finished, player, training_wheel_name)
   sleep(4000)
   if constructionsite_field.immovable ~= nil and
   constructionsite_field.immovable.descr.name == "constructionsite" and
   constructionsite_field.immovable.building == buildingname then
      -- Congratulate the player
      scroll_to_field(constructionsite_field)
      campaign_message_box(msg_finished)
      sleep(4000)

      -- Teaching is done, so mark it as solved
      player:mark_training_wheel_as_solved(training_wheel_name)
   else
      -- Player was too uncooperative, we'll have to try again some time with a new game
      player:release_training_wheel_lock()
   end
end
