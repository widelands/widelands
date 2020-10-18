-- NOCOM document

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


function find_buildable_field(center_field, size, min_radius, max_radius)
   if max_radius < min_radius then
      print("ERROR: max_radius < min_radius in find_buildable_field")
      return nil
   end

   local function find_buildable_field_helper(starting_field, size, range)
      for f_idx, field in ipairs(starting_field:region(range)) do
         if field:has_caps(size) then
            return field
         end
      end
      return nil
   end

   local target_field = nil
   local radius = min_radius
   repeat
      target_field = find_buildable_field_helper(center_field, size, radius)
      if target_field ~= nil then
         return target_field
      end
      radius = radius + 1
   until radius == max_radius
   return target_field
end


function wait_for_constructionsite_field(buildingname, center_field, radius)
   local search_area = center_field:region(radius)
   local target_field = nil
   repeat
      sleep(100)
      for f_idx, field in ipairs(search_area) do
         if field.immovable ~= nil then
            if field.immovable.descr.name == "constructionsite" and field.immovable.building == buildingname then
               target_field = field.immovable.flag.fields[1]
            end
         end
      end
   until target_field ~= nil
   return target_field
end
