-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")
p1:allow_buildings{
   "barbarians_lumberjacks_hut",
   "barbarians_quarry",
   "barbarians_rangers_hut",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_sentry",
}

hq = p1:place_building("barbarians_headquarters", wl.Game().map:get_field(52, 33), false, true)
hq:set_wares{
   ax = 1,
   blackwood = 32,
   barbarians_bread = 8,
   bread_paddle = 2,
   coal = 12,
   felling_ax = 5,
   fire_tongs = 2,
   fish = 6,
   granite = 40,
   grout = 12,
   hammer = 12,
   hunting_spear = 2,
   iron = 12,
   iron_ore = 25,
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
   barbarians_fisher = 2,
   barbarians_gardener = 1,
   barbarians_geologist = 4,
   barbarians_lime_burner = 1,
   barbarians_lumberjack = 3,
   barbarians_miner = 4,
   barbarians_ox = 40,
   barbarians_ranger = 1,
   barbarians_stonemason = 2,
}
hq:set_soldiers({0,0,0,0}, 45)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{"barbarians_lumberjacks_hut"}

wh = p2:place_building("barbarians_warehouse", wl.Game().map:get_field(100,25), false, true)
wh:set_wares{log = 200}
wh:set_workers{ barbarians_carrier=40, barbarians_builder=1, barbarians_lumberjack=4 }

prefilled_buildings(p2,
   {"barbarians_tower", 102, 71, soldiers={[{0,0,0,0}] = 4}},
   {"barbarians_tower", 104, 18, soldiers={
      [{0,0,0,0}] = 1, [{2,0,0,0}] = 1, [{0,1,0,0}] = 1
   }},
   {"barbarians_tower", 104, 25, soldiers = {[{0,0,0,0}]=2, [{1,0,0,0}]=1, }},
   {"barbarians_tower", 102, 83, soldiers = {[{0,0,0,0}]=2 }},
   {"barbarians_barrier", 103, 11, soldiers = {[{0,0,0,0}]=2, [{0,0,0,1}]=1 }},
   {"barbarians_barrier", 104, 4, soldiers = {[{0,0,0,0}]=4}},
   {"barbarians_barrier", 102, 103, soldiers = {[{0,0,0,0}]=2, [{0,1,0,1}]=1}},
   {"barbarians_barrier", 103, 96, soldiers = {[{0,0,0,0}]=3}},
   {"barbarians_barrier", 104, 110, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_barrier", 104, 30, soldiers = {
      [{0,1,0,1}]=1, [{1,0,0,0}]=1, [{0,0,0,0}]=1
   }},
   {"barbarians_barrier", 102, 90, soldiers = {[{0,0,0,0}]=1}},
   {"barbarians_sentry", 108, 3, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_sentry", 107, 104, soldiers = {[{0,0,0,0}]=1}},
   {"barbarians_sentry", 110, 19, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_sentry", 106, 91, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_sentry", 108, 84, soldiers = {[{0,0,0,0}]=2}}
)

-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:forbid_buildings("all")
prefilled_buildings(p3,
   {"barbarians_sentry", 137, 87, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_barrier", 130, 86, soldiers = {[{0,0,0,0}]=4}},
   {"barbarians_barrier", 132, 82, soldiers = {[{0,0,0,0}]=4}},
   {"barbarians_fortress", 135, 85, soldiers = {
      [{0,0,0,0}]=4, [{1,0,0,0}]=2, [{1,1,0,0}]=2,
   }},
   {"barbarians_barrier", 132, 78, soldiers = {
      [{0,0,0,0}]=1, [{1,1,0,1}]=1, [{0,1,0,0}]=1,
   }}
)

-- =======================================================================
--                                 Player 4
-- =======================================================================
p4:forbid_buildings("all")
wh = p4:place_building("barbarians_warehouse", wl.Game().map:get_field(132,110), false, true)
wh:set_wares {log = 10}
wh:set_workers {
   barbarians_carrier = 40,
   barbarians_builder = 1,
   barbarians_blacksmith = 1,
   barbarians_blacksmith_master = 1
}

prefilled_buildings(p4,
   {"barbarians_citadel", 135, 3, soldiers = {
      [{0,0,0,0}]=2, [{1,1,0,0}]=1, [{0,0,0,1}]=2, [{0,1,0,1}]=1,
   }},
   {"barbarians_sentry", 137, 5, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_sentry", 133, 1, soldiers = {[{0,1,0,0}]=2}},
   {"barbarians_sentry", 130, 6, soldiers = {[{0,0,0,0}]=2}},
   {"barbarians_barrier", 133, 106, soldiers = {[{0,0,0,0}]=2, [{1,0,0,0}]=2}},
   {"barbarians_tower", 132, 7, soldiers = {
      [{0,0,0,0}]=2, [{1,1,0,0}]=1, [{1,0,0,1}]=1,
   }},
   {"barbarians_barrier", 132, 3, soldiers = {
      [{0,0,0,0}]=1, [{1,0,0,1}]=1, [{0,1,0,1}]=1,
   }},
   {"barbarians_warmill", 135, 7}
)
