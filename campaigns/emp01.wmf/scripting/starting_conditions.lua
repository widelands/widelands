-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")

hq = p1:place_building("empire_headquarters_shipwreck",
   wl.Game().map:get_field(6, 62), false, true)
hq:set_wares {
   basket = 1,
   empire_bread = 8,
   bread_paddle = 2,
   cloth = 5,
   coal = 12,
   felling_ax = 6,
   fire_tongs = 2,
   fish = 6,
   fishing_rod = 2,
   flour = 4,
   gold = 4,
   granite = 40,
   grape = 4,
   hammer = 12,
   armor_helmet = 4,
   hunting_spear = 2,
   iron = 12,
   iron_ore = 5,
   kitchen_tools = 4,
   log = 30,
   marble = 25,
   marble_column = 6,
   meal = 4,
   meat = 6,
   pick = 14,
   planks = 45,
   ration = 12,
   saw = 3,
   scythe = 5,
   shovel = 6,
   spear_wooden = 5,
   water = 12,
   wheat = 4,
   wine = 8,
   wool = 2,
}
hq:set_workers {
   empire_armorsmith = 1,
   empire_brewer = 1,
   empire_builder = 10,
   empire_carrier = 40,
   empire_charcoal_burner = 1,
   empire_geologist = 4,
   empire_lumberjack = 3,
   empire_miner = 4,
   empire_stonemason = 2,
   empire_toolsmith = 2,
   empire_weaponsmith = 1,
}
hq:set_soldiers({0,0,0,0}, 45)
