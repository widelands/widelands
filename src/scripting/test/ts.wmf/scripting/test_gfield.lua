-- ==================================================
-- Tests for Fields that are only useful in the Game 
-- ==================================================

function field_resources_tests:test_default_resource_in_game()
   -- The game sets the default resources from the world in the map
   -- when it starts up.
   assert_equal("water", self.f.resource)
   assert_equal(5, self.f.resource_amount)
end




