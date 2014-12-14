-- ========================
-- Does the actual hopping
-- ========================
function add_wares(hq, wares)
   for name, count in pairs(wares) do
      hq:set_wares(name, hq:get_wares(name) + count)
   end
end

function add_workers(hq, workers)
   for name, count in pairs(workers) do
      hq:set_workers(name, hq:get_workers(name) + count)
   end
end

function add_soldiers(hq, soldiers)
   local setpoints = {}
   for sdescr, count in pairs(soldiers) do
      setpoints[ {sdescr:match("(%d):(%d):(%d):(%d)")} ] = count
   end
   hq:set_soldiers(setpoints)
end

function _remove_content(old)
   local soldiers, workers, wares = {}, {}, {}

   -- salvage soldiers
   for descr, count in pairs(old:get_soldiers("all")) do
      local sdescr = ("%i:%i:%i:%i"):format(table.unpack(descr))
      if not soldiers[sdescr] then soldiers[sdescr] = 0 end
      soldiers[sdescr] = soldiers[sdescr] + count
   end
   old:set_soldiers({0,0,0,0}, 0)

   -- salvage workers
   for name, count in pairs(old:get_workers("all")) do
      if not workers[name] then workers[name] = 0 end
      workers[name] = workers[name] + count
      old:set_workers(name, 0)
   end

   -- salvage wares
   for name, count in pairs(old:get_wares("all")) do
      if not wares[name] then wares[name] = 0 end
      wares[name] = wares[name] + count
      old:set_wares(name, 0)
   end
   return soldiers, workers, wares
end

function hop_to_next_island(plr, island_idx)
   -- Place the new HQ
   local new_hq_field = _start_fields[island_idx + 1][plr.number]
   local new_hq = plr:place_building("atlanteans_headquarters", new_hq_field, false, true)

   -- Transfer stuff from HQs
   for i=1,island_idx do
      local old_hq = _start_fields[i][plr.number].immovable
      if not old_hq then break end

      local soldiers, workers, wares = _remove_content(old_hq)
      add_soldiers(new_hq, soldiers)
      add_workers(new_hq, workers)
      add_wares(new_hq, wares)
   end

   -- Transfer from existing warehouses
   for idx,wh in ipairs(plr:get_buildings("atlanteans_warehouse")) do
      local soldiers, workers, wares = _remove_content(wh)
      add_soldiers(new_hq, soldiers)
      add_workers(new_hq, workers)
      add_wares(new_hq, wares)
   end

   return new_hq
end
