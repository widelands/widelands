-- TODO(GunChleoc): Document more fully

function select_warehouse_types(buildings)
   local result = {}
   for b_idx, building in ipairs(buildings) do
      if (building.type_name == "warehouse") then
         table.insert(result, building.name)
      end
   end
   return result
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

   local function find_buildable_field_helper(starting_field, player, size, range)
      for f_idx, field in ipairs(starting_field:region(range)) do
         if player == field.owner and field:has_caps(size) then
            local space_for_flags = true
            for nf_idx, nearby_field in ipairs(field:region(1)) do
               if not (player == nearby_field.owner and nearby_field:has_caps("flag")) then
                  space_for_flags = false
               end
            end
            if space_for_flags then
               return field
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

function find_immovable_field(center_field, immovable_attribute, inner_radius, outer_radius)
   if outer_radius <= inner_radius then
      print("ERROR: outer_radius <= inner_radius in find_immovable_field")
      return nil
   end

   local function find_immovable_field_helper(center_field, immovable_attribute, inner_radius, outer_radius)
      -- Hollow region takes first the outer, then the inner radius
      for f_idx, field in ipairs(center_field:region(outer_radius, inner_radius)) do
         if field.immovable ~= nil and field.immovable:has_attribute(immovable_attribute) then
            return field
         end
      end
   end

   local target_field = nil
   local radius = inner_radius + 1
   repeat
      target_field = find_immovable_field_helper(center_field, immovable_attribute, inner_radius, radius)
      if target_field ~= nil then
         return target_field
      end
      radius = radius + 1
   until radius == outer_radius
   return target_field
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

function wait_for_constructionsite_field(buildingname, search_area)
   local target_field = nil
   repeat
      sleep(100)
      target_field = find_constructionsite_field(buildingname, search_area)
   until target_field ~= nil
   return target_field
end

function find_needed_flag_on_road(starting_field, player, radius)
   for f_idx, field in ipairs(starting_field:region(radius)) do
      if player == field.owner and field.buildable and field.has_roads == true then
         return field
      end
   end
   return nil
end
