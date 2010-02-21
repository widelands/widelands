-- ==============
-- Testing roads 
-- ==============

-- TODO: proper and more complete build tests, but maybe with convenience functions
-- TODO: building a road that crosses itself, check for error

-- TODO: second carrier only on busy road allowed


road_tests = lunit.TestCase("road tests")
function road_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.start_flag = self.p:place_flag(self.f)
   self.r = self.p:place_road(self.start_flag, "r", "r")
   self.end_flag = wl.map.Field(12,10).immovable
end
function road_tests:teardown()
   pcall(function()
      f1 = self.r.start_flag
      f2 = self.r.end_flag
      f1:remove() -- removes road as well
      f2:remove() 
   end)
end

function road_tests:test_length()
   assert_equal(2, self.r.length)
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


