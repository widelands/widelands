-- ==============
-- Testing roads 
-- ==============
road_tests = lunit.TestCase("road tests")
function road_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.p:place_flag(f)
   self.r = self.p:place_road(self.f, "r", "r")
end
function road_tests:teardown()
   pcall(function()
      f1 = self.r.start_flag
      f2 = self.r.end_flag
      f1:remove() -- should remove road as well
      f2:remove() 
   end)
end

function road_tests:test_length()
   assert_equal(2, self.r.length)
   -- TODO: remove both flags!
end

function road_tests:test_deletion_of_flag()
   self.r.start_flag:remove()
   assert_error("Road vanished!", function() self.r:length() end)
end

-- TODO: test for error if flag is not already set at beginning
