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
