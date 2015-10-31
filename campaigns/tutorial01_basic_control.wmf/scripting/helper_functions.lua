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
               map:place_immovable("greenland_rocks" .. n-1, f, "world")
            end
            sleep(sleeptime)
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end
end