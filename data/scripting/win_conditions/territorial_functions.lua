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

function get_teamnumbers(plrs)
   local teamnumbers = {} -- array with team numbers
   for idx,p in ipairs(plrs) do
      local team = p.team
      if team > 0 then
         local found = false
         for idy,t in ipairs(teamnumbers) do
            if t == team then
               found = true
               break
            end
         end
         if not found then
            table.insert(teamnumbers, team)
         end
      end
   end
   return teamnumbers
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
         owned_fields[n] = owned_fields[n] + 1
      end
   end
   return owned_fields
end
