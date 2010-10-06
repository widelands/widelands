-- ==============
-- Testing roads
-- ==============
function _cnt(i)
   local rv = 0
   for name,cnt in pairs(i) do rv = rv + cnt end
   return rv
end


-- ============
-- Usage tests
-- ============
road_tests = lunit.TestCase("road tests")
function road_tests:setup()
   self.f = map:get_field(10,10)
   player1:conquer(self.f, 6)

   self.start_flag = player1:place_flag(self.f)

   self.r = player1:place_road(self.start_flag, "r", "r", "br", "br")
   self.end_flag = self.f.rn.rn.brn.brn.immovable
end
function road_tests:teardown()
   flags = {self.start_flag, self.end_flag}
   for temp, f in ipairs(flags) do
      pcall(function() f:remove() end)
   end
end

function road_tests:test_length()
   assert_equal(4, self.r.length)
end
function road_tests:test_field()
   local f = self.r.fields
   assert_equal(#f, 3)
   assert_equal(self.f.rn, f[1])
   assert_equal(self.f.rn.rn, f[2])
   assert_equal(self.f.rn.rn.brn, f[3])
end
function road_tests:test_roadtype()
   assert_equal("normal", self.r.road_type)
end
function road_tests:test_type()
   assert_equal("road", self.r.type)
end
function road_tests:test_name()
   assert_equal("road", self.r.name)
end

function road_tests:test_deletion_of_flag()
   self.r.start_flag:remove()
   assert_error("Road vanished!", function() self.r:length() end)
end
function road_tests:test_access_of_flags()
   assert_equal(self.end_flag, self.r.end_flag)
   assert_equal(self.start_flag, self.r.start_flag)
end
function road_tests:test_road_upcasting()
   i = self.f.rn.immovable
   assert_equal(i, self.r)
   assert_equal(4, i.length)
end

-- ====================
-- Creating of carrier
-- ====================
function road_tests:test_no_worker_at_creation()
   assert_equal(0, _cnt(self.r:get_workers("all")))
end
function road_tests:test_carrier_creation()
   self.r:set_workers("carrier",1)
   assert_equal(1, _cnt(self.r:get_workers("all")))
   assert_equal(1, self.r:get_workers("carrier"))
end
function road_tests:test_carrier_creation_and_deletion()
   self.r:set_workers("carrier",1)
   assert_equal(1, _cnt(self.r:get_workers("all")))
   assert_equal(1, self.r:get_workers("carrier"))
   self.r:set_workers("carrier",0)
   assert_equal(0, _cnt(self.r:get_workers("all")))
   assert_equal(0, self.r:get_workers("carrier"))
   self.r:set_workers("carrier",1)
   assert_equal(1, _cnt(self.r:get_workers("all")))
   assert_equal(1, self.r:get_workers("carrier"))
end
function road_tests:test_carrier_creation_not_a_carrier()
   assert_error("Not an carrier!", function()
      self.r:set_workers{lumberjack=1}
   end)
end
function road_tests:test_carrier_creation_illegal_name()
   assert_error("Illegal name", function()
      self.r:set_workers("kjhskjh", 1)
   end)
end
function road_tests:test_carrier_no_space()
   self.r:set_workers{carrier=1}
   assert_error("No space!", function ()
      self.r:set_workers{carrier=2}
   end)
end
function road_tests:test_valid_workers()
   assert_equal(1, _cnt(self.r.valid_workers))
   assert_equal(1, self.r.valid_workers.carrier)
end

