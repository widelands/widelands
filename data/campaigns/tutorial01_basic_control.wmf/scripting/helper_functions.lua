-- =================
-- Helper functions
-- =================

-- Remove all rocks in a given environment. This is done
-- in a loop for a nice optical effect
function remove_all_rocks(fields, g_sleeptime)
   local sleeptime = g_sleeptime or 150
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         local n = f.immovable.descr.name:match("greenland_rocks(%d*)")
         if n then
            n = tonumber(n)
            f.immovable:remove()
            if n > 1 then
               remove_field = false
               map:place_immovable("greenland_rocks" .. n-1, f)
            end
            sleep(sleeptime)
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end
end

-- We have to update the serial for every check because it can change on save and reload
function field_building_window(field)
   if field.immovable == nil then return nil end
   local window_name = string.bformat("building_window_%u", field.immovable.serial)
   return wl.ui.MapView().windows[window_name]
end

function lumberjack_window()
   return field_building_window(first_lumberjack_field)
end

function hq_window()
   return field_building_window(sf)
end
