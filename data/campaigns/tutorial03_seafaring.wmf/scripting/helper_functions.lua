-- =================
-- Helper functions
-- =================

-- A copy of prefilled_buildings from scripting/infrastructure.lua, but with
-- as much wares as possible, if not indicated otherwise
function filled_buildings(p, ...)
   for idx,bdescr in ipairs({...}) do
      b = p:place_building(bdescr[1], wl.Game().map:get_field(bdescr[2],bdescr[3]), false, true)
      -- Fill with workers
      if b.valid_workers then b:set_workers(b.valid_workers) end
      if bdescr.workers then b:set_workers(bdescr.workers) end
      -- Fill with soldiers
      if b.max_soldiers and b.set_soldiers then
         if bdescr.soldiers then
            b:set_soldiers(bdescr.soldiers)
         else
            b:set_soldiers({0,0,0,0}, b.max_soldiers)
         end
      elseif bdescr.soldiers then -- Must be a warehouse
         b:set_soldiers(bdescr.soldiers)
      end
      -- Fill with wares
      if bdescr.wares then b:set_wares(bdescr.wares)
      elseif b.valid_wares then b:set_wares(b.valid_wares) end
      if bdescr.inputs then b:set_inputs(bdescr.inputs)
      elseif b.valid_inputs then b:set_inputs(b.valid_inputs) end
   end
end

-- Check if construction of specified building has started on given field
-- First bob on the field indicates start of construction works
function construction_started(field, building_desc)
   return field.immovable and
      field.immovable.descr.type_name == "constructionsite" and
      field.immovable.building == building_desc and
      #field.bobs > 0
end

-- Check if construction of specified building has started in given region
-- construction_started() function is called for each field in region
function construction_started_region(region, building_desc)
   for k,v in pairs(region) do
      if construction_started(v, building_desc) then
         return true
      end
   end
   return false
end

-- Returns array of all swimmable fields in the map
function get_swimmable_fields()
   local fields = {}
   for x = 0, (map.width - 1) do
      for y = 0, (map.height - 1) do
         local f = map:get_field(x, y)
         -- don't use f:has_caps("swimmable"), ferry can sit on a beach
         if f.terr == "summer_water" or f.terd == "summer_water" then
            table.insert(fields, f)
         end
      end
   end
   return fields
end

-- Returns array of fields in area where bob is "atlanteans_ferry"
function get_fields_with_ferry(area)
   local fields = {}
   for i,field in pairs(area) do
      for j,bob in pairs(field.bobs) do
         if bob.descr.name == "atlanteans_ferry" then
            table.insert(fields, field)
         end
      end
   end
   return fields
end

-- Returns array of fields in area of "waterway" type
function get_waterway_fields(area)
   local fields = {}
   for i,field in pairs(area) do
      if field.immovable and field.immovable.descr.type_name == "waterway" then
         table.insert(fields, field)
      end
   end
   return fields
end

-- Scroll to first building of `which` type
-- Return prior center map pixel of the MapView,
--  otherwise nil if no building exists
function scroll_to_first_building(which)
   local buildings = plr:get_buildings(which)
   if #buildings > 0 then
      local first = buildings[1]
      return wait_for_roadbuilding_and_scroll(first.fields[1])
   end
   return nil
end
