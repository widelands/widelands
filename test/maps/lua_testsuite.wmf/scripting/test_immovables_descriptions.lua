test_descr = lunit.TestCase("Immovable descriptions test")
function test_descr:test_instantiation_forbidden()
   assert_error("Cannot instantiate", function()
      wl.map.BuildingDescription()
   end)
end

function test_descr:test_buildable()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").buildable)
   assert_equal(true, egbase:get_building_description("barbarians","sentry").buildable)
end

function test_descr:test_building_descr()
   assert_error("Wrong tribe", function() egbase:get_building_description("XXX","sentry") end)
   assert_error("Wrong building", function() egbase:get_building_description("barbarians","XXX") end)
   assert_error("Wrong number of parameters: 1", function() egbase:get_building_description("XXX") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_building_description("XXX","YYY","ZZZ") end)
end
