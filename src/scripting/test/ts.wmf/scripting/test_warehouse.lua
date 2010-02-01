-- =========================
-- Warehouses Functionality 
-- =========================
warehouse_tests = lunit.TestCase("warehouse tests")
function warehouse_tests:setup()
   self.f = wl.map.Field(10,10)
   self.p = wl.game.Player(1)
   self.w = self.p:place_building("headquarters", self.f)
end
function warehouse_tests:teardown()
   pcall(self.w.remove, self.w)
   -- TODO: these tests do not clean up after itself: the player remains owner
   -- TODO: of the field
end

function warehouse_tests:test_upcasting_from_immovable_to_building()
   i = self.f.immovable
   assert_equal(i, self.w)
   assert_function(i.set_wares) -- set_wares is non nil
end

function warehouse_tests:test_set_ware_illegal_ware()
   function ill()
      self.w:set_wares("sdjsgfhg", 100)
   end
   assert_error("Illegal ware should throw an error!", ill)
end
function warehouse_tests:test_set_get_wares_string_arg()
   assert_equal(0, self.w:get_wares("trunk"))
   self.w:set_wares("trunk", 190)
   assert_equal(190, self.w:get_wares("trunk"))
end
function warehouse_tests:test_set_get_wares_table_arg()
   k = self.w:get_wares{"trunk", "raw_stone"}
   assert_equal(0, k.trunk)
   assert_equal(0, k.raw_stone)
   self.w:set_wares{trunk=190, raw_stone=170}
   k = self.w:get_wares{"trunk", "raw_stone"}
   assert_equal(190, k.trunk)
   assert_equal(170, k.raw_stone)
end
function warehouse_tests:test_set_get_wares_set_is_not_increase()
   k = self.w:get_wares{"trunk", "raw_stone"}
   k.trunk = 20
   k.raw_stone = 40
   self.w:set_wares(k)
   k = self.w:get_wares{"trunk", "raw_stone"}
   assert_equal(20, k.trunk)
   assert_equal(40, k.raw_stone)

   k.trunk = 10
   k.raw_stone = 20
   self.w:set_wares(k)
   k = self.w:get_wares{"trunk", "raw_stone"}
   assert_equal(10, k.trunk)
   assert_equal(20, k.raw_stone)
end

function warehouse_tests:test_set_worker_illegal_worker()
   function ill()
      self.w:set_workers("sdjsgfhg", 100)
   end
   assert_error("Illegal worker should throw an error!", ill)
end
function warehouse_tests:test_set_get_workers_string_arg()
   assert_equal(0, self.w:get_workers("builder"))
   self.w:set_workers("builder", 190)
   assert_equal(190, self.w:get_workers("builder"))
end
function warehouse_tests:test_set_get_workers_table_arg()
   k = self.w:get_workers{"builder", "lumberjack"}
   assert_equal(0, k.builder)
   assert_equal(0, k.lumberjack)
   self.w:set_workers{builder=190, lumberjack=170}
   k = self.w:get_workers{"builder", "lumberjack"}
   assert_equal(190, k.builder)
   assert_equal(170, k.lumberjack)
end
function warehouse_tests:test_set_get_workers_set_is_not_increase()
   k = self.w:get_workers{"builder", "lumberjack"}
   k.builder = 20
   k.lumberjack = 40
   self.w:set_workers(k)
   k = self.w:get_workers{"builder", "lumberjack"}
   assert_equal(20, k.builder)
   assert_equal(40, k.lumberjack)

   k.builder = 10
   k.lumberjack = 20
   self.w:set_workers(k)
   k = self.w:get_workers{"builder", "lumberjack"}
   assert_equal(10, k.builder)
   assert_equal(20, k.lumberjack)
end

