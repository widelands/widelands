-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")
p1:allow_buildings {"frisians_sentinel","frisians_warehouse"}

hq = p1:place_building("frisians_headquarters", map.player_slots [1].starting_field, false, true)
hq:set_wares {
   log = 40,
   brick = 50,
   clay = 20,
   granite = 40,
   water = 40,
   coal = 20,
   reed = 20,
   fruit = 10,
   fish = 10,
   meat = 10,
   smoked_fish = 10,
   smoked_meat = 10,
   bread_frisians = 10,
   ration = 24,
   iron = 2,
   iron_ore = 5,
   gold_ore = 1,
}
hq:set_workers {
   frisians_woodcutter = 3,
   frisians_forester = 6,
   frisians_claydigger = 3,
   frisians_brickmaker = 2,
   frisians_builder = 10,
   frisians_blacksmith = 4,
   frisians_miner = 5,
   frisians_smelter = 2,
   frisians_smoker = 2,
   frisians_seamstress = 2,
   frisians_landlady = 3,
   frisians_berry_farmer = 3,
   frisians_fruit_collector = 3,
   frisians_beekeeper = 2,
   frisians_fisher = 3,
   frisians_hunter = 1,
   frisians_geologist = 2,
   frisians_farmer = 3,
   frisians_reed_farmer = 3,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_trainer = 3,
}
nr_soldiers = {25, 12, 4}
nr_soldiers = nr_soldiers[difficulty]
hq:set_soldiers({0,0,0,0}, nr_soldiers)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings {
   "frisians_woodcutters_house",
   "frisians_foresters_house",
   "frisians_well",
   "frisians_reed_farm",
   "frisians_clay_pit",
   "frisians_brick_kiln",
   "frisians_fishers_house",
   "frisians_hunters_house",
   "frisians_quarry",
   "frisians_smokery",
   "frisians_tavern",
   "frisians_coalmine",
   "frisians_ironmine",
   "frisians_goldmine",
   "frisians_rockmine",
   "frisians_coalmine_deep",
   "frisians_ironmine_deep",
   "frisians_goldmine_deep",
   "frisians_rockmine_deep",
   "frisians_farm",
   "frisians_bakery",
   "frisians_brewery",
   "frisians_furnace",
   "frisians_blacksmithy",
   "frisians_mead_brewery",
   "frisians_honey_bread_bakery",
   "frisians_drinking_hall",
   "frisians_sentinel"}

hq2 = p2:place_building("frisians_headquarters", map.player_slots [2].starting_field, false, true)
hq2:set_wares {
   log = 50,
   brick = 200,
   clay = 30,
   granite = 200,
   reed = 200,
   water = 20,
   coal = 100,
   iron = 50,
}
hq2:set_workers {
   frisians_woodcutter = 10,
   frisians_forester = 20,
   frisians_brickmaker = 10,
   frisians_builder = 10,
   frisians_blacksmith = 5,
   frisians_miner_master = 10,
   frisians_smelter = 4,
   frisians_smoker = 10,
   frisians_landlady = 10,
   frisians_berry_farmer = 20,
   frisians_fruit_collector = 20,
   frisians_beekeeper = 10,
   frisians_fisher = 20,
   frisians_hunter = 4,
   frisians_geologist = 5,
   frisians_farmer = 20,
   frisians_reed_farmer = 10,
   frisians_baker_master = 5,
   frisians_brewer_master = 5,
   frisians_reindeer = 50,
}
nr_soldiers = {30, 50, 90}
nr_soldiers = nr_soldiers[difficulty]
hq2:set_soldiers({0,6,0,0}, nr_soldiers)
