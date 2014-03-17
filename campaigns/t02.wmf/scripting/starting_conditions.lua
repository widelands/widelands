-- =======================================================================
--                            Starting conditions                           
-- =======================================================================

-- Allow some buildings
plr:forbid_buildings("all")
plr:allow_buildings{
   "lumberjacks_hut",
   "quarry", 
   "rangers_hut", 
   "sentry",
   "barrier"
}

-- Place hq and fill it with wares
hq_pos = wl.Game().map:get_field(12,10)
hq = plr:place_building("headquarters_interim", hq_pos, false, true)
hq:set_wares{
   ax=6,
   bread_paddle=2,
   blackwood=32,
   coal=12,
   fire_tongs=2,
   fish=6,
   grout=12,
   hammer=12,
   hunting_spear=2,
   iron=12,
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
   burner=1,
   carrier=40,
   gardener=1,
   fisher=1,
   geologist=4,
   ["lime-burner"]=1,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2
}
hq:set_soldiers({0,0,0,0}, 45)

-- ============
-- Build roads 
-- ============
connected_road(plr, hq_pos.brn.immovable, "r,r|br,r|r,r")
connected_road(plr, hq_pos.brn.immovable, "l,l|l,bl,bl|br,r|br,r|r,tr|tr,tr,tr")

-- Place some buildings from the last map
prefilled_buildings(plr,
   {"lumberjacks_hut", 15, 11},
   {"lumberjacks_hut", 12, 13},
   {"quarry", 8, 12},
   {"rangers_hut", 9, 13}
) 


