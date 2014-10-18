-- =======================================================================
--                            Starting conditions
-- =======================================================================

-- Allow some buildings
plr:forbid_buildings("all")
plr:allow_buildings{
   "lumberjacks_hut",
   "quarry",
   "rangers_hut"
}

-- Place headquarters and fill it with wares
hq = plr:place_building("headquarters_interim", sf, false, true)
hq:set_wares{
   ax=6,
   bread_paddle=2,
   blackwood=32,
   coal=5,
   fire_tongs=2,
   fish=6,
   grout=4,
   hammer=12,
   hunting_spear=2,
   ironore=5,
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
   blacksmith=2,
   brewer=1,
   builder=10,
   carrier=40,
   charcoal_burner=1,
   gardener=1,
   fisher=1,
   geologist=4,
   ["lime-burner"]=1,
   lumberjack=3,
   miner=4,
   ranger=2,
   stonemason=2
}
hq:set_soldiers({0,0,0,0}, 45)

-- ============
-- Build roads
-- ============
--connected_road(plr,sf.brn.immovable, "r,r|br,r")
connected_road(plr, sf.brn.immovable, "r,r|br,r|r,r")
connected_road(plr, sf.brn.immovable, "l,l|l,bl,bl|br,r|br,r|r,tr|tr,tr,tr")
connected_road(plr, map:get_field(13,14).immovable, "r,br|br,r")

-- Place some initial buildings
prefilled_buildings(plr,
   {"lumberjacks_hut", 15, 11},
   {"lumberjacks_hut", 9, 13},
   {"quarry", 8, 12}
)
