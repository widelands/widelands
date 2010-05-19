-- =======================================================================
--                        Water rising Functionality                        
-- =======================================================================

use("aux", "set")

WaterRiser = {}
function WaterRiser:new(seeds, g_water_level)
   local rv = {
      _water_level = g_water_level or 10,
      _ncoroutines = 0,
      _step = false,
      _future_seeding_fields = Set:new(seeds),
      _current_seeding_fields = Set:new{},
      _normalization_fields = Set:new(), 
   }

   setmetatable(rv, self)
   self.__index = self

   for x=1,wl.map.get_width()-1 do
      for y=1,wl.map.get_height()-1 do
         local f = wl.map.Field(x,y)
         if rv:_is_overflooded(f) then -- all water fields atm
            rv._normalization_fields:add(f)
         end
      end
   end
   -- Exclude the see in the middle of the map as long as
   -- it is not connected to the open see
   rv._normalization_fields = rv._normalization_fields -
      Set:new(wl.map.Field(75, 80):region(12))

   return rv
end

-- Check if this field is under water
function WaterRiser:_is_overflooded(f)
   if f.tln.terr == "wasser" and f.tln.terd == "wasser" and
      f.terr == "wasser" and f.terd == "wasser" and
      f.trn.terd == "wasser" and f.ln.terr == "wasser"
   then
      self._normalization_fields:add(f)

      if f.height < self._water_level then
         f.height = self._water_level
         for idx, nf in ipairs{f.trn, f.tln, f.rn, f.ln, f.brn, f.bln} do
            self:_is_overflooded(nf)
         end
         return true
      elseif f.height == self._water_level then 
         return true
      end
   end
   return false
end

function WaterRiser:_rise_for(f, sleeptime)
   -- If we can't do anything yet, put it back to be considered
   -- when the water rises again
   if f.height > self._water_level then
      if f.terr ~= "wasser" or f.terd ~= "wasser" then
         self._future_seeding_fields:add(f)
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
         f.height = self._water_level

         self._normalization_fields:add(f)

         for idx, nf in ipairs{f.trn, f.tln, f.rn, f.ln, f.brn, f.bln} do
            if not self:_is_overflooded(f) then
               self._current_seeding_fields:add(nf)
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
         self._step = 1;
         sleep(300)
      end
   end)
end

function WaterRiser:_do_normalize()
   print("size: ", self._normalization_fields.size)
   local st = os.clock()

   local new_overflooded_fields = Set:new()
   local function _check_if_newly_flooded(f)
      if self:_is_overflooded(f) and
         not self._normalization_fields:contains(f)
      then
         new_overflooded_fields:add(f)
      end
   end

   print ("Buh!")
   local ncount = 0
   print("Beginning head of loop!")
   for f in self._normalization_fields:items() do
      print("In the loop!")
      -- TODO: this is stupid. The first is_overflooded drags all other fields
      -- with it. So nothing is won by the sleep inside the loop.
      if self:_is_overflooded(f) then
         f.height = self._water_level
         _check_if_newly_flooded(f)
         _check_if_newly_flooded(f.rn)
         _check_if_newly_flooded(f.ln)
         _check_if_newly_flooded(f.brn)
         _check_if_newly_flooded(f.bln)
         _check_if_newly_flooded(f.trn)
         _check_if_newly_flooded(f.tln)
      end
      if ncount > 1000 then
         print("Sleeping half a second!")
         sleep(500)
         ncount = 0
      end
      ncount = ncount + 1
   end
   print("End of loop!")
   self._normalization_fields = self._normalization_fields +
      new_overflooded_fields

   local delta = os.clock() - st
   print("### Took: ", delta)

end
function WaterRiser:rise()
   self._water_level = self._water_level + 1

   self._current_seeding_fields = self._current_seeding_fields + 
      self._future_seeding_fields
  
   -- Normalize all fields, then flood new ones
   run(function()
      self:_do_normalize()

      repeat
         if self._current_seeding_fields.size > 0 then 
            local f = self._current_seeding_fields:pop_at(
               math.random(self._current_seeding_fields.size)
            )
            run(function()
               self:_rise_for(f, math.random(300,1000))
            end)
         end
         sleep(10)
      until self._ncoroutines == 0 and self._current_seeding_fields.size == 0
      print("and size: ", self._current_seeding_fields.size)
      -- Make sure that the function has run at least once.
      sleep(3000)
   end)

end


