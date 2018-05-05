-- NOCOM for Territorial Lord and Time. Document.

function get_buildable_fields()
   local fields = {}
   local map = wl.Game().map
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local f = map:get_field(x,y)
         if f.is_buildable then
            table.insert(fields, f)
         end
      end
   end
   print("Found " .. #fields .. " buildable fields")
   return fields
end

function count_owned_fields_for_all_players(fields, plrs)
   local owned_fields = {}
   -- init the landsizes for each player
   for idx,plr in ipairs(plrs) do
      owned_fields[plr.number] = 0
   end

   for idx,f in ipairs(fields) do
      -- check if field is owned by a player
      local o = f.owner
      if o then
         local n = o.number
         if owned_fields[n] == nil then
            -- In case player was defeated and lost all their warehouses, make sure they don't count
            owned_fields[n] = -1
         elseif owned_fields[n] >= 0 then
            owned_fields[n] = owned_fields[n] + 1
         end
      end
   end
   return owned_fields
end
