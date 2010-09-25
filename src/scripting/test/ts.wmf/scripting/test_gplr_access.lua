
function plr_access:test_plr_immovable_owner()
   assert_equal(player1, self.f.owner)
   assert_not_nil(self.f.owner.message_box)
end

function plr_access:test_field_owners()
   assert_equal(1, #self.field.owners)
   assert_equal(player1, self.field.owners[1])
   assert_not_nil(self.field.owners[1].message_box)
end

