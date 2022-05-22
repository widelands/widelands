function volcano_eruptions()
   local fields_to_erupt = Set:new{}
   local fields_to_setback = Set:new{}
   local central_field = map:get_field(133,47)
   local region_to_erupt = central_field:region(17)
   for a,field in ipairs(region_to_erupt) do
      if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 6 < 2) then
         for i, f in ipairs(map:get_field(field.x, field.y):region(1)) do
            if f.immovable and not f.immovable:has_attribute("rocks") then f.immovable:remove() end
            if #f.bobs > 0 then
               for j, bob in ipairs(f.bobs) do
                  bob:remove()
               end
            end
         end
         fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
         fields_to_setback:add(f_Field:new(field, field.terd, field.terr))
      end
   end
   local prior_center = scroll_to_field(central_field)
   p1:reveal_fields(central_field:region(19))
   while fields_to_erupt.size > 0 do
      local ff = fields_to_erupt:pop_at(1)
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"d")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"r")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      sleep(5)
   end

   local central_field = map:get_field(120,18)
   local region_to_erupt = central_field:region(14)
   for a,field in ipairs(region_to_erupt) do
      if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 6 < 2) then
         for i, f in ipairs(map:get_field(field.x, field.y):region(1)) do
            if f.immovable and not f.immovable:has_attribute("rocks") then f.immovable:remove() end
            if #f.bobs > 0 then
               for j, bob in ipairs(f.bobs) do
                  bob:remove()
               end
            end
         end
         fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
         fields_to_setback:add(f_Field:new(field, field.terd, field.terr))
      end
   end
   sleep(1000)
   scroll_to_field(central_field)
   p1:reveal_fields(central_field:region(16))
   while fields_to_erupt.size > 0 do
      local ff = fields_to_erupt:pop_at(1)
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"d")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"r")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      sleep(5)
   end

   local central_field = map:get_field(163,128)
   local region_to_erupt = central_field:region(11)
   for a,field in ipairs(region_to_erupt) do
      if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 6 < 2) then
         for i, f in ipairs(map:get_field(field.x, field.y):region(1)) do
            if f.immovable and not f.immovable:has_attribute("rocks") then f.immovable:remove() end
            if #f.bobs > 0 then
               for j, bob in ipairs(f.bobs) do
                  bob:remove()
               end
            end
         end
         fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
         fields_to_setback:add(f_Field:new(field, field.terd, field.terr))
      end
   end
   sleep(1000)
   scroll_to_field(central_field)
   p1:reveal_fields(central_field:region(13))
   while fields_to_erupt.size > 0 do
      local ff = fields_to_erupt:pop_at(1)
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"d")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      local tr_to_erupt = Set:new{Triangle:new(ff._f,"r")}
      local tr = tr_to_erupt:pop_at(1)
      tr:set_ter("lava")
      sleep(5)
   end

   sleep(2000)
   scroll_to_map_pixel(prior_center)
   sleep(2*60*1000)
-- set back all eruptions
   while fields_to_setback.size > 0 do
      local ff = fields_to_setback:pop_at(1)
      local tr_to_setback = Set:new{Triangle:new(ff._f,"d")}
      local tr = tr_to_setback:pop_at(1)
      tr:set_ter(ff._td)
      local tr_to_setback = Set:new{Triangle:new(ff._f,"r")}
      local tr = tr_to_setback:pop_at(1)
      tr:set_ter(ff._tr)
   end
end


-- =================
-- A Field Triangle
-- =================
Triangle = {}
function Triangle:new(f, which)
   local rv = {
      _f = f,
      _d = which,
      __hash = ("%i_%i_%s"):format(f.x, f.y, which),
   }

   setmetatable(rv,self)
   self.__index = self

   return rv
end

function Triangle:get_ter()
   if self._d == "d" then
      return self._f.terd
   end
   return self._f.terr
end

function Triangle:set_ter(t)
   if self._d == "d" then
      self._f.terd = t
   else
      self._f.terr = t
   end
end

-- =================
-- A changed Field
-- =================
f_Field = {}
function f_Field:new(f, ter_d, ter_r)
   local rv = {
      _f = f,
      _td = ter_d,
      _tr = ter_r,
      __hash = ("%i_%i_%s_%s"):format(f.x, f.y, ter_d, ter_r),
   }
   setmetatable(rv,self)
   self.__index = self

   return rv
end
