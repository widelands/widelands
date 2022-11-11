-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")
p1:forbid_buildings {
   "frisians_aqua_farm",
   "frisians_recycling_center",
   "frisians_furnace",
   "frisians_armor_smithy_small",
   "frisians_barracks",
   "frisians_training_camp",
   "frisians_training_arena",
   "frisians_sewing_room",
   "frisians_tailors_shop",
   "frisians_dikers_house",
   -- The map is non-seafaring, but just in case:
   "frisians_shipyard",
   "frisians_weaving_mill",
   "frisians_port",
}

port1 = p1:place_building("frisians_port", p1_start, false, true)
local wares = {
   log = 40,
   brick = 50,
   granite = 40,
   reed = 20,
   clay = 10,
   water = 5,
   coal = 30,
   fish = 2,
   meat = 2,
   fruit = 10,
   bread_frisians = 4,
   beer = 4,
   ration = 10,
   sword_long = 2,
   sword_broad = 1
}
local workers = {
   frisians_woodcutter = 3,
   frisians_forester = 7,
   frisians_claydigger = 3,
   frisians_brickmaker = 2,
   frisians_builder = 9,
   frisians_blacksmith = 2,
   frisians_smelter = 3,
   frisians_smoker = 2,
   frisians_seamstress = 1,
   frisians_landlady = 2,
   frisians_berry_farmer = 4,
   frisians_fruit_collector = 4,
   frisians_beekeeper = 1,
   frisians_fisher = 3,
   frisians_geologist = 1,
   frisians_farmer = 5,
   frisians_reed_farmer = 4,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_trainer = 2,
   frisians_stonemason = 4,
   frisians_reindeer = 1,
   frisians_charcoal_burner = 3
}

-- Saved wares
for name,amount in pairs(campaign_data.wares) do
   if name ~= "pick" and name ~= "iron" and name ~= "iron_ore" and name ~= "scrap_metal_mixed" and name ~= "scrap_iron" then
      if wares[name] then
         wares[name] = wares[name] + amount
      else
         wares[name] = amount
      end
   end
end
-- Subtract port buildcost (which was also saved)
-- (but not the builder, he will be added to the list of workers below)
for name,amount in pairs(wl.Game():get_building_description("frisians_port").buildcost) do
   if wares[name] then wares[name] = wares[name] - amount end
   if name == "gold" or name == "gold_ore" then has_gold = has_gold - amount end
end

-- Saved workers
for name,amount in pairs(campaign_data.workers) do
   if name == "frisians_miner" or name == "frisians_miner_master" then
      name = "frisians_carrier"
   end
   if workers[name] then
      workers[name] = workers[name] + amount
   else
      workers[name] = amount
   end
end

port1:set_wares(wares)
port1:set_workers(workers)

local trained = 0
for descr,n in pairs(campaign_data.port_soldiers) do
   if descr ~= "000" then
      trained = trained + n
   end
end
local soldiers = {}
if trained > takeover_soldiers then
   -- We have more soldiers than allowed, so pick 10 at random
   local remaining = takeover_soldiers
   repeat
      local key1 = math.random(0, 2)
      local key2 = math.random(0, 6)
      local key3 = math.random(0, 2)
      for descr,n in pairs(campaign_data.port_soldiers) do
         if descr == (key1 .. key2 .. key3) and n > 0 and not (key1 == 0 and key2 == 0 and key3 == 0) then
            if n < remaining then
               soldiers[{key1, key2, key3, 0}] = n
               remaining = remaining - n
            else
               soldiers[{key1, key2, key3, 0}] = remaining
               remaining = 0
            end
            campaign_data.port_soldiers[descr] = nil
            break
         end
      end
   until remaining == 0
   soldiers[{0, 0, 0, 0}] = total_soldiers - takeover_soldiers
else
   -- We have less than 10 soldiers, so take them all plus some new ones
   for h=0,2 do
      for a=0,6 do
         for d=0,2 do
            if not (h == 0 and a == 0 and d == 0) and campaign_data.port_soldiers[h .. a .. d] and campaign_data.port_soldiers[h .. a .. d] > 0 then
               soldiers[{h, a, d, 0}] = campaign_data.port_soldiers[h .. a .. d]
            end
         end
      end
   end
   soldiers[{0, 0, 0, 0}] = total_soldiers - trained
end

-- Additionally take all ship soldiers
for descr,n in pairs(campaign_data.ship_soldiers) do
   local key = {tonumber(descr:sub(1,1)), tonumber(descr:sub(2,2)), tonumber(descr:sub(3,3)), 0}
   if not soldiers[key] then
      soldiers[key] = n
   else
      soldiers[key] = soldiers[key] + n
   end
end

port1:set_soldiers(soldiers)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings {
   "empire_warehouse",
   "empire_sentry",
   "empire_blockhouse",
   "empire_barrier",
   "empire_tower",
   "empire_fortress",
   "empire_lumberjacks_house",
   "empire_foresters_house",
   "empire_quarry",
   "empire_hunters_house",
   "empire_fishers_house",
   "empire_smelting_works",
   "empire_toolsmithy",
   "empire_tavern",
   "empire_inn",
   "empire_ironmine",
   "empire_coalmine",
   "empire_marblemine",
   "empire_ironmine_deep",
   "empire_coalmine_deep",
   "empire_marblemine_deep",
   "empire_bakery",
   "empire_brewery",
   "empire_vineyard",
   "empire_winery",
   "empire_donkeyfarm",
   "empire_charcoal_kiln",
   "empire_farm",
   "empire_mill",
   "empire_piggery",
   "empire_sheepfarm",
   "empire_well",
   "empire_stonemasons_house",
   "empire_sawmill"
}
hq2 = p2:place_building("empire_headquarters", map.player_slots[2].starting_field, false, true)
hq2:set_wares {
   log = 80,
   granite = 40,
   marble = 80,
   marble_column = 20,
   planks = 40,
   cloth = 10,
   ration = 30,
   beer = 15,
   meal = 5,
   wine = 15,
   grape = 10,
   wheat = 30,
   water = 50,
   coal = 50,
   iron = 20,
   gold = 10,
   gold_ore = 5,
   iron_ore = 10,
   meat = 20,
   fish = 20,
   empire_bread = 20
}
hq2:set_workers {
   empire_builder = 3, -- To slow him down a bit
   empire_lumberjack = 8,
   empire_forester = 18,
   empire_stonemason = 7,
   empire_miner = 12,
   empire_miner_master = 4,
   empire_toolsmith = 3,
   empire_carpenter = 3,
   empire_donkey = 40,
   empire_donkeybreeder = 1,
   empire_weaver = 2,
   empire_hunter = 2,
   empire_fisher = 3,
   empire_shepherd = 2,
   empire_farmer = 9,
   empire_miller = 4,
   empire_baker = 4,
   empire_brewer = 4,
   empire_vintner = 3,
   empire_vinefarmer = 5,
   empire_pigbreeder = 3,
   empire_innkeeper = 7,
   empire_charcoal_burner = 2,
   empire_smelter = 4,
   empire_trainer = 3,
   empire_armorsmith = 1,
   empire_weaponsmith = 1,
   empire_geologist = 2
}
local nr_soldiers = {40, 40, 100}
nr_soldiers = nr_soldiers[difficulty]
hq2:set_soldiers({4,4,0,2}, nr_soldiers)

-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:forbid_buildings("all")
p3:allow_buildings {
   "barbarians_warehouse",
   "barbarians_sentry",
   "barbarians_barrier",
   "barbarians_tower",
   "barbarians_lumberjacks_hut",
   "barbarians_rangers_hut",
   "barbarians_quarry",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_tavern",
   "barbarians_inn",
   "barbarians_big_inn",
   "barbarians_bakery",
   "barbarians_brewery",
   "barbarians_micro_brewery",
   "barbarians_charcoal_kiln",
   "barbarians_smelting_works",
   "barbarians_granitemine",
   "barbarians_coalmine",
   "barbarians_ironmine",
   "barbarians_goldmine",
   "barbarians_ironmine_deep",
   "barbarians_coalmine_deep",
   "barbarians_goldmine_deep",
   "barbarians_goldmine_deeper",
   "barbarians_ironmine_deeper",
   "barbarians_goldmine_deeper",
   "barbarians_metal_workshop",
   "barbarians_farm",
   "barbarians_well",
   "barbarians_cattlefarm",
   "barbarians_hunters_hut",
   "barbarians_fishers_hut",
   "barbarians_gamekeepers_hut"
}
hq3 = p3:place_building("barbarians_headquarters", map.player_slots[3].starting_field, false, true)
hq3:set_wares {
   log = 100,
   granite = 50,
   reed = 50,
   blackwood = 10, -- To delay expansion
   grout = 20,
   coal = 20,
   water = 50,
   wheat = 30,
   iron = 20,
   gold = 5,
   iron_ore = 10,
   gold_ore = 5,
   barbarians_bread = 20,
   beer = 10,
   beer_strong = 20,
   ration = 200, -- Headstart at stealing ores
   snack = 50,
   meal = 50,
   fish = 20,
   meat = 20
}
hq3:set_workers {
   barbarians_builder = 7,
   barbarians_lumberjack = 10,
   barbarians_ranger = 25,
   barbarians_gardener = 4,
   barbarians_ox = 20,
   barbarians_cattlebreeder = 2,
   barbarians_stonemason = 8,
   barbarians_lime_burner = 4,
   barbarians_smelter = 5,
   barbarians_charcoal_burner = 2,
   barbarians_fisher = 2,
   barbarians_hunter = 2,
   barbarians_baker = 3,
   barbarians_brewer = 3,
   barbarians_brewer_master = 1,
   barbarians_miner = 19,
   barbarians_miner_chief = 9,
   barbarians_miner_master = 4,
   barbarians_innkeeper = 6,
   barbarians_geologist = 2,
   barbarians_trainer = 3,
   barbarians_blacksmith = 4,
   barbarians_blacksmith_master = 1,
   barbarians_helmsmith = 1
}
nr_soldiers = {30, 50, 80}
nr_soldiers = nr_soldiers[difficulty]
hq3:set_soldiers({3,5,0,2}, nr_soldiers)
