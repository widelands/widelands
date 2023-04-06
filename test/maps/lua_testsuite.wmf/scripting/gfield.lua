-- ==================================================
-- Tests for Fields that are only useful in the Game
-- ==================================================

function field_resources_tests:test_default_resource_in_game()
   -- The game sets the default resources from the world in the map
   -- when it starts up.
   assert_equal("resource_water", self.f.resource)
   assert_equal(5, self.f.resource_amount)
end

function field_resources_tests:test_initial_resources_in_game()
   -- Changing a resource in game should not change the
   -- initial_resources.
   local f = map:get_field(61,40)
   assert_equal("resource_water", f.resource)
   assert_equal(5, f.initial_resource_amount)
   assert_equal(5, f.resource_amount)
   f.resource_amount = 1
   assert_equal(5, f.initial_resource_amount)
   assert_equal(1, f.resource_amount)
end
