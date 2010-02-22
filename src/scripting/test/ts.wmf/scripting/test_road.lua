-- ==============
-- Testing roads 
-- ==============

-- ===================
-- Construction tests 
-- ===================
road_construction_tests = lunit.TestCase("road construction tests")
function road_construction_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.start_flag = self.p:place_flag(self.f)
   self.flags = {
      self.start_flag
   }
end
function road_construction_tests:teardown()
   pcall(function()
      for temp,f in ipairs(self.flags) do
         f:remove()
      end
   end)
end
function road_construction_tests:test_simple()
   r = self.p:place_road(self.start_flag,"r","br","r")
   self.flags[#self.flags+1] = self.f.rn.brn.rn.immovable
   assert_equal(3, r.length)
end
function road_construction_tests:test_too_short()
   assert_error("can't place flag", function()
      self.p:place_road(self.start_flag,"r")
   end)
end
function road_construction_tests:test_connect_two_flags()
   self.flags[#self.flags+1] = self.p:place_flag(self.f.rn.rn)
   r = self.p:place_road(self.start_flag,"r", "r")
   assert_equal(2, r.length)
end
function road_construction_tests:test_road_crosses_another()
   self.flags[#self.flags+1] = self.p:place_flag(self.f.rn.rn.rn.rn)
   r = self.p:place_road(self.start_flag,"r", "r", "r", "r")
   assert_equal(4, r.length)
   self.flags[#self.flags+1] = self.p:place_flag(self.f.rn.rn.tln.tln)
   self.flags[#self.flags+1] = self.p:place_flag(self.f.rn.rn.brn.brn)
   assert_error("Immovable in the way", function()
      r = self.p:place_road(self.flags[3],"br", "br", "br", "br")
   end)
end
function road_construction_tests:test_road_is_closed_loop()
   assert_error("Cannot build closed loop", function()
      self.p:place_road(self.start_flag,"r", "r", "tl", "tl", "bl", "bl")
   end)
end
function road_construction_tests:test_road_crosses_itself()
   assert_error("The road crosses itself", function()
      self.p:place_road(self.start_flag,"r", "r", "r", "tl", "bl", "bl")
   end)
end

-- ============
-- Usage tests 
-- ============
road_tests = lunit.TestCase("road tests")
function road_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.start_flag = self.p:place_flag(self.f)

   self.r = self.p:place_road(self.start_flag, "r", "r")
   self.end_flag = self.f.rn.rn.immovable
end
function road_tests:teardown()
   flags = {self.start_flag, self.end_flag}
   for temp, f in ipairs(flags) do
      pcall(function() f:remove() end)
   end
end

function road_tests:test_length()
   assert_equal(2, self.r.length)
end
function road_tests:test_roadtype()
   assert_equal("normal", self.r.type)
end

function road_tests:test_deletion_of_flag()
   self.r.start_flag:remove()
   assert_error("Road vanished!", function() self.r:length() end)
end
function road_tests:test_access_of_flags()
   assert_equal(self.end_flag, self.r.end_flag)
   assert_equal(self.start_flag, self.r.start_flag)
end

function road_tests:test_no_worker_at_creation()
   assert_equal(0, #self.r.workers)
end
function road_tests:test_carrier_creation()
   self.r:warp_worker("carrier")
   assert_equal(1, #self.r.workers)
   assert_equal("carrier", self.r.workers[1])
end
function road_tests:test_carrier_creation2()
   self.r:warp_worker("carrier")
   self.r:warp_worker("ox", 1)
   assert_equal(2, #self.r.workers)
   assert_equal("carrier", self.r.workers[1])
   assert_equal("ox", self.r.workers[2])
end
function road_tests:test_carrier_creation_not_a_carrier()
   self.r:warp_worker("carrier", 0)
   assert_error("Not an carrier!", function()
      self.r:warp_worker("lumberjack", 1)
   end)
end
function road_tests:test_carrier_creation_illegal_name()
   assert_error("Illegal name", function()
      self.r:warp_worker("kjhskjh", 0)
   end)
   assert_error("Illegal name", function()
      self.r:warp_worker("lkjshdf", 1)
   end)
end
function road_tests:test_carrier_creation_illegal_slot()
   assert_error("Slot 0 must be 'carrier'", function()
      self.r:warp_worker("ox", 0)
   end)
end
function road_tests:test_carrier_replace_one()
   self.r:warp_worker("carrier")
   self.r:warp_worker("ox", 1)
   assert_equal(2, #self.r.workers)
   assert_equal("ox", self.r.workers[2])
   self.r:warp_worker("carrier", 1)
   assert_equal(2, #self.r.workers)
   assert_equal("carrier", self.r.workers[2])
end
function road_tests:test_road_upcasting()
   i = self.f.rn.immovable
   assert_equal(i, self.r)
   assert_equal(2, i.length)
end


