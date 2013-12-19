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


function test_descr:test_global()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").global)
  -- TODO find a building that is global
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
end
