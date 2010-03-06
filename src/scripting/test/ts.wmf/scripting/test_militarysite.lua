-- =======================================================================
--                          MilitarySite Testings                         
-- =======================================================================

-- ================
-- Soldier creation 
-- ================
militarysite_tests = lunit.TestCase("MilitarySite Tests")
function militarysite_tests:setup()
   self.f1 = wl.map.Field(10,10)
   self.p = wl.game.Player(1)

   self.sentry = self.p:place_building("sentry", self.f1)
end
function militarysite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
end
function militarysite_tests:test_no_soldiers_initially()
   assert_equal(0, self.sentry:get_soldiers())
end
function militarysite_tests:test_warp_soldiers_one_at_a_time()
   self.sentry:warp_soldiers{[{0,0,0,0}] = 1}
   assert_equal(1, self.sentry:get_soldiers())
   assert_equal(1, self.sentry:get_soldiers({0,0,0,0}))
   self.sentry:warp_soldiers{[{3,0,0,1}] = 1}
   assert_equal(2, self.sentry:get_soldiers())
   assert_equal(1, self.sentry:get_soldiers({0,0,0,0}))
   assert_equal(1, self.sentry:get_soldiers({3,0,0,1}))
end
function militarysite_tests:test_warp_soldiers_all_at_once()
   self.sentry:warp_soldiers{[{0,0,0,0}] = 2}
   assert_equal(2, self.sentry:get_soldiers())
   assert_equal(2, self.sentry:get_soldiers({0,0,0,0}))
end
function militarysite_tests:test_illegal_soldier()
   assert_error("illegal name", function()
      self.sentry:warp_soldiers{[{10,0,0,0}] = 1}
   end)
end
function militarysite_tests:test_no_space()
   self.sentry:warp_soldiers{[{0,0,0,0}] = 1}
   assert_error("no_space", function()
      self.sentry:warp_soldiers{[{0,0,0,0}] = 2}
  end)
   assert_equal(2, self.sentry:get_soldiers())
end

