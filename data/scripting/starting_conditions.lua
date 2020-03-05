function find_ocean_fields(how_many)
   local map = wl.Game().map
   local fields = {}
   repeat
      local f = map:get_field(math.random(0, map.width - 1), math.random(0, map.height - 1))
      if f:has_caps("swimmable") then
         local route_found = false
         for i,port in pairs(map.port_spaces) do
            if map:sea_route_exists(f, map:get_field(port.x, port.y)) then
               table.insert(fields, f)
               break
            end
         end
      end
   until #fields == how_many
   return fields
end
