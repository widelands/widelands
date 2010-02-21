-- ==============
-- Testing roads 
-- ==============

-- TODO: proper and more complete build tests, but maybe with convenience functions

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

