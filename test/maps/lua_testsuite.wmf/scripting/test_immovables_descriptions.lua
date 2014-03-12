test_descr = lunit.TestCase("Immovable descriptions test")
function test_descr:test_instantiation_forbidden()
   assert_error("Cannot instantiate", function()
      wl.map.BuildingDescription()
   end)
end


function test_descr:test_building_descr()
   assert_error("Wrong tribe", function() egbase:get_building_description("XXX","sentry") end)
   assert_error("Wrong building", function() egbase:get_building_description("barbarians","XXX") end)
   assert_error("Wrong number of parameters: 1", function() egbase:get_building_description("XXX") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_building_description("XXX","YYY","ZZZ") end)
end


function test_descr:test_buildable()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").buildable)
   assert_equal(true, egbase:get_building_description("barbarians","sentry").buildable)
end


function test_descr:test_destructible()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").destructible)
   assert_equal(true, egbase:get_building_description("barbarians","sentry").destructible)
end


function test_descr:test_enhanced()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").enhanced)
   assert_equal(true, egbase:get_building_description("barbarians","axefactory").enhanced)
end


function test_descr:test_ismine()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").ismine)
   assert_equal(true, egbase:get_building_description("barbarians","oremine").ismine)
end


function test_descr:test_isport()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").isport)
   assert_equal(true, egbase:get_building_description("barbarians","port").isport)
end


function test_descr:test_size()
   assert_equal(1, egbase:get_building_description("barbarians","lumberjacks_hut").size)
   assert_equal(2, egbase:get_building_description("barbarians","fernery").size)
   assert_equal(3, egbase:get_building_description("barbarians","fortress").size)
   assert_equal(1, egbase:get_building_description("barbarians","coalmine").size)
end


function test_descr:test_build_cost()
   assert_equal(2, egbase:get_building_description("barbarians","sentry").build_cost)
   assert_equal(20, egbase:get_building_description("barbarians","fortress").build_cost)
   assert_equal(0, egbase:get_building_description("barbarians","citadel").build_cost)
end


function test_descr:test_returned_wares()
   assert_equal(1, egbase:get_building_description("barbarians","sentry").returned_wares)
   assert_equal(9, egbase:get_building_description("barbarians","fortress").returned_wares)
   assert_equal(0, egbase:get_building_description("barbarians","citadel").returned_wares)
end


function test_descr:test_enhancement_cost()
   assert_equal(0, egbase:get_building_description("barbarians","sentry").enhancement_cost)
   assert_equal(20, egbase:get_building_description("barbarians","citadel").enhancement_cost)
end


function test_descr:test_returned_wares_enhanced()
   assert_equal(0, egbase:get_building_description("barbarians","sentry").returned_wares_enhanced)
   assert_equal(0, egbase:get_building_description("barbarians","fortress").returned_wares_enhanced)
   assert_equal(10, egbase:get_building_description("barbarians","citadel").returned_wares_enhanced)
end


