-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")
p1:allow_buildings{
   "lumberjacks_hut",
   "quarry",
   "rangers_hut",
   "hardener",
   "lime_kiln",
   "reed_yard",
   "sentry",
}

hq = p1:place_building("headquarters", wl.Game().map:get_field(52, 33), false, true)
hq:set_wares{
   ax=1,
   bread_paddle=2,
   blackwood=32,
   coal=12,
   felling_ax=5,
   fire_tongs=2,
   fish=6,
   grout=12,
   hammer=12,
   hunting_spear=2,
   iron=12,
   ironore=25,
   kitchen_tools=4,
   meal=4,
   meat=6,
   pick=14,
   pittabread=8,
   ration=12,
   raw_stone=40,
   scythe=6,
   shovel=4,
   snack=3,
   thatchreed=24,
   log=80,
}
hq:set_workers{
   carrier=40,
   blacksmith=2,
   brewer=1,
   builder=10,
   burner=1,
   gardener=1,
   fisher=2,
   geologist=4,
   ["lime-burner"]=1,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2,
   ox = 40,
}
hq:set_soldiers({0,0,0,0}, 45)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{"lumberjacks_hut"}

wh = p2:place_building("warehouse", wl.Game().map:get_field(100,25), false, true)
wh:set_wares{log=200}
wh:set_workers{ carrier=40, builder=1, lumberjack=4 }

prefilled_buildings(p2,
   {"donjon", 102, 71, soldiers={[{0,0,0,0}] = 4}},
   {"donjon", 104, 18, soldiers={
      [{0,0,0,0}] = 1, [{2,0,0,0}] = 1, [{0,1,0,0}] = 1
   }},
   {"donjon", 104, 25, soldiers = {[{0,0,0,0}]=2, [{1,0,0,0}]=1, }},
   {"donjon", 102, 83, soldiers = {[{0,0,0,0}]=2 }},
   {"barrier", 103, 11, soldiers = {[{0,0,0,0}]=2, [{0,0,0,1}]=1 }},
   {"barrier", 104, 4, soldiers = {[{0,0,0,0}]=4}},
   {"barrier", 102, 103, soldiers = {[{0,0,0,0}]=2, [{0,1,0,1}]=1}},
   {"barrier", 103, 96, soldiers = {[{0,0,0,0}]=3}},
   {"barrier", 104, 110, soldiers = {[{0,0,0,0}]=2}},
   {"barrier", 104, 30, soldiers = {
      [{0,1,0,1}]=1, [{1,0,0,0}]=1, [{0,0,0,0}]=1
   }},
   {"barrier", 102, 90, soldiers = {[{0,0,0,0}]=1}},
   {"sentry", 108, 3, soldiers = {[{0,0,0,0}]=2}},
   {"sentry", 107, 104, soldiers = {[{0,0,0,0}]=1}},
   {"sentry", 110, 19, soldiers = {[{0,0,0,0}]=2}},
   {"sentry", 106, 91, soldiers = {[{0,0,0,0}]=2}},
   {"sentry", 108, 84, soldiers = {[{0,0,0,0}]=2}}
)

-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:forbid_buildings("all")
prefilled_buildings(p3,
   {"sentry", 137, 87, soldiers = {[{0,0,0,0}]=2}},
   {"barrier", 130, 86, soldiers = {[{0,0,0,0}]=4}},
   {"barrier", 132, 82, soldiers = {[{0,0,0,0}]=4}},
   {"fortress", 135, 85, soldiers = {
      [{0,0,0,0}]=4, [{1,0,0,0}]=2, [{1,1,0,0}]=2,
   }},
   {"barrier", 132, 78, soldiers = {
      [{0,0,0,0}]=1, [{1,1,0,1}]=1, [{0,1,0,0}]=1,
   }}
)

-- =======================================================================
--                                 Player 4
-- =======================================================================
p4:forbid_buildings("all")
wh = p4:place_building("warehouse", wl.Game().map:get_field(132,110), false, true)
wh:set_wares{log=10}
wh:set_workers{carrier=40, builder=1, blacksmith=1, ["master-blacksmith"]=1}

prefilled_buildings(p4,
   {"citadel", 135, 3, soldiers = {
      [{0,0,0,0}]=2, [{1,1,0,0}]=1, [{0,0,0,1}]=2, [{0,1,0,1}]=1,
   }},
   {"sentry", 137, 5, soldiers = {[{0,0,0,0}]=2}},
   {"sentry", 133, 1, soldiers = {[{0,1,0,0}]=2}},
   {"sentry", 130, 6, soldiers = {[{0,0,0,0}]=2}},
   {"barrier", 133, 106, soldiers = {[{0,0,0,0}]=2, [{1,0,0,0}]=2}},
   {"donjon", 132, 7, soldiers = {
      [{0,0,0,0}]=2, [{1,1,0,0}]=1, [{1,0,0,1}]=1,
   }},
   {"barrier", 132, 3, soldiers = {
      [{0,0,0,0}]=1, [{1,0,0,1}]=1, [{0,1,0,1}]=1,
   }},
   {"warmill", 135, 7}
)

