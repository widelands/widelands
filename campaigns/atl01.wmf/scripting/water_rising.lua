-- =======================================================================
--                        Water rising Functionality
-- =======================================================================

include "scripting/set.lua"

-- ================
-- Helper function
-- ================
function _format_time(t)
   local mins = math.floor(t/1000 / 60)
   t = t - mins * 1000 * 60
   local secs = math.floor(t/1000)

   if mins > 0 then
      return ("%im%isec"):format(mins,secs)
   else
      return ("%isec"):format(secs)
   end
end


function _fully_flooded(f)
   if f.terd == "wasser" and f.terr == "wasser" and
      f.tln.terr == "wasser" and f.tln.terd == "wasser" and
      f.ln.terr == "wasser" and f.trn.terd == "wasser" then
      return true
   end
   return false
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

function Triangle:get_height()
   if self._d == "d" then
      return (self._f.height + self._f.bln.height + self._f.brn.height) / 3
   else
      return (self._f.height + self._f.brn.height + self._f.rn.height) / 3
   end
end

function Triangle:fields()
   if self._d == "d" then
      return {self._f, self._f.bln, self._f.brn}
   else
      return {self._f, self._f.brn, self._f.rn}
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

   local triangles_to_check = Set:new{
      Triangle:new(ocean_seed, "r"),
      Triangle:new(ocean_seed, "d"),
   }

   while triangles_to_check.size > 0 do
      local tr = triangles_to_check:pop_at(1)
      if tr:get_ter() == "wasser" and tr:get_height() <= rv._water_level then
         rv._ocean:add(tr)
         for idx, ntr in ipairs(tr:neighbours()) do
            if not (rv._ocean:contains(ntr) or rv._shore:contains(ntr)) then
               triangles_to_check:add(ntr)
            end
         end
      else
         rv._shore:add(tr)
      end
   end

   return rv
end

function WaterRiser:rise(level)
   run(function()
   while self._water_level < level do
      self._water_level = self._water_level + 1

      print(("Beginning rise to: %i"):format(self._water_level))
      local st = game.time

      self:_relevel_ocean()

      local delta = game.time - st
      print(("Done with normalization, took %s"):format(_format_time(delta)))

      self:_reevaluate_shore()

      self:_rise_water()

      local delta = game.time - st
      print(("Raising to %i took %s"):format(self._water_level,
         _format_time(delta)))
   end
   end)
end

function WaterRiser:_relevel_ocean()
   -- Relevels the ocean over 7.5 mins
   local scnt = math.floor(self._ocean.size / 450)
   local cnt = scnt
   for tr in self._ocean:items() do
      for idx,f in ipairs(tr:fields()) do
         if _fully_flooded(f) then
            f.raw_height = self._water_level
         end
      end
      cnt = cnt - 1
      if cnt == 0 then
         cnt = scnt
         sleep(1000)
      end
   end
   map:recalculate()
end

function WaterRiser:_reevaluate_shore()
   -- Check for all shore fields if they remain shore or are going to be
   -- reflooded
   for tr in self._shore:items() do
      if tr:get_height() <= self._water_level then
         self._shore:discard(tr)
         self._to_flood:add(tr)
      end
   end
end

function WaterRiser:_rise_water()
   -- Rises the water, floods the land up to the current water level
   while self._to_flood.size > 0 do
      local tr = self._to_flood:pop_at(math.random(self._to_flood.size))

      tr:set_ter("wasser")
      for idx,f in ipairs(tr:fields()) do
         if _fully_flooded(f) then
            if self.field_flooded_callback then
               self.field_flooded_callback(f)
            end

            f.height = self._water_level
            if f.immovable then f.immovable:remove() end
            for idx,b in ipairs(f.bobs) do
               if not b:has_caps("swims") then b:remove() end
            end
         end
      end

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
