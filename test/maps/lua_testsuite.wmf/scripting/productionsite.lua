-- =======================================================================
--                          ProductionSite Testings                         
-- =======================================================================
   
function _cnt(i)
   local rv = 0
   for name,cnt in pairs(i) do rv = rv + cnt end
   return rv
end

-- ================
-- Worker creation 
-- ================
productionsite_tests = lunit.TestCase("Productionsite Tests")
function productionsite_tests:setup()
   self.f1 = map:get_field(10,10)
   self.f2 = map:get_field(12,10)
   self.f3 = map:get_field(14,10)
   player1:conquer(self.f1, 4)
   player1:conquer(self.f2, 4)
   player1:conquer(self.f3, 4)

   self.inn = player1:place_building("big_inn", self.f1, false, true)
   self.warmill = player1:place_building("warmill", self.f2, false, true)
   self.lumberjack = player1:place_building("lumberjacks_hut", self.f3, false, true)
end
function productionsite_tests:teardown()
   pcall(function()
      self.f1.brn.immovable:remove()
   end)
   pcall(function()
      self.f2.brn.immovable:remove()
   end)
   pcall(function()
      self.f3.brn.immovable:remove()
   end)
end
function productionsite_tests:test_name()
   assert_equal("big_inn", self.inn.name)
   assert_equal("warmill", self.warmill.name)
   assert_equal("lumberjacks_hut", self.lumberjack.name)
end
function productionsite_tests:test_type()
   assert_equal("productionsite", self.inn.type)
   assert_equal("productionsite", self.warmill.type)
   assert_equal("productionsite", self.lumberjack.type)
end
function productionsite_tests:test_no_workers_initially()
   assert_equal(0, _cnt(self.inn:get_workers("all")))
   assert_equal(0, _cnt(self.warmill:get_workers("all")))
end
function productionsite_tests:test_valid_workers()
   assert_equal(2, _cnt(self.inn.valid_workers))
   assert_equal(2, self.inn.valid_workers.innkeeper)
   assert_equal(2, _cnt(self.warmill.valid_workers))
   assert_equal(1, self.warmill.valid_workers.blacksmith)
   assert_equal(1, self.warmill.valid_workers["master-blacksmith"])
end
function productionsite_tests:test_set_workers()
   self.inn:set_workers("innkeeper", 1)
   assert_equal(1, _cnt(self.inn:get_workers("all")))
   assert_equal(1, self.inn:get_workers("innkeeper"))
   self.inn:set_workers{innkeeper=2}
   assert_equal(2, _cnt(self.inn:get_workers("all")))
   assert_equal(2, self.inn:get_workers("innkeeper"))
   local rv = self.inn:get_workers{"innkeeper", "carrier"}
   assert_equal(2, rv.innkeeper)
   assert_equal(0, rv.carrier)
   assert_equal(nil, rv.blacksmith)
end
function productionsite_tests:test_worker_name()
   local f = self.inn.fields[1]
   while #f.bobs > 0 do
      f.bobs[1]:remove()
   end
   self.inn:set_workers("innkeeper", 1) -- Innkeeper will be created as an instance.
   assert_equal(1, #f.bobs)
   assert_equal(f.bobs[1].name, "innkeeper")
end
function productionsite_tests:test_set_workers_warmill()
   self.warmill:set_workers("master-blacksmith",1)
   assert_equal(1, _cnt(self.warmill:get_workers("all")))
   local rv = self.warmill:get_workers{"blacksmith", "master-blacksmith", "carrier"}
   assert_equal(0, rv.blacksmith)
   assert_equal(1, rv["master-blacksmith"])
   assert_equal(0, rv.carrier)

   self.warmill:set_workers("blacksmith",1)
   assert_equal(1, _cnt(self.warmill:get_workers("all")))
   local rv = self.warmill:get_workers{"blacksmith", "master-blacksmith", "carrier"}
   assert_equal(1, rv.blacksmith)
   assert_equal(0, rv["master-blacksmith"])
   assert_equal(0, rv.carrier)

   self.warmill:set_workers{blacksmith=1, ["master-blacksmith"] = 1}
   assert_equal(2, _cnt(self.warmill:get_workers("all")))
   local rv = self.warmill:get_workers{"blacksmith", "master-blacksmith", "carrier"}
   assert_equal(1, rv.blacksmith)
   assert_equal(1, rv["master-blacksmith"])
   assert_equal(0, rv.carrier)
end
function productionsite_tests:test_get_workers_all()
   self.warmill:set_workers{blacksmith=1, ["master-blacksmith"] = 1}
   local rv = self.warmill:get_workers("all")
   assert_equal(1, rv.blacksmith)
   assert_equal(1, rv["master-blacksmith"])
   assert_equal(nil, rv.carrier)
end

function productionsite_tests:test_illegal_name()
   assert_error("illegal name", function()
      self.warmill:set_workers("jhsf",1)
   end)
end
function productionsite_tests:test_illegal_worker()
  assert_error("illegal worker", function()
      self.warmill:set_workers("lumberjack", 1)
  end)
end
function productionsite_tests:test_no_space()
  assert_error("no_space", function()
      self.warmill:set_workers{blacksmith=2}
  end)
end

-- ==============
-- Ware creation
-- ==============
function productionsite_tests:test_valid_wares()
   ww = self.warmill.valid_wares
   assert_equal(8, ww.iron)
   assert_equal(8, ww.coal)
   assert_equal(8, ww.gold)
   assert_equal(nil, ww.water)
end
function productionsite_tests:test_valid_wares_correct_length()
   inn = self.inn.valid_wares
   c = {}
   for n,count in pairs(inn) do c[#c+1] = n end
   assert_equal(5, #c)
   assert_equal(4, inn.fish)
   assert_equal(4, inn.pittabread)
   assert_equal(4, inn.meat)
   assert_equal(4, inn.beer)
   assert_equal(4, inn.strongbeer)
end
function productionsite_tests:test_valid_wares_correct_length1()
   c = {}
   for n,count in pairs(self.lumberjack.valid_wares) do c[#c+1] = n end
   assert_equal(0, #c)
end

function productionsite_tests:test_houses_empty_at_creation()
   for idx,house in ipairs{self.warmill, self.inn, self.lumberjack} do
      print(house)
      for wname, count in pairs(house.valid_wares) do
         assert_equal(0, house:get_wares(wname))
      end
   end
end
function productionsite_tests:test_set_wares_string_arg()
   self.inn:set_wares("fish", 3)
   assert_equal(3, self.inn:get_wares("fish"))
end
function productionsite_tests:test_set_wares_array_arg()
   self.inn:set_wares{fish=3, strongbeer=2}
   assert_equal(3, self.inn:get_wares("fish"))
   assert_equal(2, self.inn:get_wares("strongbeer"))
end
function productionsite_tests:test_set_wares_illegal_name()
   assert_error("illegal ware", function()
      self.inn:set_wares{meat = 2, log=1}
   end)
   assert_error("illegal ware", function()
      self.inn:set_wares("log",1)
   end)
end
function productionsite_tests:test_set_wares_nonexistant_name()
   assert_error("illegal ware", function()
      self.inn:set_wares{meat = 2, balloon=1}
   end)
   assert_error("illegal ware", function()
      self.inn:set_wares("balloon",1)
   end)
end
function productionsite_tests:test_set_wares_negative_count()
   assert_error("negative counts", function()
      self.inn:set_wares("meat", -1)
   end)
end
function productionsite_tests:test_set_wares_illegal_count()
   self.inn:set_wares("meat", 4)
   assert_error("too big count", function()
      self.inn:set_wares("meat", 5)
   end)
end
function productionsite_tests:test_get_wares_array_arg()
   self.inn:set_wares{fish=3, strongbeer=2}
   rv = self.inn:get_wares{"fish", "strongbeer"}
   assert_equal(3, rv.fish)
   assert_equal(2, rv.strongbeer)
   assert_equal(nil, rv.meat)
end
function productionsite_tests:test_get_wares_all_arg()
   self.inn:set_wares{fish=3, strongbeer=2}
   rv = self.inn:get_wares("all")
   assert_equal(0, rv.pittabread)
   assert_equal(0, rv.meat)
   assert_equal(0, rv.beer)
   assert_equal(3, rv.fish)
   assert_equal(2, rv.strongbeer)
   assert_equal(nil, rv.log)
end
function productionsite_tests:test_get_wares_string_arg()
   self.inn:set_wares{fish=3, strongbeer=2}
   assert_equal(0, self.inn:get_wares("pittabread"))
   assert_equal(0, self.inn:get_wares("meat"))
   assert_equal(0, self.inn:get_wares("beer"))
   assert_equal(3, self.inn:get_wares("fish"))
   assert_equal(2, self.inn:get_wares("strongbeer"))
   assert_equal(0, self.inn:get_wares("log"))
end
function productionsite_tests:test_get_wares_non_storable_wares()
   self.inn:set_wares{fish=3, strongbeer=2}
   local rv = self.inn:get_wares{"meat", "log", "fish"}
   assert_equal(0, rv.meat)
   assert_equal(0, rv.log)
   assert_equal(3, rv.fish)
   assert_equal(nil, rv.strongbeer)
end
function productionsite_tests:test_get_wares_non_existant_name()
   assert_error("non existent ware", function()
      self.inn:get_wares("balloon")
   end)
   assert_error("non existent ware", function()
      self.inn:get_wares{"meat", "balloon"}
   end)
end

