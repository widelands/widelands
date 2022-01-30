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
   if point.immovable and point.immovable.descr.type_name == _type then
      point.immovable:remove()
   end
end

-- =======================================================================

function immovable_has_name(immovable, name)
   return (immovable ~= nil and immovable.descr.name == name)
end

-- =======================================================================

function add_wares(immovable, ware, amount)
   if immovable and (immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite") then
      immovable:set_wares(ware, immovable:get_wares(ware) + amount)
   end
end

-- =======================================================================

-- Transports wares to flag, warehouse or productionsite.
function transport_wares(player, point, ware, amount, alert, image)
   local count = amount
   while count > 0 do
      sleep(499)
      add_wares(point.immovable, ware, 1)
      count = count - 1
   end
   if (alert) then
      local ware_name =  wl.Game():get_ware_description(ware).descname
      local immovable = point.immovable
      if immovable then
         if immovable.descr.type_name == "flag" or immovable.descr.type_name == "warehouse" or immovable.descr.type_name == "productionsite" then
            push_textdomain("map_along_the_river.wmf")
            if image then
               send_to_inbox(player, _("Transport"), li_image(image, ngettext("%1% unit of %2% has been brought to your ‘%3%’ building.", "%1% units of %2% have been brought to your ‘%3%’ building.", amount):bformat(amount, ware_name, immovable.descr.descname)), { field = point, popup = true, })
            else
               send_to_inbox(player, _("Transport"), p(ngettext("%1% unit of %2% has been brought to your ‘%3%’ building.", "%1% units of %2% have been brought to your ‘%3%’ building.", amount):bformat(amount, ware_name, immovable.descr.descname)), { field = point, popup = true, })
            end
            pop_textdomain()
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
            local soldiers = immovable:get_soldiers("all")
            local did_set = false
            for key,value in pairs(soldiers) do
               if key[1] == ranking[1] and key[2] == ranking[2] and key[3] == ranking[3] and key[4] == ranking[4] then
                  soldiers[key] = value + 1
                  did_set = true
                  break
               end
            end
            if not did_set then soldiers[ranking] = 1 end
            immovable:set_soldiers(soldiers)
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
