-- ================================
-- Test functionality in wl.map.Map
-- ================================

test_map = lunit.TestCase("Map functions test")
function test_map:test_instantiation_forbidden()
   assert_error("Cannot instantiate", function()
      wl.map.Map()
   end)
end

function test_map:test_width()
   assert_equal(64, map.width)
end
function test_map:test_height()
   assert_equal(80, map.height)
end

function test_map:test_playerslots_name()
   assert_equal("Barbarian Bouncer", map.player_slots[1].name)
   assert_equal("Epic Empire", map.player_slots[2].name)
   assert_equal("Awesome Atlantean", map.player_slots[3].name)
end

function test_map:test_playerslots_tribes()
   assert_equal("barbarians", map.player_slots[1].tribe_name)
   assert_equal("empire", map.player_slots[2].tribe_name)
   assert_equal("atlanteans", map.player_slots[3].tribe_name)
end

function test_map:test_playerslots_starting_field()
   assert_equal(map:get_field(10,10), map.player_slots[1].starting_field)
   assert_equal(map:get_field(30,10), map.player_slots[2].starting_field)
   assert_equal(map:get_field(50,10), map.player_slots[3].starting_field)
end


