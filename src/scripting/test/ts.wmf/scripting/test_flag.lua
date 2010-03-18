-- ===================
-- Flag functionality 
-- ===================
flag_tests = lunit.TestCase("flag tests")
function flag_tests:setup()
   self.p = wl.game.Player(1)
   self.f = self.p:place_flag(wl.map.Field(13,10), 1)
end
function flag_tests:teardown()
   pcall(self.f.remove, self.f)
end

function flag_tests:test_no_wares_on_creation()
   local rv = self.f:get_wares("all")
   cnt = 0
   for name,c in pairs(rv) do cnt = cnt + c end
   assert_equal(0, cnt)
end
function flag_tests:test_add_ware_illegal_ware()
   function ill()
      self.f:add_ware("kjhsh")
   end
   assert_error("Illegal ware", ill)
end
function flag_tests:test_add_too_many_wares()
   for i=1,7 do self.f:add_ware("trunk") end
   self.f:add_ware("raw_stone") -- should be fine

   function one_more() self.f:add_ware("trunk") end
   assert_error("Should be too much", one_more)
end

-- =========
-- get_ware 
-- =========
flag_tests_get_ware = lunit.TestCase("flag tests: get_ware")
function flag_tests_get_ware:setup()
   self.p = wl.game.Player(1)
   self.f = self.p:place_flag(wl.map.Field(13,10), 1)
   self.f:add_ware("trunk")
   self.f:add_ware("trunk")
   self.f:add_ware("raw_stone")
   self.f:add_ware("raw_stone")
   self.f:add_ware("coal")
end
function flag_tests_get_ware:teardown()
   pcall(self.f.remove, self.f)
end
   
function flag_tests_get_ware:test_get_ware_all()
   local rv = self.f:get_wares("all")
   assert_equal(2, rv.trunk)
   assert_equal(2, rv.raw_stone)
   assert_equal(1, rv.coal)
   local c = 0
   for name,cnt in pairs(rv) do c = c+cnt end
   assert_equal(5, c)
end
function flag_tests_get_ware:test_get_ware_one_returns_number()
   local rv = self.f:get_wares("trunk")
   assert_number(rv)
end
function flag_tests_get_ware:test_get_ware_one()
   assert_equal(2, self.f:get_wares("trunk"))
   assert_equal(2, self.f:get_wares("raw_stone"))
   assert_equal(1, self.f:get_wares("coal"))
end
function flag_tests_get_ware:test_get_ware_one()
   assert_equal(2, self.f:get_wares("trunk"))
   assert_equal(2, self.f:get_wares("raw_stone"))
   assert_equal(1, self.f:get_wares("coal"))
end

function flag_tests_get_ware:test_get_ware_many()
   local rv = self.f:get_wares{"trunk", "raw_stone"}
   assert_equal(2, rv.trunk)
   assert_equal(2, rv.raw_stone)
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



