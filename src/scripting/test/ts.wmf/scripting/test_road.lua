-- ==============
-- Testing roads 
-- ==============
road_tests = lunit.TestCase("road tests")
function road_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.p:place_flag(f)
end

function road_tests:test_length()
   r = self.p:place_road(self.f, "r", "r")
   assert_equal(2, r.length)
   r:remove()
   -- TODO: remove both flags!
end


-- TODO: test for error if flag is not already set at beginning
