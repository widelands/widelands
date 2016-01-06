-- ==================================================
-- Tests for Fields that are only useful in the Editor
-- ==================================================

function field_resources_tests:test_default_resource_in_editor()
   -- The editor doesn't bother to set the default resources on the map.
   -- The user might want to overwrite them anyways. It therefore sets
   -- the resource of all fields to 'none' and the amount to zero.
   assert_equal("none", self.f.resource)
   assert_equal(0, self.f.resource_amount)
end

function field_resources_tests:test_initial_resource_in_editor()
   -- making sure that (set_) resource_amount sets also initial resource in the editor
   -- the resource has to be set to something different than 'none', as none has no amount
   self.f.resource = "coal"
   self.f.resource_amount=10
   assert_equal(self.f.initial_resource_amount, self.f.resource_amount)
   assert_equal(10, self.f.initial_resource_amount)
   self.f.resource_amount=20
   assert_equal(self.f.initial_resource_amount, self.f.resource_amount)
   assert_equal(20, self.f.initial_resource_amount)
end
