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
   assert_equal(_("Atlanteans"), egbase:get_tribe_description("atlanteans").descname)
   assert_equal(_("Barbarians"), egbase:get_tribe_description("barbarians").descname)
   assert_equal(_("Empire"), egbase:get_tribe_description("empire").descname)
end

function test_descr:test_name()
   assert_equal("atlanteans", egbase:get_tribe_description("atlanteans").name)
   assert_equal("barbarians", egbase:get_tribe_description("barbarians").name)
   assert_equal("empire", egbase:get_tribe_description("empire").name)
end

function test_descr:test_get_buildings()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(43, #tribe.buildings)
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(52, #tribe.buildings)
   tribe = egbase:get_tribe_description("empire")
   assert_equal(53, #tribe.buildings)

   -- Test if buildings have been casted to their correct types
   for i, building in ipairs(tribe.buildings) do
      if (building.type_name == "productionsite") then
         assert_true(#building.production_programs > 0)
      elseif (building.type_name == "militarysite") then
         assert_true(building.heal_per_second > 0)
         assert_true(building.max_number_of_soldiers > 0)
      elseif (building.type_name == "warehouse") then
         assert_true(building.heal_per_second > 0)
         assert_true(building.max_number_of_soldiers == nil)
      elseif (building.type_name == "trainingsite") then
         assert_true(building.max_attack ~= nil or building.max_defense ~= nil or building.max_evade ~= nil or building.max_health ~= nil)
      end
   end
end

function test_descr:test_get_builder()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_builder", tribe.builder)
end

function test_descr:test_get_carriers()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal("atlanteans_carrier", tribe.carriers[1])
   assert_equal("atlanteans_horse", tribe.carriers[2])
   assert_equal(2, #tribe.carriers)
   -- Deprecated, but shall still work for API compatibility:
   assert_equal("atlanteans_carrier", tribe.carrier)
   assert_equal("atlanteans_horse", tribe.carrier2)
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
   assert_equal(44, #tribe.wares)
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(40, #tribe.wares)
   tribe = egbase:get_tribe_description("empire")
   assert_equal(44, #tribe.wares)
end

function test_descr:test_get_workers()
   local tribe = egbase:get_tribe_description("atlanteans")
   assert_equal(31, #tribe.workers)
   tribe = egbase:get_tribe_description("barbarians")
   assert_equal(33, #tribe.workers)
   tribe = egbase:get_tribe_description("empire")
   assert_equal(34, #tribe.workers)
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
