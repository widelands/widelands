-- =======================================================================
--                            Starting conditions
-- =======================================================================

-- Allow some buildings
plr:forbid_buildings("all")
plr:allow_buildings{
   "barbarians_lumberjacks_hut",
   "barbarians_quarry",
   "barbarians_rangers_hut"
}

-- Place headquarters and fill it with wares
hq = plr:place_building("barbarians_headquarters_interim", sf, false, true)
hq:set_wares{
   ax = 6,
   blackwood = 32,
   barbarians_bread = 8,
   bread_paddle = 2,
   coal = 5,
   fire_tongs = 2,
   fish = 6,
   granite = 40,
   grout = 4,
   hammer = 12,
   hunting_spear = 2,
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
   thatch_reed = 24,
}
hq:set_workers{
   barbarians_blacksmith = 2,
   barbarians_brewer = 1,
   barbarians_builder = 10,
   barbarians_carrier = 40,
   barbarians_charcoal_burner = 1,
   barbarians_fisher = 1,
   barbarians_gardener = 1,
   barbarians_geologist = 4,
   barbarians_lime_burner = 1,
   barbarians_lumberjack = 6,
   barbarians_miner = 4,
   barbarians_ranger = 2,
   barbarians_stonemason = 2
}
hq:set_soldiers({0,0,0,0}, 45)

-- ============
-- Build roads
-- ============
connected_road(plr, sf.brn.immovable, "r,r|br,r|r,r")
connected_road(plr, sf.brn.immovable, "l,l|l,bl,bl|br,r|br,r|r,tr|tr,tr,tr")
connected_road(plr, map:get_field(13,14).immovable, "r,br|br,r")

-- Place some initial buildings
prefilled_buildings(plr,
   {"barbarians_lumberjacks_hut", 15, 11},
   {"barbarians_lumberjacks_hut", 9, 13},
   {"barbarians_quarry", 8, 12}
)
