-- =======================================================================
--                          MilitarySite Testings                         
-- =======================================================================

-- ================
-- Soldier creation 
-- ================
function _cnt(a)
   local rv = 0
   for sd, c in pairs(a) do rv = rv + c end
   return rv
end

militarysite_tests = lunit.TestCase("MilitarySite Tests")
function militarysite_tests:setup()
   self.f1 = map:get_field(10,10)

   self.fortress = player1:place_building("fortress", self.f1)
end
function militarysite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
end
function militarysite_tests:test_name()
   assert_equal("fortress", self.fortress.name)
end
function militarysite_tests:test_type()
   assert_equal("militarysite", self.fortress.type)
end
function militarysite_tests:test_no_soldiers_initially()
   assert_equal(0, _cnt(self.fortress:get_soldiers("all")))
end
function militarysite_tests:test_max_soldiers()
   assert_equal(8, self.fortress.max_soldiers);
end
function militarysite_tests:test_set_soldiers_single_arg()
   self.fortress:set_soldiers({0,0,0,0}, 2)
   assert_equal(2, self.fortress:get_soldiers({0,0,0,0}))
end
function militarysite_tests:test_set_soldiers_multi_arg()
   self.fortress:set_soldiers{
      [{0,0,0,0}] = 2,
      [{1,1,0,1}] = 3
   }
   assert_equal(5, _cnt(self.fortress:get_soldiers("all")))
   assert_equal(2, self.fortress:get_soldiers({0,0,0,0}))
   assert_equal(3, self.fortress:get_soldiers({1,1,0,1}))
end
function militarysite_tests:test_set_soldiers_add_and_remove()
   self.fortress:set_soldiers{[{0,0,0,0}] = 3}
   assert_equal(3, _cnt(self.fortress:get_soldiers("all")))
   assert_equal(3, self.fortress:get_soldiers({0,0,0,0}))
   self.fortress:set_soldiers{[{3,0,0,1}] = 1, [{0,0,0,0}] = 1}
   assert_equal(2, _cnt(self.fortress:get_soldiers("all")))
   assert_equal(1, self.fortress:get_soldiers({0,0,0,0}))
   assert_equal(1, self.fortress:get_soldiers({3,0,0,1}))
   self.fortress:set_soldiers({3,2,0,1}, 1)
   assert_equal(1, _cnt(self.fortress:get_soldiers("all")))
   assert_equal(0, self.fortress:get_soldiers({0,0,0,0}))
   assert_equal(0, self.fortress:get_soldiers({3,0,0,1}))
   assert_equal(1, self.fortress:get_soldiers({3,2,0,1}))
end

function militarysite_tests:test_set_soldiers_all_at_once()
   self.fortress:set_soldiers{[{0,0,0,0}] = self.fortress.max_soldiers}
   assert_equal(self.fortress.max_soldiers,
      _cnt(self.fortress:get_soldiers("all")))
   assert_equal(self.fortress.max_soldiers,
      self.fortress:get_soldiers({0,0,0,0}))
end
function militarysite_tests:test_illegal_soldier()
   assert_error("illegal level", function()
      self.fortress:set_soldiers{[{10,0,0,0}] = 1}
   end)
end
function militarysite_tests:test_no_space()
   assert_error("no_space", function()
      self.fortress:set_soldiers{[{0,0,0,0}] = 9}
  end)
   assert_equal(8, _cnt(self.fortress:get_soldiers("all")))
end

