
function plr_access:test_plr_immovable_owner()
   assert_equal(player1, self.f.owner)
   assert_not_nil(self.f.owner.message_box)
end

function plr_access:test_field_owner()
   assert_equal(player1, self.field.owner)
   assert_not_nil(self.field.owner.message_box)
end

