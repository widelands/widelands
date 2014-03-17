-- =========================
-- Warehouses Functionality 
-- =========================
warehouse_tests = lunit.TestCase("warehouse tests")
function warehouse_tests:setup()
   self.f = map:get_field(10,10)
   self.w = player1:place_building("warehouse", self.f)
end
function warehouse_tests:teardown()
   pcall(function() self.f.brn.immovable:remove() end)
end

function warehouse_tests:test_name()
   assert_equal("warehouse", self.w.name)
end
function warehouse_tests:test_type()
   assert_equal("warehouse", self.w.type)
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
function warehouse_tests:test_set_ware_no_argument()
   assert_error("Need an argument!", function()
      self.w:set_wares()
   end)
end
function warehouse_tests:test_set_ware_nil_as_argument()
   assert_error("nil as first argument makes no sense!", function()
      self.w:set_wares(iron)
   end)
end
function warehouse_tests:test_set_get_wares_string_arg()
   assert_equal(0, self.w:get_wares("log"))
   self.w:set_wares("log", 190)
   assert_equal(190, self.w:get_wares("log"))
   assert_equal(0, self.w:get_wares("raw_stone"))
end
function warehouse_tests:test_set_get_wares_all()
   self.w:set_wares{log=190, raw_stone=170}
   local rv = self.w:get_wares("all")
   assert_equal(190, rv.log)
   assert_equal(170, rv.raw_stone)
   assert_equal(0, rv.coal)
end
function warehouse_tests:test_set_get_wares_table_arg()
   k = self.w:get_wares{"log", "raw_stone"}
   assert_equal(0, k.log)
   assert_equal(0, k.raw_stone)
   assert_equal(nil, k.coal)
   self.w:set_wares{log=190, raw_stone=170}
   k = self.w:get_wares{"log", "raw_stone"}
   assert_equal(190, k.log)
   assert_equal(170, k.raw_stone)
   assert_equal(nil, k.coal)
end
function warehouse_tests:test_set_get_wares_set_is_not_increase()
   k = self.w:get_wares{"log", "raw_stone"}
   k.log = 20
   k.raw_stone = 40
   self.w:set_wares(k)
   k = self.w:get_wares{"log", "raw_stone"}
   assert_equal(20, k.log)
   assert_equal(40, k.raw_stone)

   k.log = 10
   k.raw_stone = 20
   self.w:set_wares(k)
   k = self.w:get_wares{"log", "raw_stone"}
   assert_equal(10, k.log)
   assert_equal(20, k.raw_stone)
end
function warehouse_tests:test_get_wares_non_existant_name()
   assert_error("non existent ware", function() 
      self.w:get_wares("balloon")
   end)
   assert_error("non existent ware", function() 
      self.w:get_wares{"meat", "balloon"}
   end)
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


-- =========
-- Soldiers 
-- =========
function _cnt(a)
   local rv = 0
   for sd, c in pairs(a) do rv = rv + c end
   return rv
end

function warehouse_tests:test_get_soldiers_empty_at_start()
   assert_equal(0, _cnt(self.w:get_soldiers("all")))
end
function warehouse_tests:test_set_soldiers()
   self.w:set_soldiers({0,0,0,0}, 100)
   assert_equal(100, _cnt(self.w:get_soldiers("all")))
end
function warehouse_tests:test_reduce_soldiers_number()
   self.w:set_soldiers{
      [{0,0,0,0}] = 10,
      [{1,1,0,1}] = 20,
   }
   assert_equal(10, self.w:get_soldiers({0,0,0,0}))
   assert_equal(20, self.w:get_soldiers({1,1,0,1}))
   assert_equal(30, _cnt(self.w:get_soldiers("all")))
   self.w:set_soldiers{
      [{0,0,0,0}] = 1,
      [{1,1,0,1}] = 2,
   }
   assert_equal(1, self.w:get_soldiers({0,0,0,0}))
   assert_equal(2, self.w:get_soldiers({1,1,0,1}))
   assert_equal(3, _cnt(self.w:get_soldiers("all")))
end
function warehouse_tests:test_set_soldiers_by_list()
   self.w:set_soldiers{
      [{0,0,0,0}] = 1,
      [{1,1,0,1}] = 2,
   }
   assert_equal(3, _cnt(self.w:get_soldiers("all")))
   assert_equal(1, self.w:get_soldiers({0,0,0,0}))
   assert_equal(2, self.w:get_soldiers({1,1,0,1}))
   assert_equal(0, self.w:get_soldiers({2,1,0,1}))
end
function warehouse_tests:test_set_soldiers_hp_too_high()
   assert_error("hp too high", function()
      self.w:set_soldiers({10,0,0,0}, 1)
   end)
end
function warehouse_tests:test_set_soldiers_attack_too_high()
   assert_error("attack too high", function()
      self.w:set_soldiers({0,10,0,0}, 1)
   end)
end
function warehouse_tests:test_set_soldiers_defense_too_high()
   assert_error("defense too high", function()
      self.w:set_soldiers({0,0,10,0}, 1)
   end)
end
function warehouse_tests:test_set_soldiers_evade_too_high()
   assert_error("evade too high", function()
      self.w:set_soldiers({0,0,0,10}, 1)
   end)
end


