-- =======================================================================
--                                Field test                               
-- =======================================================================
field_tests = lunit.TestCase("Field creation")
function field_tests:test_create()
   c = wl.map.Field(25,32)
   assert_equal(c.x, 25)
   assert_equal(c.y, 32)
end
function field_tests:test_create_xistobig()
   assert_error("x should be too big", function() wl.map.Field(64, 23) end)
end
function field_tests:test_create_yistobig()
   assert_error("y should be too big", function() wl.map.Field(25, 80) end)
end
function field_tests:test_create_xisnegativ()
   assert_error("x is negativ", function() wl.map.Field(-12, 23) end)
end
function field_tests:test_create_yisnegativ()
   assert_error("y is negativ", function() wl.map.Field(25, -12) end)
end
function field_tests:test_direct_change_impossible()
   assert_error("c.x should be read only", function() c.x = 12 end) 
   assert_error("c.y should be read only", function() c.y = 12 end) 
end
function field_tests:test_r_neighbour()
   c = wl.map.Field(25,40)
   assert_equal(wl.map.Field(26,40), c.rn)
   c = wl.map.Field(63,40)
   assert_equal(wl.map.Field(0,40), c.rn)
end
function field_tests:test_l_neighbour()
   c = wl.map.Field(25,40)
   assert_equal(wl.map.Field(24,40), c.ln)
   c = wl.map.Field(0,40)
   assert_equal(wl.map.Field(63,40), c.ln)
end
function field_tests:test_trn_neighbour()
   c = wl.map.Field(25,40)
   assert_equal(wl.map.Field(25,39), c.trn)
   assert_equal(wl.map.Field(26,38), c.trn.trn)
   assert_equal(wl.map.Field(26,37), c.trn.trn.trn)
   c = wl.map.Field(0,0)
   assert_equal(wl.map.Field(0,79), c.trn)
   assert_equal(wl.map.Field(1,78), c.trn.trn)
   assert_equal(wl.map.Field(1,77), c.trn.trn.trn)
end
function field_tests:test_tln_neighbour()
   c = wl.map.Field(25,40)
   assert_equal(wl.map.Field(24,39), c.tln)
   assert_equal(wl.map.Field(24,38), c.tln.tln)
   assert_equal(wl.map.Field(23,37), c.tln.tln.tln)
   c = wl.map.Field(1,1)
   assert_equal(wl.map.Field(1,0), c.tln)
   assert_equal(wl.map.Field(0,79), c.tln.tln)
   assert_equal(wl.map.Field(0,78), c.tln.tln.tln)
end
function field_tests:test_bln_neighbour()
   c = wl.map.Field(26,37)
   assert_equal(wl.map.Field(26,38), c.bln)
   assert_equal(wl.map.Field(25,39), c.bln.bln)
   assert_equal(wl.map.Field(25,40), c.bln.bln.bln)

   c = wl.map.Field(1,77)
   assert_equal(wl.map.Field(1,78), c.bln)
   assert_equal(wl.map.Field(0,79), c.bln.bln)
   assert_equal(wl.map.Field(0,0), c.bln.bln.bln)
end
function field_tests:test_brn_neighbour()
   c = wl.map.Field(35,22)
   assert_equal(wl.map.Field(35,23), c.brn)
   assert_equal(wl.map.Field(36,24), c.brn.brn)
   assert_equal(wl.map.Field(36,25), c.brn.brn.brn)

   c = wl.map.Field(63,79)
   assert_equal(wl.map.Field(0,0), c.brn)
   assert_equal(wl.map.Field(0,1), c.brn.brn)
   assert_equal(wl.map.Field(1,2), c.brn.brn.brn)
end
function field_tests:test_equality()
   c = wl.map.Field(32,33)
   c1 = wl.map.Field(32,33)

   assert_equal(c,c1)
end
function field_tests:test_inequality()
   c = wl.map.Field(32,33)
   c1 = wl.map.Field(33,32)

   assert_not_equal(c,c1)
end
function field_tests:test_getting_height()
   assert_equal(10, wl.map.Field(32,33).height)
end
function field_tests:test_setting_height()
   f = wl.map.Field(32,33)
   f.height = 12
   assert_equal(12, wl.map.Field(32,33).height)
   f.height = 10
end
function field_tests:test_setting_illegal_negativ_height()
   assert_error("height must be positive", function()
      wl.map.Field(50,40).height =  -12
   end)
end
function field_tests:test_setting_illegal_too_big_height()
   assert_error("height must be <= 60", function()
      wl.map.Field(50,40).height = 61
   end)
end
function field_tests:test_getting_terr()
   assert_equal("steppe", wl.map.Field(50,40).terr)
end
function field_tests:test_getting_terd()
   assert_equal("steppe", wl.map.Field(50,40).terd)
end
function field_tests:test_setting_terr()
   f = wl.map.Field(50,40)
   f.terr = "wasser"
   assert_equal("wasser", f.terr)
   f.terr = "steppe"
end
function field_tests:test_setting_terd()
   f = wl.map.Field(50,40)
   f.terd = "wasser"
   assert_equal("wasser", f.terd)
   f.terd = "steppe"
end
function field_tests:test_region_radius_zero()
   f = wl.map.Field(50,40)
   list = f:region(0)
   assert_equal(1, #list)
   assert_equal(f, list[1])
end
function field_tests:test_region_radius_one()
   f = wl.map.Field(50,40)
   list = f:region(1)
   assert_equal(7, #list)
end
function field_tests:test_hollow_region_radius_one()
   f = wl.map.Field(50,40)
   list = f:region(2,1)
   assert_equal(13, #list)
end

-- ==========
-- Resources 
-- ==========
field_resources_tests = lunit.TestCase("Field resources test")
function field_resources_tests:setup()
   self.f = wl.map.Field(60,40)
   self._amount = self.f.resource_amount
   self._terr = self.f.terr
   self._terd = self.f.terd
end
function field_resources_tests:teardown()
   self.f.terr = self._terr
   self.f.terd = self._terd
   self.f.resource_amount = self._amount 
end
function field_resources_tests:test_default_resource()
   assert_equal("water", self.f.resource)
   assert_equal(5, self.f.resource_amount)
end
function field_resources_tests:test_set_resource_amount()
   self.f.resource_amount = 25
   assert_equal("water", self.f.resource)
   assert_equal(25, self.f.resource_amount)
end
function field_resources_tests:test_set_resource_amount_negative()
   assert_error("negative forbidden!",
      function() self.f.resource_amount = -100 end)
end
function field_resources_tests:test_set_resource_amount_too_much()
   assert_error("too big!", function () self.f.resource_amount = 1000 end)
end
function field_resources_tests:test_set_resource_type()
   self.f.resource = "coal"
   assert_equal("coal", self.f.resource)
   assert_equal(5, self.f.resource_amount)
end
function field_resources_tests:test_set_resource_type_illegal_resource()
   assert_error("Illegal name!", function() self.f.resource = "ksjdjhsdf" end)
end





