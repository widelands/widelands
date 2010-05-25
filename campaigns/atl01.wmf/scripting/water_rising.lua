-- =======================================================================
--                        Water rising Functionality                        
-- =======================================================================

use("aux", "set")


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

function Triangle:__tostring()
   return self.__hash
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

function Triangle:set_height(h, g_property)
   local property = g_property or "height"

   if self._d == "d" then
      self._f[property] = h
      self._f.bln[property] = h
      self._f.brn[property] = h
   else
      self._f[property] = h
      self._f.brn[property] = h
      self._f.rn[property] = h
   end
end

function Triangle:get_height()
   if self._d == "d" then
      return (self._f.height + self._f.bln.height + self._f.brn.height) / 3
   else
      return (self._f.height + self._f.brn.height + self._f.rn.height) / 3
   end
end

function Triangle:neighbours()
   if self._d == "d" then
      return {
         Triangle:new(self._f, "r"),
         Triangle:new(self._f.ln, "r"),
         Triangle:new(self._f.bln, "r"),
      }
   else
      return {
         Triangle:new(self._f.rn, "d"),
         Triangle:new(self._f.trn, "d"),
         Triangle:new(self._f, "d"),
      }
   end
end

-- ==========
-- WaterRiser
-- ==========
WaterRiser = {}
function WaterRiser:new(ocean_seed)
   local rv = {
      _water_level = ocean_seed.height,
      _ocean = Set:new(),
      _shore = Set:new(),
      _to_flood = Set:new(),
   }

   setmetatable(rv, self)
   self.__index = self

   local check_fields = Set:new{
      Triangle:new(ocean_seed, "r"),
      Triangle:new(ocean_seed, "d"),
   }

   while check_fields.size > 0 do
      local tr = check_fields:pop_at(1)
      if tr:get_ter() == "wasser" and tr:get_height() <= rv._water_level then
         rv._ocean:add(tr)
         for idx, ntr in ipairs(tr:neighbours()) do
            if not (rv._ocean:contains(ntr) or rv._shore:contains(ntr)) then
               check_fields:add(ntr)
            end
         end
      else
         rv._shore:add(tr)
      end
   end

   return rv
end

function WaterRiser:_rise_water()
   while self._to_flood.size > 0 do
      print("In loop: ", self._to_flood.size, self._shore.size, self._ocean.size)
      local tr = self._to_flood:pop_at(math.random(self._to_flood.size))

      tr:set_ter("wasser")
      tr:set_height(self._water_level, "raw_height")
      self._ocean:add(tr)

      -- Check the neighbours
      for idx, ntr in ipairs(tr:neighbours()) do
         if not (self._ocean:contains(ntr) or self._shore:contains(ntr)) then
            if ntr:get_height() <= self._water_level then
               self._to_flood:add(ntr)
            else
               self._shore:add(ntr)
            end
         end
      end
      sleep(300)
   end
end

function WaterRiser:rise()
   self._water_level = self._water_level + 1
   
   -- Relevel the ocean
   for tr in self._ocean:items() do
      tr:set_height(self._water_level, "raw_height")
   end
   wl.map.recalculate()

   -- Check for all shore fields if they remain shore or
   -- are going to be reflooded
   for tr in self._shore:items() do
      if tr:get_height() <= self._water_level then
         self._shore:discard(tr)
         self._to_flood:add(tr)
      end
   end

   -- Launch the raising function
   run(function() self:_rise_water() end)

end

function WaterRiser:set(what, t)
   if what == "s" then
      for tr in self._shore:items() do
         tr:set_ter(t)
      end
   elseif what == "o" then
      for tr in self._ocean:items() do
         tr:set_ter(t)
      end
   elseif what == "f" then
      for tr in self._to_flood:items() do
         tr:set_ter(t)
      end
   end
end

function s()
   local st = os.clock()
   wr = WaterRiser:new(wl.map.Field(92,19))

   local delta = os.clock() - st
   print("Took: ", delta)
   print(wr._ocean.size)
   print(wr._shore.size)
end


