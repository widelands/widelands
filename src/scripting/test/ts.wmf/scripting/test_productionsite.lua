-- =======================================================================
--                          ProductionSite Testings                         
-- =======================================================================

-- ================
-- Worker creation 
-- ================
productionsite_tests = lunit.TestCase("Productionsite Tests")
function productionsite_tests:setup()
   self.f1 = wl.map.Field(10,10)
   self.f2 = wl.map.Field(12,10)
   self.p = wl.game.Player(1)
   self.p:conquer(f, 6)

   self.inn = self.p:place_building("big_inn", self.f1)
   self.warmill = self.p:place_building("warmill", self.f2)
end
function productionsite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
   pcall(function()
      self.f2.brn.immovable:remove()
   end)
end
function productionsite_tests:test_no_workers_initially()
   assert_equal(0, #self.inn.workers)
   assert_equal(0, #self.warmill.workers)
end
function productionsite_tests:test_valid_workers()
   assert_equal(2, #self.inn.valid_workers)
   assert_equal("innkeeper", self.inn.valid_workers[1])
   assert_equal("innkeeper", self.inn.valid_workers[2])
   assert_equal(2, #self.warmill.valid_workers)
   assert_equal("master-blacksmith", self.warmill.valid_workers[1])
   assert_equal("blacksmith", self.warmill.valid_workers[2])
end
function productionsite_tests:test_warp_workers_one_at_a_time_inn()
   self.inn:warp_workers{"innkeeper"}
   assert_equal(1, #self.inn.workers)
   self.inn:warp_workers{"innkeeper"}
   assert_equal(2, #self.inn.workers)
   assert_equal("innkeeper", self.inn.workers[1])
   assert_equal("innkeeper", self.inn.workers[2])
end
function productionsite_tests:test_warp_workers_one_at_a_time_warmill()
   self.warmill:warp_workers{"master-blacksmith"}
   assert_equal(1, #self.warmill.workers)
   self.warmill:warp_workers{"blacksmith"}
   assert_equal(2, #self.warmill.workers)
   assert_equal("master-blacksmith", self.warmill.workers[1])
   assert_equal("blacksmith", self.warmill.workers[2])
end

function productionsite_tests:test_illegal_name()
   assert_error("illegal name", function()
      self.warmill:warp_workers{"jhsf"}
   end)
end
function productionsite_tests:test_illegal_worker()
  assert_error("illegal worker", function()
      self.warmill:warp_workers{"lumberjack"}
  end)
end
function productionsite_tests:test_no_space()
   self.warmill:warp_workers{"blacksmith"}
  assert_error("no_space", function()
      self.warmill:warp_workers{"blacksmith"}
  end)
end
