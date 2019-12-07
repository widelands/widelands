function _cnt_wares(f)
   local rv = 0
   for name,cnt in pairs(f:get_wares("all")) do rv = rv + cnt end
   return rv
end
-- ===================
-- Flag functionality
-- ===================
flag_tests = lunit.TestCase("flag tests")
function flag_tests:setup()
   self.field = map:get_field(13,10)
   self.f = player1:place_flag(self.field, 1)
end
function flag_tests:teardown()
   pcall(function() self.f:remove() end)
   pcall(function() self.f1:remove() end)
   pcall(function() self.f2:remove() end)
   pcall(function() self.w:remove() end)
end

function flag_tests:test_name()
   assert_equal("flag", self.f.descr.name)
end
function flag_tests:test_type()
   assert_equal("flag", self.f.descr.type_name)
end
function flag_tests:test_fields()
   local f = self.f.fields
   assert_equal(1, #f)
   assert_equal(self.field, f[1])
end
function flag_tests:test_no_wares_on_creation()
   local rv = self.f:get_wares("all")
   cnt = 0
   for name,c in pairs(rv) do cnt = cnt + c end
   assert_equal(0, cnt)
end
function flag_tests:test_no_wares_on_creation_single_arg()
   assert_equal(0, self.f:get_wares("log"))
end
function flag_tests:test_no_wares_on_creation_array_arg()
   local rv = self.f:get_wares{"log", "coal"}
   assert_equal(0, rv.log)
   assert_equal(0, rv.coal)
   assert_equal(nil, rv.granite)
end
function flag_tests:test_set_wares_two_args()
   self.f:set_wares("log", 3)
   assert_equal(3, self.f:get_wares("log"))
end
function flag_tests:test_set_wares_one_arg()
   self.f:set_wares{
      log = 3,
      coal = 2,
   }
   assert_equal(3, self.f:get_wares("log"))
   assert_equal(2, self.f:get_wares("coal"))
end
function flag_tests:test_change_wares_on_flag()
   self.f:set_wares{log = 3, coal = 2}
   self.f:set_wares{fish = 3}
   assert_equal(3, self.f:get_wares("fish"))
   assert_equal(0, self.f:get_wares("log"))
   assert_equal(0, self.f:get_wares("coal"))
   assert_equal(3, _cnt_wares(self.f))
end
function flag_tests:test_change_wares_on_flag2()
   self.f:set_wares{log = 3, coal = 5}
   self.f:set_wares{log = 3, coal = 4, fish = 1}
   assert_equal(8, _cnt_wares(self.f))
   assert_equal(1, self.f:get_wares("fish"))
   assert_equal(3, self.f:get_wares("log"))
   assert_equal(4, self.f:get_wares("coal"))
end
function flag_tests:test_set_wares_illegal_ware()
   function ill()
      self.f:set_wares("kjhsh", 1)
   end
   assert_error("Illegal ware", ill)
   function ill1()
      self.f:set_wares{"log", "kjhsh"}
   end
   assert_error("Illegal ware", ill1)
end
function flag_tests:test_set_wares_many_wares()
   assert_error("Should be too much", function()
      self.f:set_wares("log", 9)
   end)
end

-- =========
-- get_ware
-- =========
flag_tests_get_ware = lunit.TestCase("flag tests: get_ware")
function flag_tests_get_ware:setup()
   self.f = player1:place_flag(map:get_field(13,10), 1)
   self.f:set_wares{log = 2, granite = 2, coal = 1}
end
function flag_tests_get_ware:teardown()
   pcall(self.f.remove, self.f)
end

function flag_tests_get_ware:test_get_ware_all()
   local rv = self.f:get_wares("all")
   assert_equal(2, rv.log)
   assert_equal(2, rv.granite)
   assert_equal(1, rv.coal)
   assert_equal(nil, rv.fish)
   local c = 0
   for name,cnt in pairs(rv) do c = c+cnt end
   assert_equal(5, c)
end
function flag_tests_get_ware:test_get_ware_one_returns_number()
   local rv = self.f:get_wares("log")
   assert_number(rv)
end
function flag_tests_get_ware:test_get_ware_one()
   assert_equal(2, self.f:get_wares("log"))
   assert_equal(2, self.f:get_wares("granite"))
   assert_equal(1, self.f:get_wares("coal"))
end
function flag_tests_get_ware:test_get_ware_one()
   assert_equal(2, self.f:get_wares("log"))
   assert_equal(2, self.f:get_wares("granite"))
   assert_equal(1, self.f:get_wares("coal"))
end

function flag_tests_get_ware:test_get_ware_many()
   local rv = self.f:get_wares{"log", "granite"}
   assert_equal(2, rv.log)
   assert_equal(2, rv.granite)
   assert_equal(nil, rv.coal)
   local c = 0
   for name,cnt in pairs(rv) do c = c+cnt end
   assert_equal(4, c)
end
function flag_tests:test_get_wares_non_existant_name()
   assert_error("non existent ware", function()
      self.f:get_wares("balloon")
   end)
   assert_error("non existent ware", function()
      self.f:get_wares{"meat", "balloon"}
   end)
end

-- ===================
-- buildings and roads
-- ===================
function flag_tests:roads_and_buildings_test()
   local field = map:get_field(4,14)
   self.w = player1:place_building("barbarians_warehouse", field)
   self.f1 = self.w.flag
   local road = player1:place_road(self.f1, "br", "br", "br")
   self.f2 = self.f1.fields[1].brn.brn.brn.immovable

   assert_equal(self.f2.roads.tl, road)
   assert_equal(self.f2.roads.tl.start_flag, self.f1)
   assert_equal(self.f2.roads.tl.end_flag, self.f2)
   assert_equal(self.f1.building, self.w)
   assert_nil(self.f2.building)
   assert_equal(self.f2.debug_ware_economy, self.w.debug_ware_economy)
   assert_equal(self.f2.debug_worker_economy, self.w.debug_worker_economy)
end
