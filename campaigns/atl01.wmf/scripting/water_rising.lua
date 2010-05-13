-- =======================================================================
--                        Water rising Functionality                        
-- =======================================================================


WaterRiser = {}
function WaterRiser:new(seeds, g_water_level)
   local rv = {
      _water_level = g_water_level or 9,
      _ncoroutines = 0,
      _step = false,
      _seeding_fields = seeds,
      _normalization_fields = {}, 
   }
   setmetatable(rv, self)
   self.__index = self

   return rv
end

function WaterRiser:_check_for_normalize(f)
   if not f.terr == "wasser" or
      not f.terd == "wasser" or
      f.height >= self._water_level
   then
      return
   end

   self._normalization_fields[#self._normalization_fields + 1] = f
end

function WaterRiser:_normalize()
   if #self._normalization_fields > 0 then 
      local idx = math.random(#self._normalization_fields)
      local f = self._normalization_fields[idx]
      table.remove(self._normalization_fields, idx)

      f.height = self._water_level

      self:_check_for_normalize(f.rn)
      self:_check_for_normalize(f.ln)
      self:_check_for_normalize(f.brn)
      self:_check_for_normalize(f.bln)
      self:_check_for_normalize(f.trn)
      self:_check_for_normalize(f.tln)
   end
end

function WaterRiser:_rise_for(f, sleeptime)
   -- If we can't do anything yet, put is back to be considered
   -- later
   if f.height > self._water_level then
      if f.terr ~= "wasser" or f.terd ~= "wasser" then
         self._seeding_fields[#self._seeding_fields + 1] = f
      end
      return
   end
   self._ncoroutines = self._ncoroutines + 1

   while not (f.terr == "wasser" and f.terd == "wasser") do
      if self._step then
         print(("Caring for %i:%i, n: %i"):format(f.x, f.y, self._ncoroutines))

         if f.terr ~= "wasser" then
            f.terr = "wasser"
            self._step = false
            sleep(sleeptime)
         end
         if f.terd ~= "wasser" then
            f.terd = "wasser"
            self._step = false
            sleep(sleeptime)
         end
         if f.immovable then f.immovable:remove() end
         
         self:_check_for_normalize(f)

         for idx, nf in ipairs{f.trn, f.tln, f.rn, f.ln, f.brn, f.bln} do
            if nf.terr ~= "wasser" or nf.terd ~= "wasser" then 
               self._seeding_fields[#self._seeding_fields + 1] = nf
            end
         end
      end
      sleep(sleeptime)
   end
   
   self._ncoroutines = self._ncoroutines - 1
end

function WaterRiser:run()
   run(function()
      while true do
         local old_seeding_fields = self._seeding_fields
         self._seeding_fields = {}
         while #old_seeding_fields > 0 do 
            local idx = math.random(#old_seeding_fields)
            local f = old_seeding_fields[idx]

            run(function()
               self:_rise_for(f, math.random(300,1000))
            end)
            table.remove(old_seeding_fields, idx)
         end
         sleep(3000)
      end
   end)

   run(function()
      while true do 
         self._step = 1;
         self:_normalize()
         -- self:_normalize()
         sleep(300)
      end
   end)
end

function WaterRiser:rise()
   self._water_level = self._water_level + 1
end


