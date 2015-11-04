set_textdomain("tribes")

test_descr = lunit.TestCase("Tribes descriptions test")

--  =======================================================
--  ****************** TribeDescription *******************
--  =======================================================

function test_descr:test_tribe_descr()
   assert_error("Wrong tribe", function() egbase:get_tribe_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function() egbase:get_tribe_description("XXX", "YYY") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_tribe_description("XXX", "YYY", "ZZZ") end)
end

function test_descr:test_descname()
   assert_equal(_"Atlanteans", egbase:get_tribe_description("atlanteans").descname)
   assert_equal(_"Barbarians", egbase:get_tribe_description("barbarians").descname)
   assert_equal(_"Empire", egbase:get_tribe_description("empire").descname)
end

function test_descr:test_name()
   assert_equal("atlanteans", egbase:get_tribe_description("atlanteans").name)
   assert_equal("barbarians", egbase:get_tribe_description("barbarians").name)
   assert_equal("empire", egbase:get_tribe_description("empire").name)
end

function test_descr:test_get_buildings()
   local nobuildings = function(t)
      local count = 0
      for _ in pairs(t) do count = count + 1 end
      return count
   end

   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(41, nobuildings(tribe.buildings))
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(50, nobuildings(tribe.buildings))
   tribe = egbase:get_tribe_description("empire")
   assert_equal(51, nobuildings(tribe.buildings))
end

function test_descr:test_get_carrier()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_carrier", tribe.carrier)
end

function test_descr:test_get_carrier2()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_horse", tribe.carrier2)
end

function test_descr:test_get_headquarters()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_headquarters", tribe.headquarters)
end

function test_descr:test_get_geologist()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_geologist", tribe.geologist)
end

function test_descr:test_get_port()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_port", tribe.port)
end

function test_descr:test_get_ship()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_ship", tribe.ship)
end

function test_descr:test_get_soldier()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_soldier", tribe.soldier)
end

function test_descr:test_get_wares()
   local tribe = egbase:get_tribe_description("atlanteans")
   local nowares = function(t)
      local count = 0
      for _ in pairs(t) do count = count + 1 end
      return count
   end

   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(44, nowares(tribe.wares))
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(40, nowares(tribe.wares))
   tribe = egbase:get_tribe_description("empire")
   assert_equal(44, nowares(tribe.wares))
end

function test_descr:test_get_workers()
   local tribe = egbase:get_tribe_description("atlanteans")
   local noworkers = function(t)
      local count = 0
      for _ in pairs(t) do count = count + 1 end
      return count
   end

   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(29, noworkers(tribe.workers))
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(31, noworkers(tribe.workers))
   tribe = egbase:get_tribe_description("empire")
   assert_equal(32, noworkers(tribe.workers))
end

function test_descr:test_has_building()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(true, tribe:has_building("atlanteans_quarry"))
   assert_equal(false, tribe:has_building("empire_quarry"))
   assert_equal(false, tribe:has_building("XXX"))
end

function test_descr:test_has_ware()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(true, tribe:has_ware("log"))
   assert_equal(false, tribe:has_ware("marble"))
   assert_equal(false, tribe:has_ware("XXX"))
end

function test_descr:test_has_worker()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(true, tribe:has_worker("atlanteans_forester"))
   assert_equal(false, tribe:has_worker("empire_forester"))
   assert_equal(false, tribe:has_worker("XXX"))
end
