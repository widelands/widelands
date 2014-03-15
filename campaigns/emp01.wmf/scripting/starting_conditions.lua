-- =======================================================================
--                                 Player 1                                 
-- =======================================================================
p1:forbid_buildings("all")

hq = p1:place_building("headquarters_shipwreck",
   wl.Game().map:get_field(6, 62), false, true)
hq:set_wares{
   helm = 4,
   wood_lance = 5,
   ax = 6,
   bread_paddle = 2,
   basket = 1,
   bread = 8,
   cloth = 5,
   coal = 12,
   fire_tongs = 2,
   fish = 6,
   fishing_rod = 2,
   flour = 4,
   gold = 4,
   grape = 4,
   hammer = 12,
   hunting_spear = 2,
   iron = 12,
   ironore = 5,
   kitchen_tools = 4,
   marble = 25,
   marblecolumn = 6,
   meal = 4,
   meat = 6,
   pick = 14,
   ration = 12,
   saw = 3,
   scythe = 5,
   shovel = 6,
   stone = 40,
   log = 30,
   water = 12,
   wheat = 4,
   wine = 8,
   wood = 45,
   wool = 2,
}
hq:set_workers{
   armorsmith = 1,
   brewer = 1,
   builder = 10,
   burner = 1,
   carrier = 40,
   geologist = 4,
   lumberjack = 3,
   miner = 4,
   stonemason = 2,
   toolsmith = 2,
   weaponsmith = 1,
}
hq:set_soldiers({0,0,0,0}, 45)

