-- Map

for i,f in pairs( -- array_combine(
   obstacles_1 --,
--    obstacles_2,
--    obstacles_3 )
) do
   map:place_immovable("debris00", f)
end

-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")
p1:forbid_buildings({"frisians_debris_quarry"})
hq = p1:place_building("frisians_port", map.player_slots[1].starting_field, false, true)
hq:set_wares {
   log = 40,
   brick = 30,
   granite = 20,
   reed = 20,
   clay = 5,
   water = 5,
   coal = 20,
   iron = 5,
   iron_ore = 5,
   gold_ore = 10,
   ration = 30,
}
hq:set_workers {
   frisians_woodcutter = 3,
   frisians_forester = 7,
   frisians_claydigger = 3,
   frisians_brickmaker = 2,
   frisians_builder = 10,
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
   frisians_reindeer = 2,
   frisians_miner = 4,
   frisians_charcoal_burner = 3
}
local soldiers = {35, 20, 5}
hq:set_soldiers({0,0,0,0}, soldiers[difficulty])

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:allow_buildings("all")
p2:forbid_buildings {
   "atlanteans_port",
   "atlanteans_shipyard",
   "atlanteans_castle",
}

hq = p2:place_building("atlanteans_headquarters", map.player_slots[2].starting_field, false, true)
hq:set_wares {
   diamond = 7,
   iron_ore = 5,
   quartz = 9,
   granite = 50,
   spider_silk = 9,
   log = 20,
   coal = 12,
   gold = 4,
   gold_thread = 6,
   iron = 8,
   planks = 45,
   spidercloth = 5,
   blackroot = 5,
   blackroot_flour = 12,
   atlanteans_bread = 8,
   corn = 5,
   cornmeal = 12,
   fish = 3,
   meat = 3,
   smoked_fish = 6,
   smoked_meat = 6,
   water = 12,
   bread_paddle = 2,
   buckets = 2,
   fire_tongs = 2,
   fishing_net = 4,
   hammer = 11,
   hunting_bow = 1,
   milking_tongs = 2,
   hook_pole = 2,
   pick = 8,
   saw = 9,
   scythe = 4,
   shovel = 9,
   tabard = 5,
   trident_light = 5,
}
hq:set_workers {
   atlanteans_armorsmith = 1,
   atlanteans_blackroot_farmer = 1,
   atlanteans_builder = 4, -- Not too fast!
   atlanteans_charcoal_burner = 1,
   atlanteans_carrier = 40,
   atlanteans_fishbreeder = 1,
   atlanteans_geologist = 4,
   atlanteans_miner = 4,
   atlanteans_sawyer = 1,
   atlanteans_stonecutter = 2,
   atlanteans_toolsmith = 2,
   atlanteans_trainer = 3,
   atlanteans_weaponsmith = 1,
   atlanteans_woodcutter = 3,
   atlanteans_horse = 5,
}
soldiers = {
   {{0,0,0,0}, 25},
   {{1,1,2,0}, 35},
   {{1,4,2,2}, 50},
}
hq:set_soldiers(soldiers[difficulty][1], soldiers[difficulty][2])

-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:allow_buildings("all")
p3:forbid_buildings {
   "empire_port",
   "empire_shipyard",
   "empire_fortress",
}

hq = p3:place_building("empire_port", map.player_slots[3].starting_field, false, true)
hq:set_wares {
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
hq:set_workers {
   empire_builder = 3, -- Not too fast!
   empire_lumberjack = 4,
   empire_forester = 8,
   empire_stonemason = 4,
   empire_miner = 2,
   empire_miner_master = 3,
   empire_toolsmith = 1,
   empire_carpenter = 1,
   empire_donkey = 30,
   empire_donkeybreeder = 1,
   empire_weaver = 2,
   empire_fisher = 2,
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
soldiers = {
   {{0,0,0,0}, 50},
   {{2,2,0,1}, 60},
   {{4,4,0,2}, 90},
}
hq:set_soldiers(soldiers[difficulty][1], soldiers[difficulty][2])
