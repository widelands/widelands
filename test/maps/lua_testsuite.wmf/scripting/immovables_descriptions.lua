set_textdomain("tribe_barbarians")

test_descr = lunit.TestCase("Immovable descriptions test")
function test_descr:test_instantiation_forbidden()
   assert_error("Cannot instantiate", function()
      wl.map.BuildingDescription()
   end)
   assert_error("Cannot instantiate", function()
      wl.map.WareDescription()
   end)
   assert_error("Cannot instantiate", function()
      wl.map.WorkerDescription()
   end)
end

--  =======================================================
--  ***************** BuildingDescription *****************
--  =======================================================

function test_descr:test_building_descr()
   assert_error("Wrong tribe", function() egbase:get_building_description("XXX","sentry") end)
   assert_error("Wrong building", function() egbase:get_building_description("barbarians","XXX") end)
   assert_error("Wrong number of parameters: 1", function() egbase:get_building_description("XXX") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_building_description("XXX","YYY","ZZZ") end)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Lumberjack’s Hut", egbase:get_building_description("barbarians","lumberjacks_hut").descname)
   assert_equal(_"Battle Arena", egbase:get_building_description("barbarians","battlearena").descname)
   assert_equal(_"Fortress", egbase:get_building_description("barbarians","fortress").descname)
   assert_equal(_"Coal Mine", egbase:get_building_description("barbarians","coalmine").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("lumberjacks_hut", egbase:get_building_description("barbarians","lumberjacks_hut").name)
   assert_equal("battlearena", egbase:get_building_description("barbarians","battlearena").name)
   assert_equal("fortress", egbase:get_building_description("barbarians","fortress").name)
   assert_equal("coalmine", egbase:get_building_description("barbarians","coalmine").name)
end

function test_descr:test_build_cost()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(2, total_cost(egbase:get_building_description("barbarians","sentry").build_cost))
   assert_equal(20, total_cost(egbase:get_building_description("barbarians","fortress").build_cost))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians","citadel").build_cost))
end

function test_descr:test_buildable()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").buildable)
   assert_equal(true, egbase:get_building_description("barbarians","sentry").buildable)
end

function test_descr:test_conquers()
   assert_equal(0, egbase:get_building_description("barbarians","lumberjacks_hut").conquers)
   assert_equal(6, egbase:get_building_description("barbarians","sentry").conquers)
   assert_equal(9, egbase:get_building_description("barbarians","headquarters").conquers)
   assert_equal(0, egbase:get_building_description("barbarians","coalmine").conquers)
end

function test_descr:test_destructible()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").destructible)
   assert_equal(true, egbase:get_building_description("barbarians","sentry").destructible)
end

function test_descr:test_enhanced()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").enhanced)
   assert_equal(true, egbase:get_building_description("barbarians","axfactory").enhanced)
end

function test_descr:test_enhancement_cost()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(0, total_cost(egbase:get_building_description("barbarians","sentry").enhancement_cost))
   assert_equal(20, total_cost(egbase:get_building_description("barbarians","citadel").enhancement_cost))
end

function test_descr:test_enhancements()
-- TODO
end

function test_descr:test_ismine()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").ismine)
   assert_equal(true, egbase:get_building_description("barbarians","oremine").ismine)
end

function test_descr:test_isport()
   assert_equal(false, egbase:get_building_description("barbarians","headquarters").isport)
   assert_equal(true, egbase:get_building_description("barbarians","port").isport)
end

function test_descr:test_returned_wares()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(1, total_cost(egbase:get_building_description("barbarians","sentry").returned_wares))
   assert_equal(9, total_cost(egbase:get_building_description("barbarians","fortress").returned_wares))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians","citadel").returned_wares))
end

function test_descr:test_returned_wares_enhanced()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(0, total_cost(egbase:get_building_description("barbarians","sentry").returned_wares_enhanced))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians","fortress").returned_wares_enhanced))
   assert_equal(10, total_cost(egbase:get_building_description("barbarians","citadel").returned_wares_enhanced))
end

function test_descr:test_size()
   assert_equal(1, egbase:get_building_description("barbarians","lumberjacks_hut").size)
   assert_equal(2, egbase:get_building_description("barbarians","reed_yard").size)
   assert_equal(3, egbase:get_building_description("barbarians","fortress").size)
   assert_equal(1, egbase:get_building_description("barbarians","coalmine").size)
end

function test_descr:test_type()
   assert_equal("militarysite", egbase:get_building_description("barbarians","sentry").type)
end

function test_descr:test_vision_range()
-- if vision_range is not set in the conf, it is get_conquers() + 4
   assert_equal(4, egbase:get_building_description("barbarians","lumberjacks_hut").vision_range)
   assert_equal(2, egbase:get_building_description("barbarians","constructionsite").vision_range)
   assert_equal(4+11, egbase:get_building_description("barbarians","fortress").vision_range)
   assert_equal(17, egbase:get_building_description("barbarians","donjon").vision_range)
end

--  =======================================================
--  ************** ProductionSiteDescription **************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Coal Mine", egbase:get_building_description("barbarians","coalmine").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("coalmine", egbase:get_building_description("barbarians","coalmine").name)
end

-- TODO	int get_inputs(lua_State *);
-- TODO	int get_output_ware_types(lua_State *);

function test_descr:test_type()
   assert_equal("productionsite", egbase:get_building_description("barbarians","coalmine").type)
end

-- TODO	int get_working_positions(lua_State *);


--  =======================================================
--  *************** MilitarySiteDescription ***************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Sentry", egbase:get_building_description("barbarians","sentry").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("sentry", egbase:get_building_description("barbarians","sentry").name)
end

function test_descr:test_heal_per_second()
   assert_equal(80, egbase:get_building_description("barbarians","sentry").heal_per_second)
   assert_equal(170, egbase:get_building_description("barbarians","fortress").heal_per_second)
end

function test_descr:test_max_number_of_soldiers()
   assert_equal(2, egbase:get_building_description("barbarians","sentry").max_number_of_soldiers)
   assert_equal(8, egbase:get_building_description("barbarians","fortress").max_number_of_soldiers)
end

function test_descr:test_type()
   assert_equal("militarysite", egbase:get_building_description("barbarians","sentry").type)
end


--  =======================================================
--  *************** TrainingSiteDescription ***************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Battle Arena", egbase:get_building_description("barbarians","battlearena").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("battlearena", egbase:get_building_description("barbarians","battlearena").name)
end

function test_descr:test_max_attack()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").max_attack)
   assert_equal(4, egbase:get_building_description("barbarians","trainingcamp").max_attack)
end

function test_descr:test_max_defense()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").max_defense)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").max_defense)
end

function test_descr:test_max_evade()
   assert_equal(1, egbase:get_building_description("barbarians","battlearena").max_evade)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").max_evade)
end

function test_descr:test_max_hp()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").max_hp)
   assert_equal(2, egbase:get_building_description("barbarians","trainingcamp").max_hp)
end

function test_descr:test_min_attack()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").min_attack)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").min_attack)
end

function test_descr:test_min_defense()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").min_defense)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").min_defense)
end

function test_descr:test_min_evade()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").min_evade)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").min_evade)
end

function test_descr:test_min_hp()
   assert_equal(0, egbase:get_building_description("barbarians","battlearena").min_hp)
   assert_equal(0, egbase:get_building_description("barbarians","trainingcamp").min_hp)
end

function test_descr:test_type()
   assert_equal("trainingsite", egbase:get_building_description("barbarians","battlearena").type)
end


--  =======================================================
--  **************** WarehouseDescription *****************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Warehouse", egbase:get_building_description("barbarians","warehouse").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("warehouse", egbase:get_building_description("barbarians","warehouse").name)
end

function test_descr:test_heal_per_second()
   assert_equal(170, egbase:get_building_description("barbarians","warehouse").heal_per_second)
   assert_equal(220, egbase:get_building_description("barbarians","headquarters").heal_per_second)
end

function test_descr:test_type()
   assert_equal("warehouse", egbase:get_building_description("barbarians","warehouse").type)
end


--  =======================================================
--  ***************** WareDescription *****************
--  =======================================================

function test_descr:test_ware_descr()
   assert_error("Wrong tribe", function() egbase:get_ware_description("XXX","thatchreed") end)
   assert_error("Wrong ware", function() egbase:get_ware_description("barbarians","XXX") end)
   assert_error("Wrong number of parameters: 1", function() egbase:get_ware_description("XXX") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_ware_description("XXX","YYY","ZZZ") end)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Thatch Reed", egbase:get_ware_description("barbarians","thatchreed").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("thatchreed", egbase:get_ware_description("barbarians","thatchreed").name)
end

-- TODO consumers and producers


--  =======================================================
--  ****************** WorkerDescription ******************
--  =======================================================

function test_descr:test_worker_descr()
   assert_error("Wrong tribe", function() egbase:get_worker_description("XXX","miner") end)
   assert_error("Wrong ware", function() egbase:get_worker_description("barbarians","XXX") end)
   assert_error("Wrong number of parameters: 1", function() egbase:get_worker_description("XXX") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_worker_description("XXX","YYY","ZZZ") end)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal(_"Miner", egbase:get_worker_description("barbarians","miner").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("miner", egbase:get_worker_description("barbarians","miner").name)
end

function test_descr:test_becomes()
	local worker_descr = egbase:get_worker_description("barbarians","miner").becomes
   assert_equal("chief-miner", worker_descr.name)
	worker_descr = egbase:get_worker_description("barbarians","chief-miner").becomes
   assert_equal("master-miner", worker_descr.name)
end

function test_descr:test_level_experience()
   assert_equal(19, egbase:get_worker_description("barbarians","miner").level_experience)
   assert_equal(28, egbase:get_worker_description("barbarians","chief-miner").level_experience)
end

