set_textdomain("tribes")

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
--  ***************** ImmovableDescription *****************
--  =======================================================

function test_descr:test_immovable_descr()
   -- NOCOM(#codereview): Wrong immovable -> Unknown immovable?
   assert_error("Wrong immovable", function() egbase:get_immovable_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function()
      egbase:get_immovable_description("XXX", "YYY")
   end)
   assert_error("Wrong number of parameters: 3", function()
      egbase:get_immovable_description("XXX","YYY","ZZZ")
   end)
end

function test_descr:test_immovable_species()
   assert_equal("", egbase:get_immovable_description("bush1").species)
   assert_equal("", egbase:get_immovable_description("cornfield_ripe").species)
   assert_equal("", egbase:get_immovable_description("alder_summer_sapling").species)
   assert_equal("Alder", egbase:get_immovable_description("alder_summer_old").species)
end

function test_descr:test_immovable_build_cost()
   local build_cost = egbase:get_immovable_description(
      "atlanteans_shipconstruction").build_cost
   assert_equal(10, build_cost["planks"])
   assert_equal(2, build_cost["log"])
   assert_equal(4, build_cost["spidercloth"])
   assert_equal(nil, build_cost["wine"])

   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(16, total_cost(build_cost))
end

function test_descr:test_immovable_editor_category()
   assert_equal("plants", egbase:get_immovable_description("bush1").editor_category.name)
   assert_equal("Plants", egbase:get_immovable_description("bush1").editor_category.descname)
   assert_equal(nil, egbase:get_immovable_description("cornfield_ripe").editor_category)
   assert_equal("trees_deciduous", egbase:get_immovable_description(
      "alder_summer_sapling").editor_category.name)
   assert_equal("Deciduous Trees", egbase:get_immovable_description(
      "alder_summer_sapling").editor_category.descname)
   assert_equal("trees_deciduous", egbase:get_immovable_description(
      "alder_summer_old").editor_category.name)
end

function test_descr:test_immovable_terrain_affinity()
   assert_equal(nil, egbase:get_immovable_description("bush1").terrain_affinity)
   assert_equal(nil, egbase:get_immovable_description("cornfield_ripe").terrain_affinity)

   local aff_alder_sapling = egbase:get_immovable_description("alder_summer_sapling").terrain_affinity
   local aff_alder_old = egbase:get_immovable_description("alder_summer_old").terrain_affinity
   local aff_mushroom_red_pole = egbase:get_immovable_description("mushroom_red_wasteland_pole").terrain_affinity
   local aff_umbrella_green_mature = egbase:get_immovable_description("umbrella_green_wasteland_mature").terrain_affinity

   -- Pickiness
   assert_near(0.6, aff_alder_sapling["pickiness"], 0.01)
   assert_equal(aff_alder_sapling["pickiness"], aff_alder_old["pickiness"])
   assert_near(0.6, aff_mushroom_red_pole["pickiness"], 0.01)
   assert_near(0.8, aff_umbrella_green_mature["pickiness"], 0.01)

   -- preferred_fertility
   assert_near(0.6, aff_alder_sapling["preferred_fertility"], 0.01)
   assert_equal(aff_alder_sapling["preferred_fertility"], aff_alder_old["preferred_fertility"])
   assert_near(0.85, aff_mushroom_red_pole["preferred_fertility"], 0.01)
   assert_near(0.85, aff_umbrella_green_mature["preferred_fertility"], 0.01)

   -- preferred_humidity
   assert_near(0.65, aff_alder_sapling["preferred_humidity"], 0.01)
   assert_equal(aff_alder_sapling["preferred_humidity"], aff_alder_old["preferred_humidity"])
   assert_near(0.35, aff_mushroom_red_pole["preferred_humidity"], 0.01)
   assert_near(0.2, aff_umbrella_green_mature["preferred_humidity"], 0.01)

   -- preferred_temperature
   assert_equal(125, aff_alder_sapling["preferred_temperature"])
   assert_equal(aff_alder_sapling["preferred_temperature"], aff_alder_old["preferred_temperature"])
   assert_equal(80, aff_mushroom_red_pole["preferred_temperature"])
   assert_equal(110, aff_umbrella_green_mature["preferred_temperature"])
end

function test_descr:test_immovable_owner_type()
   assert_equal("world", egbase:get_immovable_description("bush1").owner_type)
   assert_equal("tribe", egbase:get_immovable_description("cornfield_ripe").owner_type)
   assert_equal("world", egbase:get_immovable_description("alder_summer_sapling").owner_type)
   assert_equal("world", egbase:get_immovable_description("alder_summer_old").owner_type)
end

function test_descr:test_immovable_size()
   assert_equal(0, egbase:get_immovable_description("bush1").size)
   assert_equal(1, egbase:get_immovable_description("cornfield_ripe").size)
   assert_equal(1, egbase:get_immovable_description("alder_summer_sapling").size)
   assert_equal(1, egbase:get_immovable_description("alder_summer_old").size)
end

function test_descr:test_immovable_has_attribute()
   assert_equal(false, egbase:get_immovable_description("bush1"):has_attribute("tree"))
   assert_equal(false, egbase:get_immovable_description("alder_summer_sapling"):has_attribute("tree"))
   assert_equal(true, egbase:get_immovable_description("alder_summer_old"):has_attribute("tree"))
end

function test_descr:test_immovable_probability_to_grow()
   assert_equal(nil, egbase:get_immovable_description("bush1"):probability_to_grow("wiese1"))

   local alder = egbase:get_immovable_description("alder_summer_sapling")
   assert_near(0.51, alder:probability_to_grow("wiese1"), 0.01)
   assert_near(0.0022, alder:probability_to_grow("wasteland_beach"), 0.0001)
   assert_near(0.66, alder:probability_to_grow("desert_forested_mountain2"), 0.01)
   assert_near(0.000037, alder:probability_to_grow("winter_water"), 0.000001)
end


--  =======================================================
--  ***************** BuildingDescription *****************
--  =======================================================

function test_descr:test_building_descr()
   assert_error("Wrong building", function() egbase:get_building_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function() egbase:get_building_description("XXX", "YYY") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_building_description("XXX","YYY","ZZZ") end)
end


-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Lumberjack’s Hut", egbase:get_building_description("barbarians_lumberjacks_hut").descname)
   assert_equal("Battle Arena", egbase:get_building_description("barbarians_battlearena").descname)
   assert_equal("Fortress", egbase:get_building_description("barbarians_fortress").descname)
   assert_equal("Coal Mine", egbase:get_building_description("barbarians_coalmine").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("barbarians_lumberjacks_hut", egbase:get_building_description("barbarians_lumberjacks_hut").name)
   assert_equal("barbarians_battlearena", egbase:get_building_description("barbarians_battlearena").name)
   assert_equal("barbarians_fortress", egbase:get_building_description("barbarians_fortress").name)
   assert_equal("barbarians_coalmine", egbase:get_building_description("barbarians_coalmine").name)
end

function test_descr:test_build_cost()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(2, total_cost(egbase:get_building_description("barbarians_sentry").build_cost))
   assert_equal(20, total_cost(egbase:get_building_description("barbarians_fortress").build_cost))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians_citadel").build_cost))
end

function test_descr:test_buildable()
   assert_equal(false, egbase:get_building_description("barbarians_headquarters").buildable)
   assert_equal(true, egbase:get_building_description("barbarians_sentry").buildable)
end

function test_descr:test_conquers()
   assert_equal(0, egbase:get_building_description("barbarians_lumberjacks_hut").conquers)
   assert_equal(6, egbase:get_building_description("barbarians_sentry").conquers)
   assert_equal(9, egbase:get_building_description("barbarians_headquarters").conquers)
   assert_equal(0, egbase:get_building_description("barbarians_coalmine").conquers)
end

function test_descr:test_destructible()
   assert_equal(false, egbase:get_building_description("barbarians_headquarters").destructible)
   assert_equal(true, egbase:get_building_description("barbarians_sentry").destructible)
end

function test_descr:test_enhanced()
   assert_equal(false, egbase:get_building_description("barbarians_headquarters").enhanced)
   assert_equal(true, egbase:get_building_description("barbarians_ax_workshop").enhanced)
end

function test_descr:test_enhancement_cost()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(0, total_cost(egbase:get_building_description("barbarians_sentry").enhancement_cost))
   assert_equal(20, total_cost(egbase:get_building_description("barbarians_citadel").enhancement_cost))
end

function test_descr:test_enhancement()
   assert_equal("barbarians_inn", egbase:get_building_description("barbarians_tavern").enhancement.name)
   assert_equal(nil, egbase:get_building_description("barbarians_lumberjacks_hut").enhancement)
end

function test_descr:test_icon_name()
   assert_equal("tribes/buildings/warehouses/barbarians/headquarters/menu.png", egbase:get_building_description("barbarians_headquarters").icon_name)
end

function test_descr:test_ismine()
   assert_equal(false, egbase:get_building_description("barbarians_headquarters").is_mine)
   assert_equal(true, egbase:get_building_description("barbarians_ironmine").is_mine)
end

function test_descr:test_isport()
   assert_equal(false, egbase:get_building_description("barbarians_headquarters").is_port)
   assert_equal(true, egbase:get_building_description("barbarians_port").is_port)
end

function test_descr:test_returned_wares()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(1, total_cost(egbase:get_building_description("barbarians_sentry").returned_wares))
   assert_equal(9, total_cost(egbase:get_building_description("barbarians_fortress").returned_wares))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians_citadel").returned_wares))
end

function test_descr:test_returned_wares_enhanced()
   local total_cost = function(t)
      local cost = 0
      for name, count in pairs(t) do
         cost = cost + count
      end
      return cost
   end
   assert_equal(0, total_cost(egbase:get_building_description("barbarians_sentry").returned_wares_enhanced))
   assert_equal(0, total_cost(egbase:get_building_description("barbarians_fortress").returned_wares_enhanced))
   assert_equal(10, total_cost(egbase:get_building_description("barbarians_citadel").returned_wares_enhanced))
end

function test_descr:test_size()
   assert_equal(1, egbase:get_building_description("barbarians_lumberjacks_hut").size)
   assert_equal(2, egbase:get_building_description("barbarians_reed_yard").size)
   assert_equal(3, egbase:get_building_description("barbarians_fortress").size)
   assert_equal(1, egbase:get_building_description("barbarians_coalmine").size)
end

function test_descr:test_type()
   assert_equal("militarysite", egbase:get_building_description("barbarians_sentry").descr.type_name)
end

function test_descr:test_vision_range()
-- if vision_range is not set in the conf, it is get_conquers() + 4
   assert_equal(4, egbase:get_building_description("barbarians_lumberjacks_hut").vision_range)
   assert_equal(2, egbase:get_building_description("constructionsite").vision_range)
   assert_equal(4+11, egbase:get_building_description("barbarians_fortress").vision_range)
   assert_equal(17, egbase:get_building_description("barbarians_tower").vision_range)
end

--  =======================================================
--  ************** ProductionSiteDescription **************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Coal Mine", egbase:get_building_description("barbarians_coalmine").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("barbarians_coalmine", egbase:get_building_description("barbarians_coalmine").name)
end

function test_descr:test_inputs()
   local building_description = egbase:get_building_description("barbarians_bakery")
   assert_equal("water", building_description.inputs[1].name)
   assert_equal("wheat", building_description.inputs[2].name)
   building_description = egbase:get_building_description("barbarians_lumberjacks_hut")
   assert_equal(nil, building_description.inputs[1])
end

function test_descr:test_output_ware_types()
   local building_description = egbase:get_building_description("barbarians_bakery")
   assert_equal("barbarians_bread", building_description.output_ware_types[1].name)
   building_description = egbase:get_building_description("barbarians_gamekeepers_hut")
   assert_equal(nil, building_description.output_ware_types[1])
end

function test_descr:test_output_worker_types()
   local building_description = egbase:get_building_description("barbarians_cattlefarm")
   assert_equal("barbarians_ox", building_description.output_worker_types[1].name)
   building_description = egbase:get_building_description("barbarians_gamekeepers_hut")
   assert_equal(nil, building_description.output_ware_types[1])
end

function test_descr:test_type()
   assert_equal("productionsite", egbase:get_building_description("barbarians_coalmine").descr.type_name)
end

function test_descr:test_working_positions()
   local building_description = egbase:get_building_description("barbarians_coalmine_deeper")
   assert_equal("barbarians_miner", building_description.working_positions[1].name)
   assert_equal("barbarians_miner_chief", building_description.working_positions[2].name)
   assert_equal("barbarians_miner_master", building_description.working_positions[3].name)
   building_description = egbase:get_building_description("barbarians_big_inn")
   assert_equal("barbarians_innkeeper", building_description.working_positions[1].name)
   assert_equal("barbarians_innkeeper", building_description.working_positions[2].name)
end


--  =======================================================
--  *************** MilitarySiteDescription ***************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Sentry", egbase:get_building_description("barbarians_sentry").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("barbarians_sentry", egbase:get_building_description("barbarians_sentry").name)
end

function test_descr:test_heal_per_second()
   assert_equal(80, egbase:get_building_description("barbarians_sentry").heal_per_second)
   assert_equal(170, egbase:get_building_description("barbarians_fortress").heal_per_second)
end

function test_descr:test_max_number_of_soldiers()
   assert_equal(2, egbase:get_building_description("barbarians_sentry").max_number_of_soldiers)
   assert_equal(8, egbase:get_building_description("barbarians_fortress").max_number_of_soldiers)
end

function test_descr:test_type()
   assert_equal("militarysite", egbase:get_building_description("barbarians_sentry").descr.type_name)
end


--  =======================================================
--  *************** TrainingSiteDescription ***************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Battle Arena", egbase:get_building_description("barbarians_battlearena").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("battlearena", egbase:get_building_description("barbarians_battlearena").name)
end

function test_descr:test_max_attack()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").max_attack)
   assert_equal(4, egbase:get_building_description("barbarians_trainingcamp").max_attack)
end

function test_descr:test_max_defense()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").max_defense)
   assert_equal(nil, egbase:get_building_description("barbarians_trainingcamp").max_defense)
   assert_equal(1, egbase:get_building_description("atlanteans_labyrinth").max_defense)
end

function test_descr:test_max_evade()
   assert_equal(1, egbase:get_building_description("barbarians_battlearena").max_evade)
   assert_equal(nil, egbase:get_building_description("barbarians_trainingcamp").max_evade)
end

function test_descr:test_max_hp()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").max_hp)
   assert_equal(2, egbase:get_building_description("barbarians_trainingcamp").max_hp)
end

function test_descr:test_min_attack()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").min_attack)
   assert_equal(0, egbase:get_building_description("barbarians_trainingcamp").min_attack)
end

function test_descr:test_min_defense()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").min_defense)
   assert_equal(nil, egbase:get_building_description("barbarians_trainingcamp").min_defense)
   assert_equal(0, egbase:get_building_description("atlanteans_labyrinth").min_defense)
end

function test_descr:test_min_evade()
   assert_equal(0, egbase:get_building_description("barbarians_battlearena").min_evade)
   assert_equal(nil, egbase:get_building_description("barbarians_trainingcamp").min_evade)
end

function test_descr:test_min_hp()
   assert_equal(nil, egbase:get_building_description("barbarians_battlearena").min_hp)
   assert_equal(0, egbase:get_building_description("barbarians_trainingcamp").min_hp)
end

function test_descr:test_type()
   assert_equal("trainingsite", egbase:get_building_description("barbarians_battlearena").descr.type_name)
end


--  =======================================================
--  **************** WarehouseDescription *****************
--  =======================================================

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Warehouse", egbase:get_building_description("barbarians_warehouse").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("barbarians_warehouse", egbase:get_building_description("barbarians_warehouse").name)
end

function test_descr:test_heal_per_second()
   assert_equal(170, egbase:get_building_description("barbarians_warehouse").heal_per_second)
   assert_equal(220, egbase:get_building_description("barbarians_headquarters").heal_per_second)
end

function test_descr:test_type()
   assert_equal("warehouse", egbase:get_building_description("barbarians_warehouse").type_name)
end


--  =======================================================
--  ***************** WareDescription *****************
--  =======================================================

function test_descr:test_ware_descr()
   assert_error("Wrong ware", function() egbase:get_ware_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function() egbase:get_ware_description("XXX","YYY") end)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Thatch Reed", egbase:get_ware_description("thatch_reed").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("thatch_reed", egbase:get_ware_description("thatch_reed").name)
end

function test_descr:test_consumers()
   local find_building = function(findme, t)
      local found = false
      for ids, ware_description in pairs(t) do
         if (ware_description.name == findme) then
            found = true
         end
      end
      return found
   end

   local ware_description = egbase:get_ware_description("coal")
   assert_equal(true, find_building("barbarians_lime_kiln", ware_description.consumers))
   assert_equal(true, find_building("empire_smelting_works", ware_description.consumers))
   assert_equal(true, find_building("atlanteans_smelting_works", ware_description.consumers))
   assert_equal(true, find_building("barbarians_warmill", ware_description.consumers))
   assert_equal(true, find_building("barbarians_ax_workshop", ware_description.consumers))
   assert_equal(true, find_building("barbarians_helmsmithy", ware_description.consumers))
   assert_equal(false, find_building("atlanteans_crystalmine", ware_description.producers))
end

function test_descr:test_icon_name()
   assert_equal("tribes/wares/coal/menu.png", egbase:get_ware_description("coal").icon_name)
end

function test_descr:test_producers()
   local find_building = function(findme, t)
      local found = false
      for ids, ware_description in pairs(t) do
         if (ware_description.name == findme) then
            found = true
         end
      end
      return found
   end

   local ware_description = egbase:get_ware_description("coal")
   assert_equal(true, find_building("empire_charcoal_kiln", ware_description.producers))
   assert_equal(true, find_building("barbarians_coalmine_deeper", ware_description.producers))
   assert_equal(true, find_building("barbarians_coalmine_deep", ware_description.producers))
   assert_equal(true, find_building("atlanteans_coalmine", ware_description.producers))
   assert_equal(false, find_building("atlanteans_crystalmine", ware_description.producers))
end

function test_descr:is_construction_material()
   local ware_description = egbase:get_ware_description("coal")
   assert_equal(false, ware_description.is_construction_material("barbarians"))
   ware_description = egbase:get_ware_description("log")
   assert_equal(true, ware_description.is_construction_material("atlanteans"))
   ware_description = egbase:get_ware_description("planks")
   assert_equal(false, ware_description.is_construction_material("barbarians"))
   ware_description = egbase:get_ware_description("planks")
   assert_equal(true, ware_description.is_construction_material("atlanteans"))
end

--  =======================================================
--  ****************** WorkerDescription ******************
--  =======================================================

function test_descr:test_worker_descr()
   assert_error("Wrong worker", function() egbase:get_worker_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function() egbase:get_worker_description("XXX","YYY") end)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_descname()
   assert_equal("Miner", egbase:get_worker_description("barbarians_miner").descname)
end

-- This is actually a property of MapOjectDescription
function test_descr:test_name()
   assert_equal("barbarians_miner", egbase:get_worker_description("barbarians_miner").name)
end

function test_descr:test_becomes()
   local worker_descr = egbase:get_worker_description("barbarians_miner").becomes
   assert_equal("barbarians_miner_chief", worker_descr.name)
   worker_descr = egbase:get_worker_description("barbarians_miner_chief").becomes
   assert_equal("barbarians_miner_master", worker_descr.name)
   worker_descr = egbase:get_worker_description("barbarians_miner_master").becomes
   assert_equal(nil, worker_descr)
end

function test_descr:test_icon_name()
   assert_equal("tribes/workers/barbarians/miner/menu.png", egbase:get_worker_description("barbarians_miner").icon_name)
end

function test_descr:test_needed_experience()
   assert_equal(19, egbase:get_worker_description("barbarians_miner").needed_experience)
   assert_equal(28, egbase:get_worker_description("barbarians_miner_chief").needed_experience)
end
