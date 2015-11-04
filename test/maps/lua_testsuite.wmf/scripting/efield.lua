-- ==================================================
-- Tests for Fields that are only useful in the Editor
-- ==================================================

function field_resources_tests:test_default_resource_in_editor()
   -- The editor doesn't bother to set the default resources on the map.
   -- The user might want to overwrite them anyways. It therefore sets
   -- the resource of all fields to the first in the world and the amount
   -- to zero.
   assert_equal("coal", self.f.resource)
   assert_equal(0, self.f.resource_amount)
end

function field_resources_tests:test_initial_resource_in_editor()
   -- making sure that (set_) resource_amount sets also starting resource in the editor
   assert_equal("coal", self.f.resource)
   self.f.resource_amount=10
   assert_equal(self.f.initial_resource_amount, self.f.resource_amount)
   assert_equal(10, self.f.initial_resource_amount)
   self.f.resource_amount=20
   assert_equal(self.f.initial_resource_amount, self.f.resource_amount)
   assert_equal(20, self.f.initial_resource_amount)
end
