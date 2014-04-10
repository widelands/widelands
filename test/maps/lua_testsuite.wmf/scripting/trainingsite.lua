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

   self.camp = player1:place_building("trainingcamp", self.f1)
end
function trainingsite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
end

function trainingsite_tests:test_name()
   assert_equal("trainingcamp", self.camp.name)
end
function trainingsite_tests:test_type()
   assert_equal("trainingsite", self.camp.type)
end

-- ================
-- Worker creation 
-- ================
function trainingsite_tests:test_no_workers_initially()
   assert_equal(0, _cnt(self.camp:get_workers("all")))
end
function trainingsite_tests:test_valid_workers()
   assert_equal(1, _cnt(self.camp.valid_workers))
   assert_equal(1, self.camp.valid_workers.trainer)
end
function trainingsite_tests:test_set_workers()
   self.camp:set_workers("trainer", 1)
   assert_equal(1, _cnt(self.camp:get_workers("all")))
   assert_equal(1, self.camp:get_workers("trainer"))
   self.camp:set_workers{trainer=1}
   assert_equal(1, _cnt(self.camp:get_workers("all")))
   assert_equal(1, self.camp:get_workers("trainer"))
   local rv = self.camp:get_workers{"trainer", "carrier"}
   assert_equal(1, rv.trainer)
   assert_equal(0, rv.carrier)
   assert_equal(nil, rv.blacksmith)
end
function trainingsite_tests:test_get_workers_all()
   self.camp:set_workers{trainer=1}
   local rv = self.camp:get_workers("all")
   assert_equal(1, rv.trainer)
   assert_equal(nil, rv["master-blacksmith"])
end

function trainingsite_tests:test_illegal_name()
   assert_error("illegal name", function()
      self.camp:set_workers("jhsf",1)
   end)
end
function trainingsite_tests:test_illegal_worker()
  assert_error("illegal worker", function()
      self.camp:set_workers("lumberjack", 1)
  end)
end
function trainingsite_tests:test_no_space()
  assert_error("no_space", function()
      self.camp:set_workers{trainer=2}
  end)
end

-- ==============
-- Ware creation
-- ==============
function trainingsite_tests:test_houses_empty_at_creation()
   for wname, count in pairs(self.camp.valid_wares) do
      assert_equal(0, self.camp:get_wares(wname))
   end
end
function trainingsite_tests:test_valid_wares()
   assert_equal(10, self.camp.valid_wares.pittabread)
   assert_equal(6, self.camp.valid_wares.fish)
   assert_equal(6, self.camp.valid_wares.meat)
   assert_equal(2, self.camp.valid_wares.sharpax)
   assert_equal(2, self.camp.valid_wares.broadax)
   assert_equal(2, self.camp.valid_wares.bronzeax)
   assert_equal(2, self.camp.valid_wares.battleax)
   assert_equal(2, self.camp.valid_wares.warriorsax)
   assert_equal(2, self.camp.valid_wares.helm)
   assert_equal(2, self.camp.valid_wares.mask)
   assert_equal(2, self.camp.valid_wares.warhelm)
   assert_equal(nil, self.camp.valid_wares.water)
end
function trainingsite_tests:test_valid_wares_correct_length()
   c = {}
   for n,count in pairs(self.camp.valid_wares) do c[#c+1] = n end
   assert_equal(11, #c)
end
function trainingsite_tests:test_set_wares_string_arg()
   self.camp:set_wares("fish", 3)
   assert_equal(3, self.camp:get_wares("fish"))
end
function trainingsite_tests:test_set_wares_array_arg()
   self.camp:set_wares{fish=3, meat=2}
   assert_equal(3, self.camp:get_wares("fish"))
   assert_equal(2, self.camp:get_wares("meat"))
end
function trainingsite_tests:test_set_wares_illegal_name()
   assert_error("illegal ware", function()
      self.camp:set_wares{meat = 2, log=1}
   end)
   assert_error("illegal ware", function()
      self.camp:set_wares("log",1)
   end)
end
function trainingsite_tests:test_set_wares_nonexistant_name()
   assert_error("illegal ware", function()
      self.camp:set_wares{meat = 2, balloon=1}
   end)
   assert_error("illegal ware", function()
      self.camp:set_wares("balloon",1)
   end)
end
function trainingsite_tests:test_set_wares_negative_count()
   assert_error("negative counts", function()
      self.camp:set_wares("meat", -1)
   end)
end
function trainingsite_tests:test_set_wares_illegal_count()
   assert_error("too big count", function()
      self.camp:set_wares("meat", 10)
   end)
end
function trainingsite_tests:test_get_wares_array_arg()
   self.camp:set_wares{fish=3, meat=2}
   rv = self.camp:get_wares{"fish", "meat"}
   assert_equal(3, rv.fish)
   assert_equal(2, rv.meat)
   assert_equal(nil, rv.helm)
end
function trainingsite_tests:test_get_wares_all_arg()
   self.camp:set_wares{fish=3, meat=2}
   rv = self.camp:get_wares("all")
   assert_equal(0, rv.pittabread)
   assert_equal(0, rv.helm)
   assert_equal(2, rv.meat)
   assert_equal(3, rv.fish)
   assert_equal(nil, rv.log)
end
function trainingsite_tests:test_get_wares_string_arg()
   self.camp:set_wares{fish=3, meat=2}
   assert_equal(2, self.camp:get_wares("meat"))
   assert_equal(3, self.camp:get_wares("fish"))
   assert_equal(0, self.camp:get_wares("beer"))
   assert_equal(0, self.camp:get_wares("strongbeer"))
   assert_equal(0, self.camp:get_wares("log"))
end
function trainingsite_tests:test_get_wares_non_storable_wares()
   self.camp:set_wares{fish=3, meat=2}
   local rv = self.camp:get_wares{"meat", "log", "fish"}
   assert_equal(0, rv.log)
   assert_equal(2, rv.meat)
   assert_equal(3, rv.fish)
   assert_equal(nil, rv.strongbeer)
end
function trainingsite_tests:test_get_wares_non_existant_name()
   assert_error("non existent ware", function()
      self.camp:get_wares("balloon")
   end)
   assert_error("non existent ware", function()
      self.camp:get_wares{"meat", "balloon"}
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

