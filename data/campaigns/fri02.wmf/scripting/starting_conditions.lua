-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")
p1:forbid_buildings{"frisians_aqua_farm", "frisians_recycling_centre"}

hq = p1:place_building("frisians_port",
   map.player_slots[1].starting_field, false, true)
hq:set_wares {
   log = 40,
   brick = 50,
   clay = 20,
   granite = 40,
   water = 40,
   coal = 20,
   thatch_reed = 20,
   fruit = 10,
   fish = 10,
   meat = 10,
   smoked_fish = 10,
   smoked_meat = 10,
   bread_frisians = 10,
   ration = 30,
   iron = 2,
   iron_ore = 5,
   gold_ore = 1,
}
hq:set_workers {
   frisians_woodcutter = 3,
   frisians_forester = 5,
   frisians_clay_burner = 4,
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
   frisians_reed_farmer = 2,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_trainer = 3,
}
hq:set_soldiers({0,0,0,0}, 20)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:allow_buildings("all")

hq2 = p2:place_building("empire_headquarters",
   map.player_slots[2].starting_field, false, true)
hq2:set_wares {
   log = 50,
   granite = 500,
   water = 100,
   coal = 200,
   iron = 50,
}
hq2:set_workers {
   empire_builder = 10,
}
hq2:set_soldiers({4,4,0,2}, 50)

-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:allow_buildings("all")

hq3 = p3:place_building("barbarians_headquarters",
   map.player_slots[3].starting_field, false, true)
hq3:set_wares {
   log = 50,
   granite = 500,
   water = 100,
   coal = 200,
   iron = 50,
}
hq3:set_workers {
   barbarians_builder = 10,
}
hq3:set_soldiers({3,5,0,2}, 100)
