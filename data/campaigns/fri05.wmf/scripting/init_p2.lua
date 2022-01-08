-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:allow_buildings("all")
p2:forbid_buildings({"empire_quarry", "empire_hunters_house", "empire_tavern", "empire_coalmine", "empire_ironmine", "empire_goldmine", "empire_marblemine", "empire_vineyard", "empire_winery"})
p2:conquer(map.player_slots[2].starting_field, 9)
hq = p2:place_building("empire_port", map.player_slots[2].starting_field, false, true)
hq:set_wares {
   log = 5000,
   granite = 10000,
   marble = 10000,
   marble_column = 20,
   planks = 40,
   cloth = 10,
   beer = 15,
   meal = 5,
   wheat = 30,
   water = 50,
   coal = 250,
   iron = 50,
   gold = 50,
   gold_ore = 250,
   iron_ore = 500,
   meat = 20,
   fish = 20,
   empire_bread = 20
}
hq:set_workers {
   empire_builder = 3,
   empire_lumberjack = 4,
   empire_forester = 8,
   empire_stonemason = 4,
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
   empire_pigbreeder = 3,
   empire_charcoal_burner = 2,
   empire_smelter = 4,
   empire_trainer = 3,
   empire_armorsmith = 1,
   empire_weaponsmith = 1,
}
local soldiers = {50, 300, 550}
hq:set_soldiers({
   [{0,0,0,0}] = soldiers[difficulty],
   [{2,2,0,1}] = soldiers[difficulty],
   [{4,4,0,2}] = soldiers[difficulty],
})
