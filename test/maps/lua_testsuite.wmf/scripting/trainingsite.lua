-- =======================================================================
--                          TrainingSite Testings
-- =======================================================================

-- ================
-- Soldier creation
-- ================
function _cnt(a)
   local rv = 0
   for sd, c in pairs(a) do rv = rv + c end
   return rv
end

trainingsite_tests = lunit.TestCase("MilitarySite Tests")
function trainingsite_tests:setup()
   self.f1 = map:get_field(10,10)

   self.camp = player1:place_building("barbarians_trainingcamp", self.f1)
end
function trainingsite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
end

function trainingsite_tests:test_name()
   assert_equal("barbarians_trainingcamp", self.camp.descr.name)
end
function trainingsite_tests:test_type()
   assert_equal("trainingsite", self.camp.descr.type_name)
end

-- ================
-- Worker creation
-- ================
function trainingsite_tests:test_no_workers_initially()
   assert_equal(0, _cnt(self.camp:get_workers("all")))
end
function trainingsite_tests:test_valid_workers()
   assert_equal(1, _cnt(self.camp.valid_workers))
   assert_equal(1, self.camp.valid_workers.barbarians_trainer)
end
function trainingsite_tests:test_set_workers()
   self.camp:set_workers("barbarians_trainer", 1)
   assert_equal(1, _cnt(self.camp:get_workers("all")))
   assert_equal(1, self.camp:get_workers("barbarians_trainer"))
   self.camp:set_workers{barbarians_trainer=1}
   assert_equal(1, _cnt(self.camp:get_workers("all")))
   assert_equal(1, self.camp:get_workers("barbarians_trainer"))
   local rv = self.camp:get_workers{"barbarians_trainer", "barbarians_carrier"}
   assert_equal(1, rv.barbarians_trainer)
   assert_equal(0, rv.barbarians_carrier)
   assert_equal(nil, rv.barbarians_blacksmith)
end
function trainingsite_tests:test_get_workers_all()
   self.camp:set_workers{barbarians_trainer=1}
   local rv = self.camp:get_workers("all")
   assert_equal(1, rv.barbarians_trainer)
   assert_equal(nil, rv.barbarians_blacksmith_master)
end

function trainingsite_tests:test_illegal_name()
   assert_error("illegal name", function()
      self.camp:set_workers("jhsf",1)
   end)
end
function trainingsite_tests:test_illegal_worker()
  assert_error("illegal worker", function()
      self.camp:set_workers("barbarians_lumberjack", 1)
  end)
end
function trainingsite_tests:test_no_space()
  assert_error("no_space", function()
      self.camp:set_workers{barbarians_trainer=2}
  end)
end

-- ==============
-- Ware creation
-- ==============
function trainingsite_tests:test_houses_empty_at_creation()
   for wname, count in pairs(self.camp.valid_inputs) do
      assert_equal(0, self.camp:get_inputs(wname))
   end
end
function trainingsite_tests:test_valid_inputs()
   assert_equal(10, self.camp.valid_inputs.barbarians_bread)
   assert_equal(6, self.camp.valid_inputs.fish)
   assert_equal(6, self.camp.valid_inputs.meat)
   assert_equal(2, self.camp.valid_inputs.ax_sharp)
   assert_equal(2, self.camp.valid_inputs.ax_broad)
   assert_equal(2, self.camp.valid_inputs.ax_bronze)
   assert_equal(2, self.camp.valid_inputs.ax_battle)
   assert_equal(2, self.camp.valid_inputs.ax_warriors)
   assert_equal(2, self.camp.valid_inputs.helmet)
   assert_equal(2, self.camp.valid_inputs.helmet_mask)
   assert_equal(2, self.camp.valid_inputs.helmet_warhelm)
   assert_equal(nil, self.camp.valid_inputs.water)
end
function trainingsite_tests:test_valid_inputs_correct_length()
   c = {}
   for n,count in pairs(self.camp.valid_inputs) do c[#c+1] = n end
   assert_equal(11, #c)
end
function trainingsite_tests:test_set_inputs_string_arg()
   self.camp:set_inputs("fish", 3)
   assert_equal(3, self.camp:get_inputs("fish"))
end
function trainingsite_tests:test_set_inputs_array_arg()
   self.camp:set_inputs{fish=3, meat=2}
   assert_equal(3, self.camp:get_inputs("fish"))
   assert_equal(2, self.camp:get_inputs("meat"))
end
function trainingsite_tests:test_set_inputs_illegal_name()
   assert_error("illegal ware", function()
      self.camp:set_inputs{meat = 2, log=1}
   end)
   assert_error("illegal ware", function()
      self.camp:set_inputs("log",1)
   end)
end
function trainingsite_tests:test_set_inputs_nonexistant_name()
   assert_error("illegal ware", function()
      self.camp:set_inputs{meat = 2, balloon=1}
   end)
   assert_error("illegal ware", function()
      self.camp:set_inputs("balloon",1)
   end)
end
function trainingsite_tests:test_set_inputs_negative_count()
   assert_error("negative counts", function()
      self.camp:set_inputs("meat", -1)
   end)
end
function trainingsite_tests:test_set_inputs_illegal_count()
   assert_error("too big count", function()
      self.camp:set_inputs("meat", 10)
   end)
end
function trainingsite_tests:test_get_inputs_array_arg()
   self.camp:set_inputs{fish=3, meat=2}
   rv = self.camp:get_inputs{"fish", "meat"}
   assert_equal(3, rv.fish)
   assert_equal(2, rv.meat)
   assert_equal(nil, rv.helmet)
end
function trainingsite_tests:test_get_inputs_all_arg()
   self.camp:set_inputs{fish=3, meat=2}
   rv = self.camp:get_inputs("all")
   assert_equal(0, rv.barbarians_bread)
   assert_equal(0, rv.helmet)
   assert_equal(2, rv.meat)
   assert_equal(3, rv.fish)
   assert_equal(nil, rv.log)
end
function trainingsite_tests:test_get_inputs_string_arg()
   self.camp:set_inputs{fish=3, meat=2}
   assert_equal(2, self.camp:get_inputs("meat"))
   assert_equal(3, self.camp:get_inputs("fish"))
   assert_equal(0, self.camp:get_inputs("beer"))
   assert_equal(0, self.camp:get_inputs("beer_strong"))
   assert_equal(0, self.camp:get_inputs("log"))
end
function trainingsite_tests:test_get_inputs_non_storable_wares()
   self.camp:set_inputs{fish=3, meat=2}
   local rv = self.camp:get_inputs{"meat", "log", "fish"}
   assert_equal(0, rv.log)
   assert_equal(2, rv.meat)
   assert_equal(3, rv.fish)
   assert_equal(nil, rv.beer_strong)
end
function trainingsite_tests:test_get_inputs_non_existant_name()
   assert_error("non existent ware", function()
      self.camp:get_inputs("balloon")
   end)
   assert_error("non existent ware", function()
      self.camp:get_inputs{"meat", "balloon"}
   end)
end

-- =================
-- Soldier creation
-- =================
function trainingsite_tests:test_no_soldiers_initially()
   assert_equal(0, _cnt(self.camp:get_soldiers("all")))
end
function trainingsite_tests:test_max_soldiers()
   assert_equal(12, self.camp.max_soldiers);
end
function trainingsite_tests:test_set_soldiers_single_arg()
   self.camp:set_soldiers({0,0,0,0}, 2)
   assert_equal(2, self.camp:get_soldiers({0,0,0,0}))
end
function trainingsite_tests:test_set_soldiers_multi_arg()
   self.camp:set_soldiers{
      [{0,0,0,0}] = 2,
      [{1,1,0,1}] = 3
   }
   assert_equal(5, _cnt(self.camp:get_soldiers("all")))
   assert_equal(2, self.camp:get_soldiers({0,0,0,0}))
   assert_equal(3, self.camp:get_soldiers({1,1,0,1}))
end
function trainingsite_tests:test_set_soldiers_add_and_remove()
   self.camp:set_soldiers{[{0,0,0,0}] = 3}
   assert_equal(3, _cnt(self.camp:get_soldiers("all")))
   assert_equal(3, self.camp:get_soldiers({0,0,0,0}))
   self.camp:set_soldiers{[{3,0,0,1}] = 1, [{0,0,0,0}] = 1}
   assert_equal(2, _cnt(self.camp:get_soldiers("all")))
   assert_equal(1, self.camp:get_soldiers({0,0,0,0}))
   assert_equal(1, self.camp:get_soldiers({3,0,0,1}))
   self.camp:set_soldiers({3,2,0,1}, 1)
   assert_equal(1, _cnt(self.camp:get_soldiers("all")))
   assert_equal(0, self.camp:get_soldiers({0,0,0,0}))
   assert_equal(0, self.camp:get_soldiers({3,0,0,1}))
   assert_equal(1, self.camp:get_soldiers({3,2,0,1}))
end

function trainingsite_tests:test_set_soldiers_all_at_once()
   self.camp:set_soldiers{[{0,0,0,0}] = self.camp.max_soldiers}
   assert_equal(self.camp.max_soldiers,
      _cnt(self.camp:get_soldiers("all")))
   assert_equal(self.camp.max_soldiers,
      self.camp:get_soldiers({0,0,0,0}))
end
function trainingsite_tests:test_illegal_soldier()
   assert_error("illegal level", function()
      self.camp:set_soldiers{[{10,0,0,0}] = 1}
   end)
end
function trainingsite_tests:test_no_space()
   assert_error("no_space", function()
      self.camp:set_soldiers{[{0,0,0,0}] = 13}
  end)
   assert_equal(12, _cnt(self.camp:get_soldiers("all")))
end
