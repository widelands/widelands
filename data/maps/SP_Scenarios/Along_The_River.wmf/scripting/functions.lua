-- =======================================================================
--                             Along The River
-- =======================================================================

-- Checks objective for done flag.
function obj_done(objective)
   return (objective ~= nil and objective.done)
end

-- =======================================================================

-- Places building at field if player owns field and extra field for flag.
function place_building_if_owner(player, building, point, cs)
   if point.owner == player and point.brn.owner == player then
      player:place_building(building, point, cs, true)
      return true
   end
   return false
end

-- =======================================================================

function remove_type(point, _type)
   local immovable = point.immovable
   if immovable then
      if immovable.descr.type_name == _type then
         immovable:remove()
      end
   end
end

-- =======================================================================

function immovable_has_name(immovable, name)
   return (immovable ~= nil and immovable.descr.name == name) 
end

-- =======================================================================

function add_wares(point, ware, amount)
   local immovable = point.immovable
   if immovable then
      if immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite" then
         immovable:set_wares(ware, immovable:get_wares(ware) + amount)
      end
   end
end

-- =======================================================================

-- Transports wares to flag, warehouse or productionsite.
-- Waits if the building does not exist.
function transport_wares(player, point, ware, amount, alert, image)
   local count = amount
   while count > 0 do
      sleep(499)
      local immovable = point.immovable
      if immovable then
         if immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite" then
            immovable:set_wares(ware, immovable:get_wares(ware) + 1)
            count = count - 1
         end
      end
   end
   if(alert) then
      local ware_name =  wl.Game():get_ware_description(ware).descname
      local immovable = point.immovable
      if immovable then
         if immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite" then
            if image then
               send_to_inbox(player, _"Transport", li_image(image, amount.._" units of "..ware_name.." have been brought to your "..immovable.descr.descname), { field = point, popup = true, })
            else
               send_to_inbox(player, _"Transport", p(amount.._" units of "..ware_name.." have been brought to your "..immovable.descr.descname), { field = point, popup = true, })
            end
         end
      end
   end
end

-- =======================================================================

-- Spawns soldiers in a warehouse, militarysite or trainingsite with intervals in ms.
-- e.g.: spawn_soldiers(map:get_field(hq_x,hq_y), {3,5,0,2}, 45, 4999)
-- Stops if the object does not exist or is invalid.
function spawn_soldiers(point, ranking, count, interval)
   while count > 0 do
      sleep(interval)
      local immovable = point.immovable
      if immovable then
         if immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "militarysite" or immovable.descr.type_name == "trainingsite" then
            immovable:set_soldiers(ranking, immovable:get_soldiers(ranking) + 1)
         else
            break
         end
      else
         break
      end
      count = count - 1
   end
end

-- =======================================================================

-- Checks if enough wares are available at flag, warehouse or productionsite and removes if true.
-- Returns true if wares are removed.
function wares_collected_at_field(point, ware, amount)
   local immovable = point.immovable
   if immovable then
      if immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite" then
         if immovable:get_wares(ware) >= amount then
            immovable:set_wares(ware, immovable:get_wares(ware) - amount)
            return true
         end
      end
   end
   return false
end

-- =======================================================================
