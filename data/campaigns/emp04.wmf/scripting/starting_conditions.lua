-- =======================================================================
--                                 Player 3
-- =======================================================================
-- AI for P3 is set to "empty" therefore we place the buildings and the roads and let the economy flow a bit
field_well = map:get_field(17, 182)
place_building_in_region(p3, "empire_well", {field_well})

field_brewery = map:get_field(19, 183)
place_building_in_region(p3, "empire_brewery", {field_brewery})

field_mill = map:get_field(18, 184)
place_building_in_region(p3, "empire_mill", {field_mill})

field_warehouse = map:get_field(21, 186)
place_building_in_region(p3, "empire_temple_of_vesta", {field_warehouse}, {workers = {empire_carrier = 0, empire_recruit = 0}, wares = {wheat = 200}, soldiers = {[{0,0,0,0}] = 3}})

   r1 = p3:place_road("normal", field_warehouse.immovable.flag, "l", "tl", true)
   r2 = p3:place_road("normal", field_mill.immovable.flag, "tr", "r", true)
   r3 = p3:place_road("normal", field_mill.immovable.flag, "l", "tl", "tr", true)
   r4 = p3:place_road("normal", field_mill.immovable.flag, "br", "r", true)

p3:forbid_buildings("all")
field_warehouse.brn.immovable.ware_economy:set_target_quantity("beer", 180)

-- =======================================================================
--                                 Player 1
-- =======================================================================

p1:forbid_buildings("all")
p1:allow_buildings{"empire_lumberjacks_house"}

prefilled_buildings(p1,
   {"empire_headquarters", sf.x, sf.y,
   workers = {
      empire_builder = 10,
      empire_geologist = 1,
   }
   }
)

-- Lower resources to zero in starting region (to cope with default ressources)
for x=7, 42 do
   for y=10, 50 do
      local field = map:get_field(x,y)
      if field.resource == "resource_fish" or field.resource == "resource_water" then
         field.resource_amount = 0
      end
   end
end

-- Place towers and fortress
place_building_in_region(p1, "empire_tower", {map:get_field(10, 16)})
place_building_in_region(p1, "empire_tower", {map:get_field(24, 10)})
place_building_in_region(p1, "empire_tower", {map:get_field(16, 46)})
place_building_in_region(p1, "empire_tower", {map:get_field(28, 52)})
place_building_in_region(p1, "empire_tower", {map:get_field(35, 22)})
place_building_in_region(p1, "empire_fortress", {map:get_field(31, 33)})


-- Place farm
place_building_in_region(p1, "empire_farm1", {map:get_field(20, 14)})
place_building_in_region(p1, "empire_farm1", sf:region(15))

-- Place fishers_house
place_building_in_region(p1, "empire_fishers_house", {map:get_field(12, 23)})
place_building_in_region(p1, "empire_fishers_house", {map:get_field(12, 43)})

-- Place well
place_building_in_region(p1, "empire_well1", sf:region(15))
place_building_in_region(p1, "empire_well1", sf:region(15))

-- Place lumberjacks
place_building_in_region(p1, "empire_lumberjacks_house1", sf:region(10))
place_building_in_region(p1, "empire_lumberjacks_house1", sf:region(10))
place_building_in_region(p1, "empire_lumberjacks_house1", sf:region(10))
place_building_in_region(p1, "empire_foresters_house1", {map:get_field(19, 10)})
place_building_in_region(p1, "empire_foresters_house1", {map:get_field(19, 18)})

-- Mines
place_building_in_region(p1, "empire_ironmine", {map:get_field(33, 14)})
place_building_in_region(p1, "empire_coalmine", {map:get_field(24, 45)})
place_building_in_region(p1, "empire_coalmine", {map:get_field(31, 48)})
place_building_in_region(p1, "empire_goldmine", sf:region(25))

-- Place quarry
place_building_in_region(p1, "empire_quarry", sf:region(9))
place_building_in_region(p1, "empire_quarry", sf:region(9))

-- Place build material infrastructure
place_building_in_region(p1, "empire_sawmill", sf:region(8))
place_building_in_region(p1, "empire_stonemasons_house", sf:region(8))

-- Place metal industry
place_building_in_region(p1, "empire_armorsmithy", sf:region(10))
place_building_in_region(p1, "empire_toolsmithy", sf:region(10))
place_building_in_region(p1, "empire_weaponsmithy", sf:region(15))
place_building_in_region(p1, "empire_smelting_works", sf:region(15))

-- Food supply
place_building_in_region(p1, "empire_bakery", sf:region(15))
place_building_in_region(p1, "empire_brewery1", sf:region(15))
place_building_in_region(p1, "empire_winery", sf:region(15))
place_building_in_region(p1, "empire_mill1", sf:region(15))
place_building_in_region(p1, "empire_tavern", sf:region(15))

-- Military training
place_building_in_region(p1, "empire_arena", sf:region(20))
place_building_in_region(p1, "empire_trainingcamp1", sf:region(20))
place_building_in_region(p1, "empire_barracks", sf:region(20), {inputs = {empire_recruit = 8}})


-- Helper function for placing roads
function try_place_roads_with_carriers(x, y)
   local directions = {
      { "bl", "bl" },
      { "l", "l" },
      { "tl", "tl" },
      { "r", "r" },
      { "bl", "l" },
      { "tr", "tl" },
      { "br", "r" }
   }
   local field = map:get_field(x,y)
   if field.immovable and field.immovable.descr.type_name == "flag" then
      for idx, direction in ipairs(directions) do
         local from_field = field[direction[1] .. "n"]
         local to_field = from_field[direction[2] .. "n"]
         if from_field.immovable == nil and
            from_field:has_caps("walkable") and (
               to_field:has_caps("flag") or
               (to_field.immovable and to_field.immovable.descr.type_name == "flag")
            ) then
            local road = p1:place_road("normal", field.immovable, direction[1], direction[2], true)
            road:set_workers('empire_carrier', 1)
         end
      end
   end
end

-- Roads
for y=0, 53 do
   for x=5, 35 do
      try_place_roads_with_carriers(x, y)
   end
end

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{
   "barbarians_bakery",
   "barbarians_barrier",
   "barbarians_farm",
   "barbarians_fishers_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_hunters_hut",
   "barbarians_lime_kiln",
   "barbarians_lumberjacks_hut",
   "barbarians_micro_brewery",
   "barbarians_rangers_hut",
   "barbarians_reed_yard",
   "barbarians_sentry",
   "barbarians_tower",
   "barbarians_well",
   "barbarians_wood_hardener",
   "barbarians_quarry",
   "barbarians_coalmine",
   "barbarians_ironmine",
   "barbarians_goldmine",
   "barbarians_granitemine",
   "barbarians_coalmine_deep",
   "barbarians_ironmine_deep",
   "barbarians_goldmine_deep",
   "barbarians_coalmine_deeper",
   "barbarians_ironmine_deeper",
   "barbarians_goldmine_deeper",
   "barbarians_port",
   "barbarians_cattlefarm",
   "barbarians_charcoal_kiln",
   "barbarians_brewery",
   "barbarians_tavern",
   "barbarians_inn",
   "barbarians_smelting_works",
   "barbarians_metal_workshop",
   "barbarians_ax_workshop",
   "barbarians_warmill",
   "barbarians_helmsmithy",
   "barbarians_barracks",
   "barbarians_battlearena",
   "barbarians_trainingcamp",
   "barbarians_big_inn",
   "barbarians_scouts_hut",
   "barbarians_citadel",
   "barbarians_tower",
   "barbarians_fortress",
}

prefilled_buildings(p2,
   {"barbarians_headquarters", 85, 132,
   wares = {
      ax = 6,
      blackwood = 32,
      barbarians_bread = 8,
      bread_paddle = 2,
      cloth = 5,
      coal = 12,
      fire_tongs = 2,
      fish = 6,
      fishing_rod = 2,
      gold = 4,
      granite = 40,
      grout = 12,
      hammer = 12,
      hunting_spear = 2,
      iron = 12,
      iron_ore = 5,
      kitchen_tools = 4,
      log = 80,
      meal = 4,
      meat = 6,
      pick = 14,
      ration = 12,
      scythe = 6,
      shovel = 4,
      snack = 3,
      reed = 24,
   },
   workers = {
      barbarians_blacksmith = 2,
      barbarians_brewer = 1,
      barbarians_builder = 10,
      barbarians_carrier = 40,
      barbarians_charcoal_burner = 1,
      barbarians_gardener = 1,
      barbarians_geologist = 4,
      barbarians_lime_burner = 1,
      barbarians_lumberjack = 3,
      barbarians_miner = 4,
      barbarians_ranger = 1,
      barbarians_stonemason = 2,
   },

   soldiers = {
      [{0,0,0,0}] = 45,
   }
   }
)

p1.team = 1
p2.team = 2
p3.team = 2
