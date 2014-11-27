-- ===================
-- Tests for the game 
-- ===================

test_game = lunit.TestCase("Game functions test")

function test_game:test_no_editor_module()
   assert_equal(nil, wl.editor)
end
function test_game:test_no_editor_class()
   assert_equal(nil, wl.Editor)
end

-- =========
-- roads related
-- =========
function flag_tests:roads_test()
   self.wf = map:get_field(4,14)
   self.w = player1:place_building("warehouse", self.wf)
   self.f1=self.w.flag
   self.r = player1:place_road(self.f1, "br", "br", "br")
   self.f2 = self.f1.fields[1].brn.brn.brn.immovable
   assert_not_nil(self.f2.roads.tl)
   assert_not_nil(self.f2.roads.tl.start_flag)
   assert_not_nil(self.f2.has_warehouse)
   assert_not_nil(self.f1.building)
end
