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
function flag_tests:test_no_wares_on_creation()
   local rv = self.f:get_wares("all")
   cnt = 0
   for name,c in pairs(rv) do cnt = cnt + c end
   assert_equal(0, cnt)
end
function flag_tests:test_add_ware()
   self.f:add_ware("trunk")
   self.f:add_ware("trunk")
   self.f:add_ware("raw_stone")
   self.f:add_ware("raw_stone")
   self.f:add_ware("coal")
   assert_equal(2, self.f:get_wares("all").trunk)
   assert_equal(2, self.f:get_wares("all").raw_stone)
   assert_equal(1, self.f:get_wares("all").coal)
end

