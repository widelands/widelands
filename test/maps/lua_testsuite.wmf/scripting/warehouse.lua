-- =========================
-- Warehouses Functionality
-- =========================
warehouse_tests = lunit.TestCase("warehouse tests")
function warehouse_tests:setup()
   self.f = map:get_field(10,10)
   self.w = player1:place_building("barbarians_warehouse", self.f)
end
function warehouse_tests:teardown()
   pcall(function() self.f.brn.immovable:remove() end)
end

function warehouse_tests:test_name()
   assert_equal("barbarians_warehouse", self.w.descr.name)
end
function warehouse_tests:test_type()
   assert_equal("warehouse", self.w.descr.type_name)
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
   assert_equal(0, self.w:get_wares("granite"))
end
function warehouse_tests:test_set_get_wares_all()
   self.w:set_wares{log=190, granite=170}
   local rv = self.w:get_wares("all")
   assert_equal(190, rv.log)
   assert_equal(170, rv.granite)
   assert_equal(0, rv.coal)
end
function warehouse_tests:test_set_get_wares_table_arg()
   k = self.w:get_wares{"log", "granite"}
   assert_equal(0, k.log)
   assert_equal(0, k.granite)
   assert_equal(nil, k.coal)
   self.w:set_wares{log=190, granite=170}
   k = self.w:get_wares{"log", "granite"}
   assert_equal(190, k.log)
   assert_equal(170, k.granite)
   assert_equal(nil, k.coal)
end
function warehouse_tests:test_set_get_wares_set_is_not_increase()
   k = self.w:get_wares{"log", "granite"}
   k.log = 20
   k.granite = 40
   self.w:set_wares(k)
   k = self.w:get_wares{"log", "granite"}
   assert_equal(20, k.log)
   assert_equal(40, k.granite)

   k.log = 10
   k.granite = 20
   self.w:set_wares(k)
   k = self.w:get_wares{"log", "granite"}
   assert_equal(10, k.log)
   assert_equal(20, k.granite)
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
   assert_equal(0, self.w:get_workers("barbarians_builder"))
   self.w:set_workers("barbarians_builder", 190)
   assert_equal(190, self.w:get_workers("barbarians_builder"))
end
function warehouse_tests:test_set_get_workers_table_arg()
   k = self.w:get_workers{"barbarians_builder", "barbarians_lumberjack"}
   assert_equal(0, k.barbarians_builder)
   assert_equal(0, k.barbarians_lumberjack)
   self.w:set_workers{barbarians_builder=190, barbarians_lumberjack=170}
   k = self.w:get_workers{"barbarians_builder", "barbarians_lumberjack"}
   assert_equal(190, k.barbarians_builder)
   assert_equal(170, k.barbarians_lumberjack)
end
function warehouse_tests:test_set_get_workers_set_is_not_increase()
   k = self.w:get_workers{"barbarians_builder", "barbarians_lumberjack"}
   k.barbarians_builder = 20
   k.barbarians_lumberjack = 40
   self.w:set_workers(k)
   k = self.w:get_workers{"barbarians_builder", "barbarians_lumberjack"}
   assert_equal(20, k.barbarians_builder)
   assert_equal(40, k.barbarians_lumberjack)

   k.barbarians_builder = 10
   k.barbarians_lumberjack = 20
   self.w:set_workers(k)
   k = self.w:get_workers{"barbarians_builder", "barbarians_lumberjack"}
   assert_equal(10, k.barbarians_builder)
   assert_equal(20, k.barbarians_lumberjack)
end

-- ========
-- Policies
-- ========
function warehouse_tests:test_get_ware_policy()
   assert_equal(self.w:get_ware_policies("log"), "normal")
   k = self.w:get_ware_policies{"log", "granite"}
   assert_equal("normal", k.log)
   assert_equal("normal", k.granite)
   assert_equal(nil, k.coal)
   k = self.w:get_ware_policies("all")
   assert_equal("normal", k.log)
   assert_equal("normal", k.granite)
   assert_equal("normal", k.coal)
   assert_equal(nil, k.barbarians_lumberjack)
end

function warehouse_tests:test_get_worker_policy()
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "normal")
   k = self.w:get_worker_policies{"barbarians_builder", "barbarians_lumberjack"}
   assert_equal("normal", k.barbarians_builder)
   assert_equal("normal", k.barbarians_lumberjack)
   assert_equal(nil, k.coal)
   k = self.w:get_worker_policies("all")
   assert_equal("normal", k.barbarians_builder)
   assert_equal("normal", k.barbarians_lumberjack)
   assert_equal(nil, k.coal)
end

function warehouse_tests:test_get_non_existing_policy()
   assert_error("ware policy for non existing ware", function()
      self.w:get_ware_policy("plastic")
   end)
   assert_error("worker policy for non existing worker", function()
      self.w:get_worker_policy("tree_climber")
   end)
end

function warehouse_tests:test_set_ware_policy()
   -- Make sure it is the normal policy until now
   assert_equal(self.w:get_ware_policies("log"), "normal")
   assert_equal(self.w:get_ware_policies("granite"), "normal")
   -- Set new policy for wares
   self.w:set_ware_policies("log", "prefer")
   assert_equal(self.w:get_ware_policies("log"), "prefer")
   assert_equal(self.w:get_ware_policies("granite"), "normal")
   self.w:set_ware_policies("log", "dontstock")
   assert_equal(self.w:get_ware_policies("log"), "dontstock")
   assert_equal(self.w:get_ware_policies("granite"), "normal")
   self.w:set_ware_policies("log", "remove")
   assert_equal(self.w:get_ware_policies("log"), "remove")
   assert_equal(self.w:get_ware_policies("granite"), "normal")
   self.w:set_ware_policies("granite", "dontstock")
   assert_equal(self.w:get_ware_policies("log"), "remove")
   assert_equal(self.w:get_ware_policies("granite"), "dontstock")
   self.w:set_ware_policies({"log", "granite"}, "prefer")
   assert_equal(self.w:get_ware_policies("log"), "prefer")
   assert_equal(self.w:get_ware_policies("granite"), "prefer")
   self.w:set_ware_policies("log", "normal")
   self.w:set_ware_policies("granite", "normal")
   assert_equal(self.w:get_ware_policies("log"), "normal")
   assert_equal(self.w:get_ware_policies("granite"), "normal")
   self.w:set_ware_policies("all", "prefer")
   assert_equal(self.w:get_ware_policies("log"), "prefer")
   assert_equal(self.w:get_ware_policies("granite"), "prefer")
   assert_equal(self.w:get_ware_policies("meat"), "prefer")
   assert_equal(self.w:get_ware_policies("coal"), "prefer")
end

function warehouse_tests:test_set_worker_policy()
   -- Make sure it is the normal policy until now
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "normal")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "normal")
   -- Set new policy for workers
   self.w:set_worker_policies("barbarians_builder", "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "normal")
   self.w:set_worker_policies("barbarians_builder", "dontstock")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "dontstock")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "normal")
   self.w:set_worker_policies("barbarians_builder", "remove")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "remove")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "normal")
   self.w:set_worker_policies("barbarians_lumberjack", "dontstock")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "remove")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "dontstock")
   self.w:set_worker_policies({"barbarians_lumberjack", "barbarians_builder"}, "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "prefer")
   self.w:set_worker_policies("barbarians_builder", "normal")
   self.w:set_worker_policies("barbarians_lumberjack", "normal")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "normal")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "normal")
   self.w:set_worker_policies("all", "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_builder"), "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_lumberjack"), "prefer")
   assert_equal(self.w:get_worker_policies("barbarians_gardener"), "prefer")
end

function warehouse_tests:test_set_non_existing_policy()
   assert_error("ware policy for non existing ware", function()
      self.w:set_ware_policies("plastic", "prefer")
   end)
   assert_error("invalid policy for ware", function()
      self.w:set_ware_policies("log", "burn")
   end)
   assert_error("worker policy for non existing worker", function()
      self.w:set_worker_policies("tree_climber", "prefer")
   end)
   assert_error("invalid policy for worker", function()
      self.w:set_worker_policies("barbarians_builder", "burn")
   end)
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
function warehouse_tests:test_set_soldiers_health_too_high()
   assert_error("health too high", function()
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
