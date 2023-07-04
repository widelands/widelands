-- =========================
-- Constructionsite Functionality
-- =========================
constructionsite_tests = lunit.TestCase("constructionsite tests")
function constructionsite_tests:setup()
   self.f1 = map:get_field(8,10)
   self.f2 = map:get_field(12,10)
   player1:conquer(self.f1, 3)
   player1:conquer(self.f2, 3)

   player1:place_building("barbarians_lumberjacks_hut", self.f1, true)
   player1:place_building("barbarians_fortress", self.f2, true)

   self.l = self.f1.immovable
   self.f = self.f2.immovable
end

function constructionsite_tests:test_upcasting_from_immovable_to_building()
   local i = self.f1.immovable
   assert_equal(i, self.l)
   assert_not_equal(nil, i.building)
end

function constructionsite_tests:teardown()
   pcall(function() self.f1.brn.immovable:remove() end)
   pcall(function() self.f2.brn.immovable:remove() end)
end

function constructionsite_tests:test_name()
   assert_equal("constructionsite", self.l.descr.name)
   assert_equal("constructionsite", self.f.descr.name)
end
function constructionsite_tests:test_type()
   assert_equal("constructionsite", self.l.descr.type_name)
   assert_equal("constructionsite", self.f.descr.type_name)
end

function constructionsite_tests:test_size()
   local building = egbase:get_building_description(self.l.building)
   assert_equal("small", egbase:get_building_description(self.l.building).size)
   assert_equal("big", egbase:get_building_description(self.f.building).size)
end

function constructionsite_tests:test_building()
   assert_equal("barbarians_lumberjacks_hut", self.l.building)
   assert_equal("barbarians_fortress", self.f.building)
end
